#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGINIT 0
#define TAGCALC 1
#define NB_SITE 6

void simulateur(void) {
   int i;

   /* nb_voisins[i] est le nombre de voisins du site i */
   int nb_voisins[NB_SITE+1] = {-1, 2, 3, 2, 1, 1, 1};
   int min_local[NB_SITE+1] = {-1, 3, 11, 8, 14, 5, 17};

   /* liste des voisins */
   int voisins[NB_SITE+1][3] = {{-1, -1, -1},
         {2, 3, -1}, {1, 4, 5}, 
         {1, 6, -1}, {2, -1, -1},
         {2, -1, -1}, {3, -1, -1}};
                               
   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins[i],nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }
}

int indice_non_recu(int nb_voisin, int* recu) {

   for(int i = 0; i<nb_voisin; i++) {
      if(recu[i] == 0)
         return i;
   }

   return -1;
}

int nb_recu(int nb_voisin, int* recu) {
   int cpt = 0;
   for(int i = 0; i<nb_voisin; i++) {
      if(recu[i] == 1)
         cpt++;
   }
   return cpt;
}

void calcul_min(int rang) {

   int nb_voisin;
   int* voisins;
   int min_local;
   int min_calc;
   int min_recv;
   int* recu;
   int sent = 0;
   MPI_Status status;
   int q;
   int last;

   MPI_Recv(&nb_voisin, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(voisins, nb_voisin, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

   min_calc = min_local;

   recu = (int*) malloc(nb_voisin * sizeof(int));

   for(int i = 0; i<nb_voisin; i++) {
      recu[i] = 0;
   }

   while(nb_recu(nb_voisin, recu) != nb_voisin)
   {

      //r(p)
      if((!sent) && ((nb_voisin - nb_recu(nb_voisin, recu)) == 1)) {
         q = voisins[indice_non_recu(nb_voisin, recu)];
         MPI_Send(&min_calc, 1, MPI_INT, q, TAGCALC, MPI_COMM_WORLD);
         sent = 1;
         printf("[ Rank: %d ] envoie à : %d\n", rang, q);
      }

      //s(p)
      MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, TAGCALC, MPI_COMM_WORLD, &status);
      printf("[ Rank: %d ] Message recu de: %d\n", rang, status.MPI_SOURCE);
      if(min_recv < min_calc) {
         min_calc = min_recv;
      }
      for(int i = 0; i<nb_voisin; i++) {
         if(voisins[i] == status.MPI_SOURCE)
            recu[i] = 1;
      }
      last = status.MPI_SOURCE;
   }

   //Decision
   //Envoie à tous les voisin sauf le dernier qui nous a envoyé
   for(int i = 0; i<nb_voisin; i++) {
      if(voisins[i] != last)
         MPI_Send(&min_calc, 1, MPI_INT, voisins[i], TAGCALC, MPI_COMM_WORLD);
   }
   printf("[ Rank: %d ] Decision: %d\n", rang, min_calc);
   
}

/******************************************************************************/

int main (int argc, char* argv[]) {
   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc != NB_SITE+1) {
      printf("Nombre de processus incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
   if (rang == 0) {
      simulateur();
   } else {
      calcul_min(rang);
   }

   printf("[ Rank: %d ] FIN \n", rang);
   MPI_Finalize();
   printf("[ Rank: %d ] FIN2 \n", rang);
   return 0;
}
