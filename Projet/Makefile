CC       = mpicc
# compiling flags here
CFLAGS   = -Wall -Wextra -std=c99

LINKER   = mpicc
# linking flags here
LFLAGS   = 

# change these to proper directories where each file should be
SRCDIR   = .
OBJDIR   = .
BINDIR   = .

EXO1_PROGRAM_NAME = routage_anneau
EXO2_PROGRAM_NAME = chord_fingertable

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

all: $(BINDIR)/$(EXO1_PROGRAM_NAME) $(BINDIR)/$(EXO2_PROGRAM_NAME) 

$(BINDIR)/$(EXO1_PROGRAM_NAME): $(OBJDIR)/common.o $(OBJDIR)/$(EXO1_PROGRAM_NAME).o
	$(LINKER) $(LFLAGS) -o $@ $^

$(BINDIR)/$(EXO2_PROGRAM_NAME): $(OBJDIR)/common.o $(OBJDIR)/$(EXO2_PROGRAM_NAME).o
	$(LINKER) $(LFLAGS) -o $@ $^

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

run1 :
	@make && mpirun -np $(ARGS) $(EXO1_PROGRAM_NAME)
run2 :
	@make && mpirun -np $(ARGS) $(EXO2_PROGRAM_NAME)

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(BINDIR)/$(EXO1_PROGRAM_NAME)
	@$(rm) $(BINDIR)/$(EXO2_PROGRAM_NAME)