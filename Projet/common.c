#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

/*
	Fonction pour quicksort sur un tableau d'entier
*/
int cmpint(const void *a, const void *b)
{
	if(*(int*)a > *(int*)b)
		return 1;
	if(*(int*)a < *(int*)b)
		return -1;
	return 0;
}


//Retourne l'indice de tofind si il exsite, -1 sinon
int find_in_array(int *array, int array_size, int tofind)
{
	for(int i = 0; i < array_size; ++i)
	{
		if(array[i] == tofind)
			return i;
	}
	return -1;
}

int getUniqueId(int *tab, int tab_size)
{
	int random;
	do
	{
		random = rand()%M;
	}
	while(find_in_array(tab, tab_size, random) != -1);
	return random;
}

/*
	Affiche les pairs connectés, ils ne sont pas forcément classés
	en fonction de leur CHORD ID 
*/
void printRing(struct nodeIDs *correspond)
{
	printf("Ring: ");
	printf("[|");
	for(int i = 0; i < correspond->nb_node; i++)
	{
		if(correspond->chord_ids[i] != -1)
			printf(" %d |", correspond->chord_ids[i]);
	}
	printf("]\n");
	printf("\n");
}

/*
	Génération des clés CHORD dans l'ensemble [0;M[, 
	les clés sont tirées aléatoirement et de manière unique 
*/
void generate_node_ids(int *tab, int nb_node)
{
	srand(time(NULL));
	for (int i = 0; i<nb_node; ++i)
	{
		tab[i] = getUniqueId(tab, nb_node);
	}

	qsort(tab, nb_node, sizeof(int),cmpint);
}

/*
	Correspondance entre une clé CHORD (chord_id) et un identifiant MPI
	return: l'identifiant MPI de la clé CHORD, 
			ou -1 si la clé n'existe pas
*/
int chordToMpi(struct nodeIDs *correspond, int chord_id)
{
	for (int i = 0; i < correspond->nb_node; ++i)
	{
		if(correspond->chord_ids[i] == chord_id)
			return correspond->mpi_ids[i];
	}
	return -1;
}

/*
	Retire correspondance entre une clé CHORD (chord_id) et un identifiant MPI
	return: l'identifiant MPI de la clé CHORD retirée, 
			ou -1 si la clé n'existe pas
*/
int removeChordID(struct nodeIDs *correspond, int chord_id)
{
	for (int i = 0; i < correspond->nb_node; ++i)
	{
		if(correspond->chord_ids[i] == chord_id)
		{
			correspond->chord_ids[i] = -1;
			return correspond->mpi_ids[i];
		}
	}
	return -1;
}

int addChordID(struct nodeIDs *correspond, int mpi_id, int new_chord_id)
{
	for (int i = 0; i < correspond->nb_node; ++i)
	{
		if(correspond->mpi_ids[i] == mpi_id)
		{
			if(correspond->chord_ids[i] != -1)
				return -1;

			correspond->chord_ids[i] = new_chord_id;
			return 0;
		}
	}
	return -1;
}

int getRandomMPIOfConnectedPeer(struct nodeIDs *correspond)
{
	int random;
	//On cherche un processus MPI deja connecté en tant que pair
	do
	{
		random = rand()%correspond->nb_node;
	}
	while(correspond->chord_ids[random] == -1);

	return correspond->mpi_ids[random]; 
}