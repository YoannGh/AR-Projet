#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAGINIT 0
#define TAGCALC 1
#define NB_SITE 6
#define DOWN 8
#define UP 9
#define END 10

void simulateur(void) {
   int i;

   /* nb_voisins[i] est le nombre de voisins du site i */
   int nb_voisins[NB_SITE+1] = {-1, 3, 3, 2, 3, 5, 2};
   int min_local[NB_SITE+1] = {-1, 12, 11, 8, 14, 5, 17};

   /* liste des voisins */
   int voisins[NB_SITE+1][5] = {{-1, -1, -1, -1, -1},
            {2, 5, 3, -1, -1}, {4, 1, 5, -1, -1}, 
            {1, 5, -1, -1, -1}, {6, 2, 5, -1, -1},
            {1, 2, 6, 4, 3}, {4, 5, -1, -1, -1}};
                               
   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins[i], nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }
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
   int pere = -1;
   int *fils;


   MPI_Recv(&nb_voisin, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(voisins, nb_voisin, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

   min_calc = min_local;

   recu = (int*) malloc(nb_voisin * sizeof(int));
   fils = (int*) malloc(nb_voisin * sizeof(int));

   memset(fils, '\0', nb_voisin);
   memset(recu, '\0', nb_voisin);

   //Init
   if(rang == 1)
   {
      for(int i = 0; i<nb_voisin; i++)
         MPI_Send(&min_calc, 1, MPI_INT, voisins[i], DOWN, MPI_COMM_WORLD);

      for(int i = 0; i<nb_voisin; i++)
      {
         MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         if(min_recv < min_calc)
            min_calc = min_recv;
         if(status.MPI_TAG == UP)
         {  
            for(int i = 0; i<nb_voisin; i++) {
               if(voisins[i] == status.MPI_SOURCE)
               {
                  printf("[ INIT Rank: %d ] mes fils : %d\n", rang, status.MPI_SOURCE);
                  fils[i] = 1;
               }
               else
                  printf("[ INIT Rank: %d ] pas mon fils : %d\n", rang, status.MPI_SOURCE);
            }
         }
      }
      
      printf("[ INIT Rank: %d ] Decision: %d\n", rang, min_calc);
      for(int i = 0; i<nb_voisin; i++) {
         if(fils[i])
         {
            MPI_Send(&min_calc, 1, MPI_INT, voisins[i], END, MPI_COMM_WORLD);
         }
      }
      return;
   }
   //Non init
   else
   {
      while(nb_recu(nb_voisin, recu) != nb_voisin)
      {
         MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         printf("[ Rank: %d ] message reçu de : %d\n", rang, status.MPI_SOURCE);
         //Met à 1 l'envoyeur
         for(int i = 0; i<nb_voisin; i++) {
            if(voisins[i] == status.MPI_SOURCE)
               recu[i] = 1;
         }
         //maj min
         if(min_recv < min_calc)
            min_calc = min_recv;
         if(pere == -1)
         {
            pere = status.MPI_SOURCE;
            //Envoi à tous les autres
            for(int i = 0; i<nb_voisin; i++) {
               if(voisins[i] != pere)
                  MPI_Send(&min_calc, 1, MPI_INT, voisins[i], TAGCALC, MPI_COMM_WORLD);
            }
         }
         if(status.MPI_TAG == UP)
         {
            for(int i = 0; i<nb_voisin; i++) {
               if(voisins[i] == status.MPI_SOURCE)
               {
                  printf("[ Rank: %d ] mes fils : %d\n", rang, status.MPI_SOURCE);
                  fils[i] = 1;
               }
               else
                  printf("[ Rank: %d ] pas mon fils : %d\n", rang, status.MPI_SOURCE);
            }    
         }
      }

      MPI_Send(&min_calc, 1, MPI_INT, pere, UP, MPI_COMM_WORLD);
      printf("[ Rank: %d ] envoie à mon papa : %d\n", rang, pere);
   }
   
   MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, END, MPI_COMM_WORLD, &status);
   if(min_recv < min_calc)
      min_calc = min_recv;
   printf("[ Rank: %d ] recu decision de papa : %d\n", rang, min_calc);
   for(int i = 0; i<nb_voisin; i++) {
         if(fils[i])
         {
            MPI_Send(&min_calc, 1, MPI_INT, voisins[i], END, MPI_COMM_WORLD);
         }
      }

}

/******************************************************************************/

int main (int argc, char* argv[]) {
   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);

   if (nb_proc != NB_SITE+1) {
      printf("Nombre de processus incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
  
  
   if (rang == 0) {
      simulateur();
   } else {
      calcul_min(rang);
   }
  
   MPI_Finalize();
   return 0;
}
