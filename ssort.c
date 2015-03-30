/* Parallel sample sort
 */

/******************************************************************************
* Irena's comments
* Define bellow what you want to print on the screen, by default everything is printed
* The code seems to work for N that is multiple of the number of processors
******************************************************************************/

#define  MASTER		0
#define print_init 1
#define print_splitters 1
#define print_final 1
#define print_finished 1
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>


static int compare(const void *a, const void *b)
{
  int *da = (int *)a;
  int *db = (int *)b;

  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}

int main( int argc, char *argv[])
{
    int rank, size, rc;
    int i, j, N, S;
    int bln, bln2, counter, elem_count, sum, rrr, newvec_elements;
    int *vec, *samp, *num_elements, *newvec;
    int *splitter, *temp, *num_elements_all;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Number of random numbers per processor (this should be increased
     * for actual tests or could be passed in through the command line */
    N = 8;


    if (N % (size) != 0) {
        printf("Quitting. Array size must be divisible by #p-1.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(0);
    }
    S = N/(size);
    
    //allocate vectors on all processors (including root)
    vec = calloc(N, sizeof(int));
    samp = calloc(S, sizeof(int));
    /* seed random number generator differently on every core */
    srand((unsigned int) (rank + 393919));


    /* fill vector with random integers */
    for (i = 0; i < N; ++i) {
        vec[i] = rand();
        if (print_init == 1)
        printf("rank: %d, entry: %d in initial vector is: %d\n", rank, i, vec[i]);
    }

    /* randomly sample s entries from vector or select local splitters,
     * i.e., every N/P-th entry of the sorted vector */
    for (j = 0; j < S; ++j) {
         samp[j] = vec[j];
         //printf("rank: %d, sample: %d is: %d\n", rank, j, samp[j]);
    }
    //designate temporary vector on MASTER to gather info sent from otehr processors
    if (rank == 0){
        temp = calloc(S*(size), sizeof(int));
    }
    /* every processor communicates the selected entries
     * to the root processor; use for instance an MPI_Gather */
    MPI_Gather(samp, S, MPI_INT, temp, S, MPI_INT,0, MPI_COMM_WORLD);

    /* sort locally */
    qsort(vec, N, sizeof(int), compare);

    splitter = calloc(size-1, sizeof(int));

    //MASTER work only
    if (rank == 0){

    /* root processor does a sort, determinates splitters that
     * split the data into P buckets of approximately the same size */
        qsort(temp, S*size, sizeof(int), compare);
        for (j = 0; j < size-1; ++j) {
             splitter[j] = temp[(j+1)*S-1];
            if (print_splitters == 1)
             printf("These are the splitters: %d is: %d!\n", rank, j, splitter[j]);
        }

    /* root process broadcasts splitters */
    }
    MPI_Bcast(splitter, size-1, MPI_INT, MASTER, MPI_COMM_WORLD);

    /* every processor uses the obtained splitters to decide
     * which integers need to be sent to which other processor (local bins) */
    num_elements = calloc(size, sizeof(int));
    num_elements_all = calloc(size*size, sizeof(int));
    for (i = 0; i < size; ++i) {
        num_elements[i] = 0;
    }

    /* This huge nested loop that follows is determining how many elements are sent to which processors
       It is organized in a vector called num_elements on each processor, that is then sent to master 
       and broadcasted  back in a unified form via num_elements_all*/

    j = 0;
    counter = 0;
    elem_count = 0;
    for (i = 0; i < N; ++i) {       //go through the whole vector
        bln = 0;                    //boolean to get out of while loop
        while (bln == 0) {
            if (j < size-1){        //there are only size-1 splitters
                if (vec[i] < splitter[j]){  //if smaller then jth splitter count element and do not update j  
                    counter++;
                    bln = 1;
                }
                else {
                    bln2 = 0;           //boolean to get out of second while loop
                    while (bln2 == 0){          
                        if (vec[i] < splitter[j+1]){
                            num_elements[elem_count] = counter;
                            counter = 1;
                            elem_count++;
                            bln2 = 1;
                            bln = 1;
                            j++;
                            if (j > size-1){
                                j--;
                                bln = 1;
                                bln2 = 1;
                            }
                        }
                        else {
                            j++;
                            if (j > size-2){
                                j--;
                                bln = 1;
                                bln2 = 1;
                            }
                        }
            		}
                }
            }
            else if (j == size-1){
                counter++;
        	}
            else {   
                printf("j too big!!!\n");
            }
        }
    }
    num_elements[elem_count] = counter;
    sum = 0;
    
    for (i = 0; i < size-1; ++i) {
        sum = sum + num_elements[i];
    }
    num_elements[size-1] = N-sum;
    
    /*Tell each other how many elements will be sending to who
    First tell the master, then master broadcasts....could be done better but this works for now*/

    MPI_Gather(num_elements, size, MPI_INT, num_elements_all, size, MPI_INT,0, MPI_COMM_WORLD);
    
    MPI_Bcast(num_elements_all, size*size, MPI_INT, MASTER, MPI_COMM_WORLD);

    /*Now everybody should have info about how many elements it is send/receiving to/from who, so lets do it!*/
    
    MPI_Request reqs[size];
    MPI_Status stats[size];

    rrr =0; //this is to track number of requests for waitall command

    /*SEND to all processors their part*/
    sum = 0;
    for (i = 0; i < size; ++i) {
        if (num_elements_all[size*rank+i] > 0) {
            MPI_Isend(&vec[0]+sum, num_elements_all[size*rank+i], MPI_INT, i, 999, MPI_COMM_WORLD, &reqs[rrr]);
            rrr++;
            sum = sum+num_elements_all[size*rank+i];
        }
    }

    /*initialize the receiving array*/
    newvec_elements = 0;
    for (i = 0; i < size; ++i) {
        newvec_elements = newvec_elements+num_elements_all[size*i+rank];
    }
    newvec = calloc(newvec_elements, sizeof(int));

    /*RECEIVE from all processors their part*/
    sum = 0;
    for (i = 0; i < size; ++i) {
        if (num_elements_all[size*i+rank]>0) {
			MPI_Irecv(&newvec[0]+sum, num_elements_all[size*i+rank], MPI_INT, i, 999, MPI_COMM_WORLD, &reqs[rrr]);
            rrr++;
            sum = sum+num_elements_all[size*i+rank];
        }
    }

    MPI_Waitall(rrr, reqs, stats);

    /*DO FINAL SORT!!!*/
    qsort(newvec, newvec_elements, sizeof(int), compare);

    /*PRINT TO SCREEN FINAL RESULT*/
    if (print_final == 1){
        for (i = 0; i < newvec_elements; ++i) {
            //vec[i] = rand();
            printf("rank: %d, entry %i in newvec is: %d\n", rank, i, newvec[i]);
        }
    }

    if (print_finished == 1){
    printf("Finished!!!\n");
    }

    //Free stuff
    free(vec);
    free(samp);
    free(num_elements);
    free(num_elements_all);
    free(newvec);
    if (rank == 0){
        free(temp);
        free(splitter);
    }
    MPI_Finalize();
    return 0;
}
