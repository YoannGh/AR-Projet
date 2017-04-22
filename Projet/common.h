#pragma once

#define M 64

#define PRECEDENT(i, count) ((i+(count)-1)%(count))
#define SUIVANT(i, count) ((i+(count)+1)%(count))

struct nodeIDs {
	int *mpi_ids;
	int *chord_ids;
	int nb_node;
};

int cmpint(const void *a, const void *b);
int find_in_array(int *array, int array_size, int tofind);
int getUniqueId(int *tab, int size);
void printRing(struct nodeIDs *correspond);
void generate_node_ids(int *tab, int nb_node);
int chordToMpi(struct nodeIDs *correspond, int chord_id);
int removeChordID(struct nodeIDs *correspond, int chord_id);
int addChordID(struct nodeIDs *correspond, int mpi_id, int new_chord_id);
int getRandomMPIOfConnectedPeer(struct nodeIDs *correspond);
