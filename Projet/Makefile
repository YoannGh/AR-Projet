CC = mpicc 
CFLAGS = -Wall -Wextra -std=c99
 
SRC = $(wildcard *.c)
OBJS = $(SRC:.c=.o)
PROGRAM_NAME = routage_anneau
 
all : $(PROGRAM_NAME) 
 
prog : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

 #Usage make run ARGS=5
run :
	@make && mpirun -np $(ARGS) $(PROGRAM_NAME)

clean : 
	@rm $(PROGRAM_NAME)
	@rm *.o