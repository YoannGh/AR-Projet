#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define TAGINIT    0
#define TAGCALC 1
#define NB_SITE 6

#define DIAMETRE	5

void simulateur(void) {
   int i;

   /* nb_voisins_in[i] est le nombre de voisins entrants du site i */
   /* nb_voisins_out[i] est le nombre de voisins sortants du site i */
   int nb_voisins_in[NB_SITE+1] = {-1, 2, 1, 1, 2, 1, 1};
   int nb_voisins_out[NB_SITE+1] = {-1, 2, 1, 1, 1, 2, 1};

   int min_local[NB_SITE+1] = {-1, 4, 7, 1, 6, 2, 9};

   /* liste des voisins entrants */
   int voisins_in[NB_SITE+1][2] = {{-1, -1},
				{4, 5}, {1, -1}, {1, -1},
				{3, 5}, {6, -1}, {2, -1}};
                               
   /* liste des voisins sortants */
   int voisins_out[NB_SITE+1][2] = {{-1, -1},
				{2, 3}, {6, -1}, {4, -1},
				{1, -1}, {1, 4}, {5,-1}};

   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins_in[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&nb_voisins_out[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins_in[i], nb_voisins_in[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins_out[i], nb_voisins_out[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }
}

int verif(int *rcount, int nb_voisins_in, int scount)
{
   for(int i = 0; i<nb_voisins_in;++i)
   {
      if(rcount[i] != DIAMETRE)
         return 0;
   }
   return scount == DIAMETRE ? 1:0;
}

int can_send(int *rcount, int nb_voisins_in, int scount)
{
   for(int i = 0; i<nb_voisins_in;++i)
   {
      if(rcount[i] < scount)
         return 0;
   }
   return scount < DIAMETRE ? 1:0;
}

int can_decide(int *rcount, int nb_voisins_in)
{
   for(int i = 0; i<nb_voisins_in;++i)
   {
      if(rcount[i] < DIAMETRE)
         return 0;
   }
   return 1;
}

void calcul_min(int rang) {

   int nb_voisins_in;
   int nb_voisins_out;
   int* voisins_in;
   int* voisins_out;
   int min_local;
   int min_calc;
   int min_recv;
   int* rcount;
   int scount = 0;
   int decision_taken = 0;
   MPI_Status status;

   MPI_Recv(&nb_voisins_in, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(&nb_voisins_out, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   voisins_in = malloc(nb_voisins_in*sizeof(int));
   voisins_out = malloc(nb_voisins_out*sizeof(int));
   MPI_Recv(voisins_in, nb_voisins_in, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(voisins_out, nb_voisins_out, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);


   rcount = malloc(nb_voisins_in*sizeof(int));
   memset(rcount, '\0', nb_voisins_in*sizeof(int));

   min_calc = min_local;

   while(!verif(rcount,nb_voisins_in,scount))
   {
      //Send
      while(can_send(rcount,nb_voisins_in,scount))
      {
         for(int i = 0; i<nb_voisins_out;++i)
         {
            printf("[ Rank: %d ] envoie du min à : %d\n", rang, voisins_out[i]);
            MPI_Send(&min_calc, 1, MPI_INT, voisins_out[i], TAGCALC, MPI_COMM_WORLD);
         }
         ++scount;         
      }
      //Recv
      MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, TAGCALC, MPI_COMM_WORLD, &status);
      printf("[ Rank: %d ] min recu de : %d\n", rang, status.MPI_SOURCE);
      if(min_recv<min_calc)
         min_calc = min_recv;

      for(int i = 0; i<nb_voisins_in; i++) {
         if(voisins_in[i] == status.MPI_SOURCE)
            rcount[i]++;
      }

      //Decision
      if(can_decide(rcount,nb_voisins_in) && !decision_taken)
      {
         printf("[ Rank: %d ] decision finale: : %d\n", rang, min_calc);
         decision_taken++;
      }
   }
   free(rcount);
   free(voisins_in);
   free(voisins_out);
}

int* rcount;
int nb_voisins_in;
int nb_voisins_out;
int scount = 0;
int rang;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condSend = PTHREAD_COND_INITIALIZER;

void* sender(void* arg) {
   puts("Sender started");

   pthread_mutex_lock(&mutex);
   while(scount < DIAMETRE) {
      while(!can_send(rcount,nb_voisins_in,scount))
      {
         pthread_cond_wait(&condSend, &mutex);      
      }
      for(int i = 0; i<nb_voisins_out;++i)
      {
            printf("[ Rank: %d ] envoie du min à : %d\n", rang, voisins_out[i]);
            MPI_Send(&min_calc, 1, MPI_INT, voisins_out[i], TAGCALC, MPI_COMM_WORLD);
      }
      ++scount;
   }
   return NULL;
}

void* receiver(void* arg) {
   puts("receiver started");

   
   return NULL;
}

void calcul_min_threaded(int rangr) {

   
   
   int* voisins_in;
   int* voisins_out;
   int min_local;
   int min_calc;
   int min_recv;
   
   int decision_taken = 0;
   MPI_Status status;

   pthread_t send_tid;
   pthread_t receive_tid;

   rang = rangr;

   MPI_Recv(&nb_voisins_in, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(&nb_voisins_out, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   voisins_in = malloc(nb_voisins_in*sizeof(int));
   voisins_out = malloc(nb_voisins_out*sizeof(int));
   MPI_Recv(voisins_in, nb_voisins_in, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(voisins_out, nb_voisins_out, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

   rcount = malloc(nb_voisins_in*sizeof(int));
   memset(rcount, '\0', nb_voisins_in*sizeof(int));

   if(pthread_create(&send_tid, NULL, sender, NULL) != 0) {
         puts("ERROR pthread_create");
   }

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
      calcul_min_threaded(rang);
   }
  
   MPI_Finalize();
   return 0;
}
