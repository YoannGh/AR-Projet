#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define M 64
#define TAG_INIT 0
#define TAG_ASK 1
#define TAG_ANSWER 2
#define TAG_DISCONNECT 3
#define TAG_DISCONNECT_ACK 4
#define TAG_CONNECTED 5
#define TAG_STOP 6

#define PRECEDENT(i, count) ((i+(count)-1)%(count))
#define SUIVANT(i, count) ((i+(count)+1)%(count))

/* CODE DU SIMULATEUR */

static int cmpint(const void *a, const void *b)
{
	if(*(int*)a > *(int*)b)
		return 1;
	if(*(int*)a < *(int*)b)
		return -1;
	return 0;
}

int array_contains(const int *tab, const int size, const int to_ckeck)
{
	for (int i = 0; i<size; ++i)
	{
		if(tab[i] == to_ckeck)
			return 1;
	}
	return 0;
}

void generate_node_ids(int *tab, int nb_node)
{
	int random;
	memset(tab, -1, (nb_node)*sizeof(int));
	srand(time(NULL));
	for (int i = 0; i<nb_node; ++i)
	{
		do
		{
			random = rand()%M;
		}
		while(array_contains(tab, nb_node, random));
		tab[i] = random;
	}

	qsort(tab, nb_node, sizeof(int),cmpint);
	printf("[%d |", tab[0]);
	for(int i = 1; i < nb_node-1; i++)
	{
		printf(" %d |", tab[i]);
	}
	printf(" %d]", tab[nb_node-1]);
	printf("\n\n");
}

void simulateur(int nb_node) 
{
	/*Generation des CHORD id */
	int *tab = malloc(sizeof(int)*(nb_node));
	int mpi_next, first_data, data[2];
	generate_node_ids(tab, nb_node);
	char input[16];

	for (int i = 0; i < nb_node; ++i)
	{
		MPI_Send(&tab[i], 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
		first_data = tab[PRECEDENT(i, nb_node)] + 1;
		MPI_Send(&first_data, 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
		MPI_Send(&tab[SUIVANT(i, nb_node)], 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
		mpi_next = SUIVANT(i, nb_node) + 1;
		MPI_Send(&mpi_next, 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
	}
	while(1)
	{
		fgets(input, 16, stdin);
		if(input[0] == 'q')
		{
			for (int i = 1; i <= nb_node; ++i)
				MPI_Send(&data, 1, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
			break;
		}
		else if(input[0] == 's')
		{
			data[0] = atoi(&input[2]);
			data[1] = atoi(&input[5]);
			printf("Searching node %d from %d\n", data[0], data[1]);
			MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
		}
		else if(input[0] == 'd')
			MPI_Send(&data, 1, MPI_INT, atoi(&input[2]), TAG_DISCONNECT, MPI_COMM_WORLD);
		else if(input[0] == 'c')
			puts("TODO");
		else
			puts("Wrong input");
		
	}
	/*
	data[1] = 2;
	data[0] = 45;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 10;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 64;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	sleep(2);

	MPI_Send(&data, 1, MPI_INT, 1, TAG_DISCONNECT, MPI_COMM_WORLD);

	sleep(2);
	data[0] = 45;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 10;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 64;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);

	sleep(2);
	for (int i = 1; i <= nb_node; ++i)
		MPI_Send(&data, 1, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
	*/

	free(tab);
}

/* FIN CODE DU SIMULATEUR */

/* CODE DES NOEUDS */
int checkData(const int chord_id, const int first_data, const int to_check)
{
	if (chord_id >= first_data)
		return (to_check <= chord_id && to_check >= first_data);
	else
		return (to_check >= first_data || to_check <= chord_id);
}

void disconnect(MPI_Status status, int *chord_id, int *first_data, int *chord_next_node, int *mpi_next_node)
{
	int to_recv[4], to_send[4];
	if(status.MPI_SOURCE == 0) //Si c'est proc maitre qui nous demande de nous deconnecter
	{
		MPI_Recv(&to_recv, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
		to_send[0] = *chord_id;
		to_send[1] = *first_data;
		to_send[2] = *chord_next_node;
		to_send[3] = *mpi_next_node;
		MPI_Send(&to_send, 4, MPI_INT, *mpi_next_node, TAG_DISCONNECT, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Recv(&to_recv, 4, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
		//Si c'est le pair qui me precede qui se deco
		if(to_recv[0] == (*first_data-1))
			*first_data = to_recv[1];
		//Si c'est le pair qui me devance qui se deco
		if(to_recv[0] == *chord_next_node)
		{
			int save_old_mpi_node = *mpi_next_node; 
			*chord_next_node = to_recv[2];
			*mpi_next_node = to_recv[3];
			MPI_Send(&to_send, 1, MPI_INT, save_old_mpi_node, TAG_DISCONNECT_ACK, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Send(&to_recv, 4, MPI_INT, *mpi_next_node, TAG_DISCONNECT, MPI_COMM_WORLD);
		}
	}
}


void node(int rank) 
{
	int chord_id, first_data, chord_next_node, mpi_next_node;
	int to_recv[4], to_send[4];
	int run = 1;
	MPI_Status status;

	MPI_Recv(&chord_id, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&first_data, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&chord_next_node, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&mpi_next_node, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);

	printf("chord_id[%d] mpi_rank[%d] first_data: %d chord_next_node: %d mpi_next_node: %d\n", chord_id, rank, first_data, chord_next_node, mpi_next_node);

	while(run)
	{
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(status.MPI_TAG)
		{
			case TAG_ASK:
				MPI_Recv(&to_recv, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
				if(checkData(chord_id, first_data, to_recv[0]))
				{
					to_send[0] = to_recv[0];
					to_send[1] = chord_id;
					MPI_Send(&to_send, 2, MPI_INT, to_recv[1], TAG_ANSWER, MPI_COMM_WORLD);
				}
				else
					MPI_Send(&to_recv, 2, MPI_INT, mpi_next_node, TAG_ASK, MPI_COMM_WORLD);
				break;

			case TAG_ANSWER:
					MPI_Recv(&to_recv, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
					printf("chord_id[%d]: requested data %d is handle by %d\n", chord_id, to_recv[0], to_recv[1]);
				break;

			case TAG_DISCONNECT:
				disconnect(status, &chord_id, &first_data, &chord_next_node, &mpi_next_node);
				break;

			case TAG_DISCONNECT_ACK:
					MPI_Recv(&to_recv, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
					printf("old_chord_id[%d] mpi_rank[%d] properly disconnected\n", chord_id, rank);
					chord_id = -1;
				break;
			case TAG_CONNECTED:	
				break;

			case TAG_STOP:
				run = 0;
				break;

			default:
				printf("Error, tag unknown: %d\n", status.MPI_TAG);
		}
	}
	printf("chord_id[%d]/mpi_rank[%d] Killed\n", chord_id, rank);
}

/* FIN CODE DES NOEUDS */



int main (int argc, char* argv[]) {

   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc < 3 || nb_proc > M) {
      printf("Nombre de processus incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
   if (rang == 0) {
      simulateur(nb_proc-1);
   } else {
      node(rang);
   }
  
   MPI_Finalize();
   return 0;
}