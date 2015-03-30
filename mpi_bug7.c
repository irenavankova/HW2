/******************************************************************************
* FILE: mpi_bug7.c
* DESCRIPTION:
*   This program hangs.
* AUTHOER: Blaise Barney
* LAST REVISED: 01/24/09
******************************************************************************/


/******************************************************************************
* Irena's comments
* Buffer has 1 entry, not number of entries equal to rank, change count to 1 in Bcast
******************************************************************************/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
int   numtasks, taskid, len, buffer, root, count;
char hostname[MPI_MAX_PROCESSOR_NAME];

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);

printf ("Task %d on %s starting...\n", taskid, hostname);
buffer = 23;
root = 0;
count = taskid;
if (taskid == root)
   printf("Root: Number of MPI tasks is: %d\n",numtasks);

MPI_Bcast(&buffer, 1, MPI_INT, root, MPI_COMM_WORLD);


MPI_Finalize();

}

