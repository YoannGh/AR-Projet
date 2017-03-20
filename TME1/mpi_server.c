#include "mpi_server.h"

static server the_server;


void start_server(void (*callback)(int tag, int source)){
	int flag, rank;
	MPI_Status status;
	pthread_mutex_lock(the_server.getMutex());
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	pthread_mutex_unlock(the_server.getMutex());
	while(1)
	{
		pthread_mutex_lock(the_server.getMutex());
		MPI_Iprobe(rank, 0, MPI_COMM_WORLD, &flag, status);
		pthread_mutex_unlock(the_server.getMutex());
		if(*flag)
		{
			callback(0, rank);
		}
	}

}
void destroy_server(){
	pthread_mutex_destroy(the_server.getMutex());
}

pthread_mutex_t* getMutex(){
	return &the_server.mutex;
}

int main(int argc, char*argv[])
{
	the_server.start_server();
	the_server.destroy_server();
	return 0;
}