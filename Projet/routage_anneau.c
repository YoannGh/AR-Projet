#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define M 64
#define TAGINIT 0
#define TAGASK 1
#define TAGANSWER 2

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
	for(int i = 0; i < nb_node; i++)
	{
		printf(" %d :", tab[i]);
	}
	printf("\n\n");
}

void simulateur(int nb_node) 
{
	/*Generation des CHORD id */
	int *tab = malloc(sizeof(int)*(nb_node));
	int mpi_next, first_data;
	generate_node_ids(tab, nb_node);

	for (int i = 0; i < nb_node; ++i)
	{
		MPI_Send(&tab[i], 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		first_data = tab[PRECEDENT(i, nb_node)] + 1;
		MPI_Send(&first_data, 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(&tab[SUIVANT(i, nb_node)], 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		mpi_next = SUIVANT(i, nb_node) + 1;
		MPI_Send(&mpi_next, 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
	}

	free(tab);
}

/* FIN CODE DU SIMULATEUR */

/* CODE DES NOEUDS */

void node(int rank) 
{
	int chord_id, first_data, chord_next_node, mpi_next_node;
	MPI_Status status;

	MPI_Recv(&chord_id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&first_data, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&chord_next_node, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&mpi_next_node, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	printf("rank: %d chord_id: %d first_data: %d chord_next_node: %d mpi_next_node: %d\n", rank, chord_id, first_data, chord_next_node, mpi_next_node);

	if(rank == 4)
	{
		int to_send[2];
		int have_data;
		to_send[0] = 5; //Ressource demandée
		to_send[1] = 4; //MPI_ID du demandeur
		MPI_Send(&to_send, 2, MPI_INT, mpi_next_node, TAGASK, MPI_COMM_WORLD);
		MPI_Recv(&have_data, 1, MPI_INT, MPI_ANY_SOURCE, TAGANSWER, MPI_COMM_WORLD, &status);
		printf("Rank[%d] recherche %d qui est detunue par le pair %d\n", rank, to_send[0], have_data);
	}
	else
	{
		int to_recv[2];
		MPI_Recv(&to_recv, 2, MPI_INT, MPI_ANY_SOURCE, TAGASK, MPI_COMM_WORLD, &status);
		printf("rank: %d to_recv[0]: %d to_recv[1]: %d\n", rank, to_recv[0], to_recv[1]);
		if(to_recv[0] <= chord_id && to_recv[0] >= first_data) {
			MPI_Send(&chord_id, 1, MPI_INT, to_recv[1], TAGANSWER, MPI_COMM_WORLD);
		}
		else {
			MPI_Send(&to_recv, 2, MPI_INT, mpi_next_node, TAGASK, MPI_COMM_WORLD);
		}

	}
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