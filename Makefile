SHELL = /bin/sh


## Working directory #########################################################
OBJDIR = ./obj
INCLUDEDIR = ./include
SRCDIR = ./src


## Specifies a path containing the source files (.c and .h) #################
VPATH = $(SRCDIR):$(INCLUDEDIR)


## Compileur #################################################################
CC = gcc -std=c11
FLAGS = -Wall -g
LDFLAGS =
EXEC = exo2 exo2bis1 exo2bis2 exo3 exo4
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


## Command ###################################################################
MKDIR = @mkdir -p
RM = @rm -rf


## First target: the default goal is 'all' ###################################
all: $(OBJ) $(EXEC)


## Rules to create the targets files #########################################
.PHONY: all
#$(EXEC): $(OBJ)
#	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

exo2: $(OBJDIR)/exo2.o $(OBJDIR)/semaphore.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

exo2bis1: $(OBJDIR)/exo2bis1.o $(OBJDIR)/semaphore.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

exo2bis2: $(OBJDIR)/exo2bis2.o $(OBJDIR)/semaphore.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

exo3: $(OBJDIR)/exo3.o $(OBJDIR)/semaphore.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

exo4: $(OBJDIR)/exo4.o $(OBJDIR)/semaphore.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)



## Rule to create objects from source ########################################
$(OBJDIR)/%.o: %.c
	$(MKDIR) $(OBJDIR)
	$(CC) $(FLAGS) -o $@ -c $<


## Clean command #############################################################
.PHONY: clean distclean
clean:
	$(RM) $(OBJDIR)
	$(RM) ./*~
distclean: clean
	$(RM) $(EXEC)
