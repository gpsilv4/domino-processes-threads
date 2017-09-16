//
// Created by André Pinto on 17/01/17.
//

#ifndef SO_LIB_DOMINO_H
#define SO_LIB_DOMINO_H
#define SIZE 100
#define NUM_BLOCKS_HAND 7
#define NUM_BLOCKS 27
#define ASSIGNMENT_FAILURE 0
#define ASSIGNMENT_COMPLETE 1
#define ASSIGNMENT_CONSISTENT 10
#define MEMORY_MAX_SEQUENCES 100000




typedef struct block {
    short  left_side;
    short  right_side;
    short available;
    struct block *pnext;
    struct block *pprev;
} BLOCK;

typedef struct hand {
    BLOCK *pfirst;
    short hand_size;
} HAND;

typedef struct deck {
    BLOCK *pfirst;
    short num_blocks_available;
} DECK;

typedef struct sequence {
    BLOCK *pfirst;
    short size_of_sequence;
} SEQUENCE;

typedef struct seqString {
    short size_of_sequence;
    char *sequence;
    struct seqString *pnext;
} SEQSTRING;

typedef struct allSequences{
    unsigned long number_of_sequences;
    char *path_file;
    SEQSTRING *pfirst;
} ALLSEQUENCES;

BLOCK *create_deck(BLOCK *deck);

HAND *create_hand(HAND *hand, DECK *deck, short num_blockd_hand);
int uniform_index_block(short val_min, short val_max);
BLOCK *get_block(DECK *pdeck, int index);
HAND  *read_hand_from_file(HAND *hands, char *);

int recursive_backtrack(HAND *hands, SEQUENCE *seq, short inserted);
int is_current_assignment_consistent(SEQUENCE *pSequence, BLOCK *newBlock, short inserted);
void invert_block(BLOCK *pBlock);
void invert_block_sequence(SEQUENCE *pSequence);
void save_sequence(SEQUENCE *pSequence);
void freeList(SEQSTRING **pfirst);
BLOCK *insert(BLOCK *pprim, short leftSide, short rightSide, short avaiable);

//produtor consumidor
void *produtor(void * i);
void *consumidor();

//tempos
struct timeval tmp1, tmp2;
void gettimeofday();

//mão e peças
HAND * aux_hand;
BLOCK * pauxAuxhand;
BLOCK * paux_hand;
SEQUENCE * seq;
SEQSTRING * bufferarray;

int N = 0; //Produtor
int M = 500; //Consumidor
int pos_prod=0;
int pos_cons=0;
int destino;
char buffer[SIZE];

#endif //SO_LIB_DOMINO_H
