#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

void simulateur(int nb_node) 
{
	printf("Simulateur %d nodes\n", nb_node);
}

void node(int rank) 
{
	printf("Node %d\n", rank);
}

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