#include <stdio.h>
#include <mpi.h>
#include <string.h>


int main(int argc, char *argv[])
{
	int rank, size;
	MPI_Status status;
	char messSent[50];
	char messRece[50];

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	sprintf(messSent, "Hello from %d", rank);

	if(rank%2)
	{
		MPI_Ssend(messSent, strlen(messSent)+1, MPI_CHAR,((rank+1)%size), 0, MPI_COMM_WORLD);
		MPI_Recv(messRece, 50, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	else
	{
		MPI_Recv(messRece, 50, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Ssend(messSent, strlen(messSent)+1, MPI_CHAR,((rank+1)%size), 0, MPI_COMM_WORLD);
	}

	printf("%d received: %s\n",rank, messRece);

	MPI_Finalize();

	return 0;
}