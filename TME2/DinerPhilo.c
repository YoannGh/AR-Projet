#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>

/* Q1
	Message de la forme (horloge, id)

	Ordre total:
	2 evenements, e et f
	e < f si e et f sont sur i et h(e) < h(f)
	e < f <=> (h(e) < h(f)) OR (h(e) == h(f) AND i < j)

	Besoin d'un TAG pour message de différents types:
		WANNA_CHOPSTICK (je veux la baguette)
		CHOPSTICK_YOURS
		DONE_EATING (tous les NB_MEALS ont été mangé)
	2 variables:
		CHOPSTICK_LEFT (boolean)
		CHOPSTICK_RIGHT (boolean)
	1 variable état:
		initialisée à THINKING

	Q2 
	Possibilité d'interblocage/famine

	Q3
	Finit de manger et avoir rendu les baguettes et que ses 2 voisins
	aient fini de manger.
*/

typedef enum {THINKING, HUNGRY, EATING, DONE_EATING} etat_enum;

int ppetit(int rank, int source_rank, int local_clock, int receive_clock) {
	return ((receive_clock < local_clock) || ((receive_clock == local_clock) && (source_rank < rank)));
}

int check_terminaison(etat_enum local_state, etat_enum left_state, etat_enum right_state) {
	return ((local_state != DONE_EATING) || (left_state != DONE_EATING) || (right_state != DONE_EATING));
}


int NB_MEALS = 3;
#define WANNA_CHOPSTICK 99
#define CHOPSTICK_YOURS 98
#define DONE 97

int main(int argc, char *argv[])
{

	int rank, size, voisin_gauche, voisin_droite;
	etat_enum local_state = THINKING;
	etat_enum left_state = THINKING;
	etat_enum right_state = THINKING;
	int chopstick_left = 0;
	int chopstick_right = 0;
	int local_clock = 0;
	int meal_eaten = 0;
	int receive_clock;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	voisin_gauche = (rank+size-1)%size;
	voisin_droite = (rank+size+1)%size;

	while(check_terminaison(local_state, left_state, right_state)) {
		printf("[ Rank: %d Clock: %d ] CheckTerm local:%d, left:%d, right:%d\n", rank, local_clock ,local_state, left_state, right_state);


		if((meal_eaten < NB_MEALS)) {
			local_state = HUNGRY;
			printf("[ Rank: %d Clock: %d ] A faim\n", rank, local_clock);
			local_clock++;
			MPI_Send(&local_clock, 1, MPI_INT ,voisin_gauche, WANNA_CHOPSTICK, MPI_COMM_WORLD);
			local_clock++;
			MPI_Send(&local_clock, 1, MPI_INT ,voisin_droite, WANNA_CHOPSTICK, MPI_COMM_WORLD);
		}

		printf("[ Rank: %d Clock: %d ] Avant receive\n", rank, local_clock);
		MPI_Recv(&receive_clock, 1, MPI_INT , MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(receive_clock > local_clock) {
			local_clock = receive_clock;
		}
		local_clock++;

		if(status.MPI_TAG == DONE) {
			if(status.MPI_SOURCE == voisin_gauche) {
				left_state = DONE_EATING;
				printf("[ Rank: %d Clock: %d ] Done gauche\n", rank, local_clock);
			}
			else if(status.MPI_SOURCE == voisin_droite) {
				right_state = DONE_EATING;
				printf("[ Rank: %d Clock: %d ] Done Right\n", rank, local_clock);
			} 
			else 
				printf("[ Rank: %d Clock: %d ] Done ERROR\n", rank, local_clock);
		}
		else if(status.MPI_TAG == CHOPSTICK_YOURS && local_state != DONE_EATING) {
			if(status.MPI_SOURCE == voisin_gauche) {
				chopstick_left = 1;
				printf("[ Rank: %d Clock: %d ] Baguette gauche recue\n", rank, local_clock);
			}
			else if(status.MPI_SOURCE == voisin_droite) {
				chopstick_right = 1;
				printf("[ Rank: %d Clock: %d ] Baguette droite recue\n", rank, local_clock);
			}

			if(chopstick_left && chopstick_right) {
				local_state = EATING;
				meal_eaten++;
				if(meal_eaten == NB_MEALS)
				{
					printf("[ Rank: %d Clock: %d ] A fini ses repas\n", rank, local_clock);
					local_state = DONE_EATING;
					local_clock++;
					MPI_Send(&local_clock, 1, MPI_INT, voisin_gauche, DONE, MPI_COMM_WORLD);
					local_clock++;
					MPI_Send(&local_clock, 1, MPI_INT, voisin_droite, DONE, MPI_COMM_WORLD);
					
				}
				printf("[ Rank: %d Clock: %d ] Mange son repas num %d!\n", rank, local_clock, meal_eaten);
			}
		}
		else if(status.MPI_TAG == WANNA_CHOPSTICK) {
			if((local_state != HUNGRY) || ppetit(rank, status.MPI_SOURCE, local_clock, receive_clock)) {
				local_clock++;
				MPI_Send(&local_clock, 1, MPI_INT , status.MPI_SOURCE, CHOPSTICK_YOURS, MPI_COMM_WORLD);
			}
		}
		else {
			printf("[ Rank: %d Clock: %d ] TAG ERROR: %d\n", rank, local_clock, status.MPI_TAG);
		}
	}

	printf("[ Rank: %d Clock: %d ] FIN\n", rank, local_clock);

	MPI_Finalize();

}