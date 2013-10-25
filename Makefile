#-------------------------------------------------------------------------
# Makefile
# Generalized edit distance command line tool
#-------------------------------------------------------------------------

CC=gcc
CFLAGS=-Wall

##########################################################################
PROG = genEditDist
MPROG = GenEditDist.c
OBJS = Trie.o ARTrie.o FileToTrie.o List.o FindEditDistanceMod.o
##########################################################################

all: $(PROG)

$(PROG) : $(OBJS) 
	$(CC) -o $(PROG) $(MPROG) $(CFLAGS) $(OBJS)

%.o : %.c	
	$(CC) -o $@ -c $(CFLAGS) $< 

clean:
	rm -f *.o  core 
