# TODO: Fix Makefile :/

CC = mpicc 
CFLAGS = -Wall -Wextra -std=c99
 
#SRC = $(wildcard *.c)
#OBJS = $(SRC:.c=.o)
#EXO1_PROGRAM_NAME = routage_anneau
EXO2_PROGRAM_NAME = chord_fingertables
 
$(info $$OBJS are ${OBJS})

SRCDIR   = .
OBJDIR   = .
BINDIR   = .

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(CC) -o $@ $(OBJECTS)
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
    @$(CC) $(CFLAGS) -c $< -o $@

all : $(EXO1_PROGRAM_NAME) $(EXO2_PROGRAM_NAME) 
 
#prog : $(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $^
#%.o : %.c
#	$(CC) $(CFLAGS) -o $@ -c $<

 #Usage make run ARGS=5
run :
	@make && mpirun -np $(ARGS) $(EXO1_PROGRAM_NAME)

clean : 
	@rm $(EXO1_PROGRAM_NAME)
	@rm $(EXO2_PROGRAM_NAME)
	@rm *.o