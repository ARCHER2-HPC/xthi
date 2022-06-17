
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>

#include <mpi.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_max_threads() 1
#define omp_get_num_threads() 1
#define omp_get_thread_num()  0
#endif

#include "gpu.h"

struct xthi_options_s {
  unsigned int c;       /* Create, and report on, a Cartesian communicator */
  unsigned int r;       /* Use reorder true in MPI_Cart_create() */
  unsigned int s;       /* sleep(seconds) before MPI_Finalize() */
  unsigned int t;       /* Do not report threads (or "report thread 0 only") */
};

typedef struct xthi_options_s xthi_options_t;

struct xthi_cart_s {
  int rank;          /* Cartesian communicator rank */
  int ndim;          /* Must be 1, 2, or 3 */
  int dims[3];       /* Extent in each dimension */
  int coords[3];     /* Coordinates this rank */
};

typedef struct xthi_cart_s xthi_cart_t;

int xthi_options(int argc, char * argv[], xthi_options_t * opts);
int xthi_cart(MPI_Comm parent, int ndim, int reorder, xthi_cart_t * cart);
int xthi_cart_str(const xthi_cart_t * cart, char * buf);

/* Borrowed from util-linux-2.13-pre7/schedutils/taskset.c */

static char *cpuset_to_cstr(cpu_set_t *mask, char *str)
{
  char *ptr = str;
  int i, j, entry_made = 0;
  for (i = 0; i < CPU_SETSIZE; i++) {
    if (CPU_ISSET(i, mask)) {
      int run = 0;
      entry_made = 1;
      for (j = i + 1; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, mask)) run++;
        else break;
      }
      if (!run)
        sprintf(ptr, "%d,", i);
      else if (run == 1) {
        sprintf(ptr, "%d,%d,", i, i + 1);
        i++;
      } else {
        sprintf(ptr, "%d-%d,", i, i + run);
        i += run;
      }
      while (*ptr != 0) ptr++;
    }
  }
  ptr -= entry_made;
  *ptr = 0;
  return(str);
}

/* Parse command line argc, argv for options of interest. */

int xthi_options(int argc, char * argv[], xthi_options_t * opts) {

  int ierr = 0;

  assert(argv);
  assert(opts);

  for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
    switch (argv[optind][1]) {
    case 'c':
      opts->c = (unsigned int) strtoul(argv[++optind], NULL, 0);
      if (opts->c > 3) {
	fprintf(stderr, "Invalid value for -c option\n");
      }
      break;
    case 'r':
      opts->r = 1;
    case 's':
      opts->s = (unsigned int) strtoul(argv[++optind], NULL, 0);
      break;
    case 't':
      opts->t = 1;
      break;
    default:
      fprintf(stderr, "Unrecognised option: %s\n", argv[optind]);
      fprintf(stderr, "Usage: %s [-cs]\n", argv[0]);
      fprintf(stderr, "Option [-c]       Cartesian communicator\n");
      fprintf(stderr, "Option [-r]       Reorder in MPI_Cart_create()\n");
      fprintf(stderr, "Option [-s sleep] work for s seconds\n");
      fprintf(stderr, "Option [-t]       report thread 0 only\n");
      ierr = 1;
    }
  }

  return ierr;
}

/* Routine to output list of nodes in order */
/* In general, we need to worry about nodes running differing numbers
 * of MPI tasks. So counting the number of nodes from the shared
 * comunicators takes a little care */

int xthi_print_node(MPI_Comm parent, int argc, char ** argv, FILE * fp) {

  int rank;        /* Parent rank */
  int sz;          /* Parent size */

  MPI_Comm comm;   /* Shared comm */
  int srank;       /* Shared rank */
  int ssz;         /* Shared size */

  int nnodes = -1;            /* Number of nodes (total) */
  int noderank = -1;          /* Node rank */
  char hnbuf[256] = {};       /* Host name this node */
  char msg[BUFSIZ] = {};      /* Message string */

  MPI_Comm_rank(parent, &rank);
  MPI_Comm_size(parent, &sz);
  
  MPI_Comm_split_type(parent, MPI_COMM_TYPE_SHARED, rank, MPI_INFO_NULL,
		      &comm);

  MPI_Comm_rank(comm, &srank);
  MPI_Comm_size(comm, &ssz);

  /* We can define rank 0 to be on node 0, but after that, can make
   * no assumptions about where ranks have been placed.
   * So, form a communicator from ranks having rank 0 in the shared
   * comms, and use those to work out a node index. Then broadcast
   * the result in the shared comms.
   */

  nnodes = 0;
  noderank = 0;

  {
    MPI_Comm xcomm;
    MPI_Comm_split(parent, srank, rank, &xcomm);
    if (srank == 0) {
      MPI_Comm_rank(xcomm, &noderank);
      MPI_Comm_size(xcomm, &nnodes);
    }
    MPI_Bcast(&noderank, 1, MPI_INT, 0, comm);
    MPI_Comm_free(&xcomm);
  }
  /* Root receives msg from shared rank 0 in each node */

  if (1) {
    /* We can't use getDeviceCount() because we want number per node */
    int ompsz = 1;
    int ndevice = gpu_per_node();

    /* Message: node, hostname, mpi tasks (shared comm size), [gpu] exec */
    char * exec = strrchr(argv[0], '/');
    if (exec == NULL) exec = argv[0];
    if (exec[0] == '/') exec = &exec[1];

    #pragma omp parallel
    {
      ompsz = omp_get_num_threads();
    }

    (void) gethostname(hnbuf, sizeof(hnbuf));
    if (ndevice == 0) {
      sprintf(msg, "Node %4d, hostname %s, mpi %3d, omp %3d, executable %s\n",
	            noderank, hnbuf, ssz, ompsz, exec);
    }
    else {
      /* Report GPU per node */
      sprintf(msg, "Node %4d, hostname %s, mpi %3d, omp %3d, gpu %d, exe %s\n",
	    noderank, hnbuf, ssz, ompsz, ndevice, exec);
    }

    /* All ranks send again */
    if (rank > 0) {
      MPI_Send(&srank, 1, MPI_INT, 0, 9997, parent);
      MPI_Send(msg, BUFSIZ, MPI_CHAR, 0, 9996, parent);
    }
    
    /* rank 0 recv msg for each node and report */

    if (rank == 0) {
      fprintf(fp, "Node summary for %4d nodes:\n", nnodes);
      fprintf(fp, "%s", msg);
      for (int r = 1; r < sz; r++) {
	int rsrank = -1;
	MPI_Recv(&rsrank, 1, MPI_INT, r, 9997, parent, MPI_STATUS_IGNORE);
	MPI_Recv(msg, BUFSIZ, MPI_CHAR, r, 9996, parent, MPI_STATUS_IGNORE);
	if (rsrank == 0) fprintf(fp, "%s", msg);
      }
    }

  }

  MPI_Comm_free(&comm);

  return noderank;
}

/* Collect information on Cartesian communicator using the default
 * MPI_Dims_create(). The communicator itself does not survive this
 * routine. */
/* The parent communicator may typically be MPI_COMM_WORLD. */

int xthi_cart(MPI_Comm parent, int ndim, int reorder, xthi_cart_t * cart) {

  MPI_Comm comm = MPI_COMM_NULL;
  int periodic[3] = {1, 1, 1};
  int psz = -1;

  assert(cart);

  cart->ndim = ndim;

  MPI_Comm_size(parent, &psz);
  MPI_Dims_create(psz, cart->ndim, cart->dims);

  MPI_Cart_create(parent, cart->ndim, cart->dims, periodic, reorder, &comm);
  MPI_Comm_rank(comm, &cart->rank);
  MPI_Cart_coords(comm, cart->rank, cart->ndim, cart->coords);

  MPI_Comm_free(&comm);

  return 0;
}

/* Form a string which describes the Cartesian coordinates */

int xthi_cart_str(const xthi_cart_t * cart, char * buf) {

  assert(cart);

  switch (cart->ndim) {
  case 1:
    sprintf(buf, " Cartesian (%2d)", cart->coords[0]);
    break;
  case 2:
    sprintf(buf, " Cartesian (%2d %2d)", cart->coords[0], cart->coords[1]);
    break;
  case 3:
    sprintf(buf, " Cartesian (%2d %2d %2d)",
	    cart->coords[0], cart->coords[1], cart->coords[2]);
    break;
  default:
    sprintf(buf, " ");
  }

  return 0;
}
/* Same trick but for the Cartesian extent */

int xthi_cart_ext(const xthi_cart_t * cart, char * buf) {

  assert(cart);

  switch (cart->ndim) {
  case 1:
    sprintf(buf, " Cartesian extent (%2d)", cart->dims[0]);
    break;
  case 2:
    sprintf(buf, " Cartesian extent (%2d %2d)", cart->dims[0], cart->dims[1]);
    break;
  case 3:
    sprintf(buf, " Cartesian extent (%2d %2d %2d)",
	    cart->dims[0], cart->dims[1], cart->dims[2]);
    break;
  default:
    sprintf(buf, " ");
  }

  return 0;
}

/* Print node/mpi/thread placement information to fp.
 * As we are interested in physical placement, it's always apprpriate to
 * gather information by node. Otherwise, we try not to make any
 * assumptions about what processes/threads may be where. */
/* The parent communicator may typically be MPI_COMM_WORLD. */

int xthi_print(MPI_Comm parent, int argc, char ** argv, FILE * fp) {

  int prank = -1;                  /* Parent rank */
  int psz = -1;                    /* Parent size */

  MPI_Comm shared = MPI_COMM_NULL; /* Shared comm */
  int srank = -1;                  /* Shared rank */
  int ssz = -1;                    /* Shared size */

  MPI_Comm xnodes = MPI_COMM_NULL; /* 'cross-node' communicator */
  int nrank = -1;                  /* node rank */
  int nsz = -1;                    /* xnodes size (== no. nodes at root) */

  int nthreads = -1;               /* Total threads this rank */
  int nid = -1;                    /* Node id 0,1,... */

  int ndevice = 0;                 /* Default GPU not present. */

  MPI_Request sreq[3] = {};        /* Requests in shared comm */
  MPI_Request * nreq = NULL;       /* Requests in xnodes comm 1 per thread */

  int * pranks = NULL;             /* List of ranks for parent */
  int * pthreads = NULL;           /* List of nthreads for parent */
  char exbuf[256] = "";
  char ms[BUFSIZ] = {};

  xthi_options_t options = {};
  xthi_cart_t    cart = {};

  assert(fp);

  xthi_options(argc, argv, &options);
  xthi_print_node(parent, argc, argv, fp);

  MPI_Comm_rank(parent, &prank);
  MPI_Comm_size(parent, &psz);

  /* Shared communicator (per node) */
  MPI_Comm_split_type(parent, MPI_COMM_TYPE_SHARED, prank, MPI_INFO_NULL,
		      &shared);

  MPI_Comm_rank(shared, &srank);
  MPI_Comm_size(shared, &ssz);

  /* Cartesian coordinates string (if required) */
  if (options.c) {
    int ndim = options.c;
    int reorder = options.r;

    xthi_cart(parent, ndim, reorder, &cart);
    xthi_cart_str(&cart, exbuf);
  }

  /* Device count */
  xpuGetDeviceCount(&ndevice);

  /* We can define rank 0 to be on node 0, but after that, can make
   * no assumptions about where ranks have been placed.
   * So, form a communicator from ranks having srank in the shared
   * comms, and use those to work out a node index. Then broadcast
   * the result in the shared comms.
   */

  MPI_Comm_split(parent, srank, prank, &xnodes);
  MPI_Comm_rank(xnodes, &nrank);
  MPI_Comm_size(xnodes, &nsz);

  if (srank == 0) nid = nrank;
  MPI_Bcast(&nid, 1, MPI_INT, 0, shared);

  /* Gather parent ranks and number of threads at shared root */
  /* If no thread information is wanted, only tid 0 is relevant */

  nthreads = omp_get_max_threads();
  if (options.t) nthreads = 1;

  pranks = (int *) calloc(ssz, sizeof(int));
  pthreads = (int *) calloc(ssz, sizeof(int));
  assert(pranks);
  assert(pthreads);

  MPI_Gather(&prank, 1, MPI_INT, pranks, 1, MPI_INT, 0, shared);
  MPI_Gather(&nthreads, 1, MPI_INT, pthreads, 1, MPI_INT, 0, shared);

  /* Forward to root */

  if (srank == 0) {
    MPI_Isend(&ssz, 1, MPI_INT, 0, 700, xnodes, sreq);
    MPI_Isend(pranks, ssz, MPI_INT, 0, 701, xnodes, sreq + 1);
    MPI_Isend(pthreads, ssz, MPI_INT, 0, 702, xnodes, sreq + 2);
  }
  else {
    sreq[0] = MPI_REQUEST_NULL;
    sreq[1] = MPI_REQUEST_NULL;
    sreq[2] = MPI_REQUEST_NULL;
  }

  /* Every thread frms a string and sends to root in in parent.
   * These should be in thread order. */

  nreq = (MPI_Request *) calloc(nthreads, sizeof(MPI_Request));
  assert(nreq);

#pragma omp parallel private(ms)
  {
    int tid = omp_get_thread_num();
    int cpu = sched_getcpu();

    cpu_set_t coremask;
    char clbuf[7 * CPU_SETSIZE];
    char gpubuf[32] = "";

    memset(clbuf, 0, sizeof(clbuf));
    sched_getaffinity(0, sizeof(coremask), &coremask);
    cpuset_to_cstr(&coremask, clbuf);

    if (ndevice == 0) {
      sprintf(ms, "Node %4d, rank %4d, thread %3d, (cpu %3d, set %4s) %s\n",
	      nid, prank, tid, cpu, clbuf, exbuf);
    }
    else {
      /* GPU: insert local device info */
      int device = -1;
      xpuGetDevice(&device);
      assert(ndevice < 10); /* single figures at the moment! */
      sprintf(gpubuf, "device %1d/%1d (rsmi%d)", device, ndevice,
	      gpu_node_id(device));
      sprintf(ms, "Node %4d, rank %4d, thread %3d, (cpu %3d, set %7s) %s %s\n",
	      nid, prank, tid, cpu, clbuf, gpubuf, exbuf);
    }

    for (int nt = 0; nt < nthreads; nt++) {
      #pragma omp barrier
      if (nt == tid) {
	MPI_Isend(ms, BUFSIZ, MPI_CHAR, 0, 800, parent, nreq + nt);
      }
    }
  }

  /* Handle all messages at root */

  if (prank == 0) {

    char cbuf[256] = "";

    /* Summary */

    if (options.c) xthi_cart_ext(&cart, cbuf);
    fprintf(fp, "MPI summary: %d ranks %s\n", psz, cbuf);

    /* Field messages from each node in order and print ... */

    for (int node = 0; node < nsz; node++) {

      int * rranks = NULL;
      int * rthreads = NULL;

      /* Recv number of remote ranks for this node */
      int nrsz = -1;
      MPI_Recv(&nrsz, 1, MPI_INT, node, 700, xnodes, MPI_STATUS_IGNORE);

      rranks = (int *) calloc(nrsz, sizeof(int));
      rthreads = (int *) calloc(nrsz, sizeof(int));
      assert(rranks);
      assert(rthreads);
      MPI_Recv(rranks, nrsz, MPI_INT, node, 701, xnodes, MPI_STATUS_IGNORE);
      MPI_Recv(rthreads, nrsz, MPI_INT, node, 702, xnodes, MPI_STATUS_IGNORE);

      for (int nr = 0; nr < nrsz; nr++) {
	int src = rranks[nr];
	for (int nt = 0; nt < rthreads[nr]; nt++) {
	  MPI_Recv(ms, BUFSIZ, MPI_CHAR, src, 800, parent, MPI_STATUS_IGNORE);
	  fprintf(fp, "%s", ms);
	}
      }
      free(rthreads);
      free(rranks);
    }
  }

  /* Clear own send requests. */
  MPI_Waitall(3, sreq, MPI_STATUSES_IGNORE);
  MPI_Waitall(nthreads, nreq, MPI_STATUSES_IGNORE);

  free(nreq);
  MPI_Comm_free(&xnodes);
  MPI_Comm_free(&shared);

  /* Simulate some actual work, and finish. */

  sleep(options.s);
  MPI_Barrier(parent);

  return 0;
}
