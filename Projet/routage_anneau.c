#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define TAG_INIT 0
#define TAG_ASK 1
#define TAG_ANSWER 2
#define TAG_DISCONNECT 3
#define TAG_DISCONNECT_ACK 4
#define TAG_CONNECTED_NOTIFY_SUCCESSOR 5
#define TAG_CONNECTED_NOTIFY_PREDESSESSOR 6
#define TAG_CONNECTED_ACK 7
#define TAG_STOP 8

void display_help()
{
	printf("\n");
	printf("+---------------------------------H-E-L-P----------------------------------+\n");
	printf("|     Command     |          Fonction         |            Args            |\n");
	printf("|--------------------------------------------------------------------------|\n");
	printf("| s toSearch from | Search a data from a node |toSearch: Chord id to search|\n");
	printf("|                 |                           |from: Chord id searching    |\n");
	printf("|--------------------------------------------------------------------------|\n");
	printf("| h               | Diplay this help          | None                       |\n");
	printf("|--------------------------------------------------------------------------|\n");
	printf("| d chord_id      | Disconnect a chord id     | chord_id: id to disconnect |\n");
	printf("|--------------------------------------------------------------------------|\n");
	printf("| c mpi_id        | Connect a mpi id          | mpi_id: id to add          |\n");
	printf("|--------------------------------------------------------------------------|\n");
	printf("| p               | Print the ring            | None                       |\n");
	printf("|--------------------------------------------------------------------------|\n");
	printf("| q               | Quit, kill everyone       | None                       |\n");
	printf("+--------------------------------------------------------------------------+\n");
	printf("\n");
}

void routine(int nb_node, struct nodeIDs *correspond)
{
	int data[2];

	data[1] = 2;
	data[0] = 45;
	printf("*** Random search *** \n");
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 10;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 64;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	sleep(2);
	
	printf("\n*** Disconnect MPI 1 *** \n");
	MPI_Send(&data, 1, MPI_INT, 1, TAG_DISCONNECT, MPI_COMM_WORLD);
	sleep(2);
	
	printf("\n*** Random search *** \n");
	data[0] = 45;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 10;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 64;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	sleep(2);

	printf("\n*** Reconnect MPI 1 *** \n");

	data[0] = getUniqueId(correspond->chord_ids, nb_node);
	data[1] = 1; 
	printf("New chord_id[%d] mpi_rank[%d] connecting ...\n", data[0], data[1]);
	MPI_Send(&data, 2, MPI_INT, getRandomMPIOfConnectedPeer(correspond), TAG_CONNECTED_NOTIFY_SUCCESSOR, MPI_COMM_WORLD);
	sleep(2);

	printf("\n*** Random search *** \n");
	data[0] = 45;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 10;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	data[0] = 64;
	MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
	sleep(2);

	printf("\n*** Kill everyone *** \n");
	for (int i = 1; i <= nb_node; ++i)
		MPI_Send(&data, 1, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
}

void prompt(int nb_node, struct nodeIDs *correspond)
{
	char input[16];
	int data[2];
	char command;
	int identificator, origin_chord_ids;

	display_help();
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
			sscanf(input, "%c %d %d", &command, &identificator, &origin_chord_ids);
			data[0] = identificator;
			data[1] = chordToMpi(correspond, origin_chord_ids);
			if(data[1] == -1)
				puts("Chord ID not found");
			else
				MPI_Send(&data, 2, MPI_INT, data[1], TAG_ASK, MPI_COMM_WORLD);
		}
		else if(input[0] == 'd') {
			sscanf(input, "%c %d", &command, &identificator);
			if( (identificator = removeChordID(correspond, identificator)) != -1)
				MPI_Send(&data, 1, MPI_INT, identificator, TAG_DISCONNECT, MPI_COMM_WORLD);
			else
				puts("Chord ID not found");

		}
		else if(input[0] == 'c') 
		{
			sscanf(input, "%c %d", &command, &identificator);
			data[0] = getUniqueId(correspond->chord_ids, nb_node); // Nouveau CHORD ID du nouveau pair
			data[1] = identificator; // Le Rank MPI du pair se connectant

			if(addChordID(correspond, data[1], data[0]) == 0)
			{
				printf("New chord_id[%d] mpi_rank[%d] connecting ...\n", data[0], data[1]);
				MPI_Send(&data, 2, MPI_INT, getRandomMPIOfConnectedPeer(correspond), TAG_CONNECTED_NOTIFY_SUCCESSOR, MPI_COMM_WORLD);
			}
			else
				puts("Wrong MPI_rank given (already connected or non-existing");
		}
		else if(input[0] == 'p')
			printRing(correspond);
		else if(input[0] == 'h')
			display_help();
		else
			puts("Wrong input");
		
	}
}

/*
	Code du simulateur (MPI ID = 0)
*/

void simulateur(int nb_node) 
{
	/*Generation des CHORD id */
	//int *tab = malloc(sizeof(int)*(nb_node));
	int mpi_next, first_data;
	struct nodeIDs correspond;

	correspond.nb_node = nb_node;
	correspond.mpi_ids = malloc(sizeof(int)*(nb_node));
	correspond.chord_ids = malloc(sizeof(int)*(nb_node));
	
	memset(correspond.mpi_ids, -1, sizeof(int)*(nb_node));
	memset(correspond.chord_ids, -1, sizeof(int)*(nb_node));

	generate_node_ids(correspond.chord_ids, nb_node);

	for (int i = 0; i < nb_node; ++i)
	{
		correspond.mpi_ids[i] = i+1;
		MPI_Send(&correspond.chord_ids[i], 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
		first_data = correspond.chord_ids[PRECEDENT(i, nb_node)] + 1;
		MPI_Send(&first_data, 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
		MPI_Send(&correspond.chord_ids[SUIVANT(i, nb_node)], 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
		mpi_next = SUIVANT(i, nb_node) + 1;
		MPI_Send(&mpi_next, 1, MPI_INT, i+1, TAG_INIT, MPI_COMM_WORLD);
	}

	printRing(&correspond);

	/*********
	 * Mettre en commentaire une des deux lignes suivantes  
	 * pour changer en mode interractif / automatique
	 ********/

	//prompt(nb_node, &correspond);
	routine(nb_node, &correspond);

	free(correspond.mpi_ids);
	free(correspond.chord_ids);
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

void disconnect(MPI_Status status, int chord_id, int *first_data, int *chord_next_node, int *mpi_next_node)
{
	int to_recv[4], to_send[4];
	if(status.MPI_SOURCE == 0) //Si c'est proc maitre qui nous demande de nous deconnecter
	{
		MPI_Recv(&to_recv, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
		to_send[0] = chord_id;
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

void connect_notify_predessessor(MPI_Status status, int *chord_id, int *chord_next_node, int *mpi_next_node)
{
	int to_recv[4], to_send[4];
	MPI_Recv(&to_recv, 4, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
	if(*chord_next_node == to_recv[2])
	{
		*mpi_next_node = to_recv[1];
		*chord_next_node = to_recv[0];

		printf("chord_id[%d]: new chord_next_node:[%d], mpi_next_node[%d]\n", *chord_id, *chord_next_node, *mpi_next_node);

		//Envoie des inforation au nouveau pair
		to_send[0] = to_recv[0];	  // Nouveau CHORD ID du nouveau pair
		to_send[1] = to_recv[3];	  // mpi_next_node
		to_send[2] = to_recv[2]; 	  // chord_next_node
		to_send[3] = (*chord_id) + 1; // first_data
		MPI_Send(&to_send, 4, MPI_INT, to_recv[1], TAG_CONNECTED_ACK, MPI_COMM_WORLD);		
	}
	else
		MPI_Send(&to_recv, 4, MPI_INT, *mpi_next_node, TAG_CONNECTED_NOTIFY_PREDESSESSOR, MPI_COMM_WORLD);
}

void connect_notify_successor(MPI_Status status, int *chord_id, int rank, int *first_data, int *mpi_next_node)
{
	int to_recv[2], to_send[4];
	MPI_Recv(&to_recv, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
	if(checkData(*chord_id, *first_data, to_recv[0]))
	{
		*first_data = SUIVANT(to_recv[0],M);
		printf("chord_id[%d]: rangeDown because of new_chord_id[%d], new first_data: %d\n", *chord_id, to_recv[0], *first_data);


		//Envoie des inforation a l'anneau à la recherche du predecesseur
		to_send[0] = to_recv[0]; // Nouveau CHORD ID du nouveau pair
		to_send[1] = to_recv[1]; // Le Rank MPI du pair se connectant
		to_send[2] = *chord_id;	 // Le CHORD ID du successeur du nouveau pair
		to_send[3] = rank; 		 // Le MPI ID du sucesseur du nouveau pair
		MPI_Send(&to_send, 4, MPI_INT, *mpi_next_node, TAG_CONNECTED_NOTIFY_PREDESSESSOR, MPI_COMM_WORLD);		
	}
	else
		MPI_Send(&to_recv, 2, MPI_INT, *mpi_next_node, TAG_CONNECTED_NOTIFY_SUCCESSOR, MPI_COMM_WORLD);

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

	//printf("chord_id[%d] mpi_rank[%d] first_data: %d chord_next_node: %d mpi_next_node: %d\n", chord_id, rank, first_data, chord_next_node, mpi_next_node);

	while(run)
	{
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(status.MPI_TAG)
		{
			case TAG_ASK:
				MPI_Recv(&to_recv, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
				if(status.MPI_SOURCE == 0)
					printf("Searching node %d from %d\n", to_recv[0], chord_id);

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
					printf("chord_id[%d]: requested data %d is handled by %d\n", chord_id, to_recv[0], to_recv[1]);
				break;

			case TAG_DISCONNECT:
				disconnect(status, chord_id, &first_data, &chord_next_node, &mpi_next_node);
				break;

			case TAG_DISCONNECT_ACK:
					MPI_Recv(&to_recv, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
					printf("old_chord_id[%d] mpi_rank[%d] properly disconnected\n", chord_id, rank);
					chord_id = -1;
				break;
			case TAG_CONNECTED_NOTIFY_SUCCESSOR:	
				connect_notify_successor(status, &chord_id, rank, &first_data, &mpi_next_node);
				break;

			case TAG_CONNECTED_NOTIFY_PREDESSESSOR:	
				connect_notify_predessessor(status, &chord_id, &chord_next_node, &mpi_next_node);
				break;

			case TAG_CONNECTED_ACK:	
				MPI_Recv(&to_recv, 4, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
				chord_id = to_recv[0]; // Nouveau CHORD ID du nouveau pair
				mpi_next_node = to_recv[1]; // mpi_next_node
				chord_next_node = to_recv[2]; // chord_next_node
				first_data = to_recv[3]; 	 // first_data
				printf("chord_id[%d] mpi_rank[%d] properly connected ! mpi_next_node = %d, chord_next_node = %d, first_data = %d\n", chord_id, rank, mpi_next_node, chord_next_node, first_data);
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

   int nb_proc, rank;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc < 3 || nb_proc > M) {
      printf("Le nombre de processus MPI doit être compris entre %d et %d\n", 3, M);
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
   if (rank == 0) {
      simulateur(nb_proc-1);
   } else {
      node(rank);
   }
  
   MPI_Finalize();
   return 0;
}