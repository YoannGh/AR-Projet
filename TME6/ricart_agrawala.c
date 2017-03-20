#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MAX_CS 3
#define REPLY 0
#define REQUEST 1
#define DONE 2

typedef enum {NOT_REQUESTING, REQUESTING, CS} type_etat;

int ppetit(int rank, int source_rank, int local_clock, int receive_clock) {
	return ((receive_clock < local_clock) || ((receive_clock == local_clock) && (source_rank < rank)));
}

int main(int argc, char *argv[])
{
	
	int nb_proc, rang, nb_cs, clock;
	int total_request_recv = 0;
	int tmp;
	type_etat etat;
	int date_requested;
	int nb_reply = 0;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);

	int *queue;
	int add_queue = 0;
	int cpt_queue = 0;
	queue = malloc(sizeof(int)*(nb_proc-1));


	while(nb_cs != MAX_CS)
	{
		//request_CS
		clock++;
		etat = REQUESTING;
		date_requested = clock;
		nb_reply = 0;
		for(int i =0; i< nb_proc; ++i)
		{
			if(i != rang)
			{
				MPI_Send(&clock, 1, MPI_INT, i, REQUEST, MPI_COMM_WORLD);
	         	printf("[ Rank: %d ] envoie REQUEST à : %d\n", rang, i);
			}
		}
		//Receive
		while(nb_reply != (nb_proc-1))
		{
			MPI_Recv(&tmp, 1, MPI_INT , MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			clock = max(tmp, clock);
			clock++;
			switch(status.MPI_TAG)
			{
				case REPLY:
					printf("[ Rank: %d ] recoit REPLY de : %d\n", rang, status.MPI_SOURCE);
					nb_reply++;
					if(nb_reply == nb_proc -1)
						etat = CS;
					break;
				case REQUEST:
					total_request_recv++;
					printf("[ Rank: %d ] recoit REQUEST de : %d\n", rang, status.MPI_SOURCE);
					if( (etat == NOT_REQUESTING) || ((etat == REQUESTING) && ppetit(rang, status.MPI_SOURCE, clock, tmp))  )
					{
						MPI_Send(&clock, 1, MPI_INT, status.MPI_SOURCE, REPLY, MPI_COMM_WORLD);
	         			printf("[ Rank: %d ] envoie REPLY à : %d\n", rang, status.MPI_SOURCE);
					}
					else
					{
						queue[add_queue] = status.MPI_SOURCE;
						add_queue++;
						cpt_queue++;
					}
					break;
				default:
					printf("PANIC SWITCH status.MPI_TAG = %d\n", status.MPI_TAG);
			}
		}
	    printf("[ Rank: %d ] en CS §§ : %d\n", rang);
	    //release
	    clock++;
	    etat = NOT_REQUESTING;
		for(int i =0; i< cpt_queue; ++i)
		{
			MPI_Send(&clock, 1, MPI_INT, queue[i], REPLY, MPI_COMM_WORLD);
		}
		add_queue = 0;
		cpt_queue = 0;
		nb_cs++;
	}
	while(total_request_recv != (nb_proc-1)*MAX_CS)
	{
		MPI_Recv(&tmp, 1, MPI_INT , MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		total_request_recv++;
		clock = max(tmp, clock);
		clock++;
		MPI_Send(&clock, 1, MPI_INT, status.MPI_SOURCE, REPLY, MPI_COMM_WORLD);
		printf("[ Rank: %d ] envoie REPLY à : %d\n", rang, status.MPI_SOURCE);
	}
	printf("[ Rank: %d ] terminé\n", rang);

	free(queue);
	MPI_Finalize();

	return 0;
}