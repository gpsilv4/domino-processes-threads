//
// Created by arpinto on 07-10-2016.
//

#include "domino.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

//inicialização dos trincos, produtor e consumidor
pthread_mutex_t mutexprod = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexcons = PTHREAD_MUTEX_INITIALIZER;

//inicialização dos semáforos, produtor e consumidor
sem_t semProd;
sem_t semCons;

//função produtor
void * produtor(void * i) {
    int k = (intptr_t)i;
    recursive_backtrack((aux_hand+k),(seq+k), 1);
    pthread_exit(NULL);
}

//função consumidor
void *consumidor() {
    while(1) {
        sem_wait(&semCons);
        pthread_mutex_lock(&mutexcons);

        sprintf(buffer, "%s\n", (bufferarray+pos_cons)->sequence);
        pos_cons = (pos_cons+1) % M;

        pthread_mutex_unlock(&mutexcons);
        sem_post(&semProd);
        write (destino, &buffer, strlen(buffer));

    }
}



int main(int argc, const char * argv[])
{
    //inicio tempo
    gettimeofday(&tmp1, NULL);
    srand((unsigned)time(NULL));

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char fname[64];
    strftime(fname, sizeof(fname), "%a %b %d %Hh %Y", tm);

    char path_save_file[150] = "./data/"; //Caminho para a pasta de dados.
    char *path_file_hand = "./data/mao20P.txt";

    strcat(path_save_file,fname);
    strcat(path_save_file,".txt");
    strcat(path_save_file,"\0");

    //Baralho de peças
    DECK * deck = NULL;
    deck = (DECK *) malloc(sizeof(DECK));
    deck->pfirst = NULL;
    deck->num_blocks_available = NUM_BLOCKS;

    //Inicialização do baralho
    deck->pfirst = create_deck(deck->pfirst);

    //Mão do jogador
    HAND * hand = NULL;
    hand = (HAND *) malloc(sizeof(HAND));
    hand->pfirst=NULL;
    hand->hand_size=0;

    //hand = create_hand(hand,deck,10);
    hand = read_hand_from_file(hand, path_file_hand);
    aux_hand = (HAND *) malloc(sizeof(HAND)*hand->hand_size);

    //Lista de sequência de peças
    seq = (SEQUENCE *) malloc(sizeof(SEQUENCE)*hand->hand_size);

    //Concatena e envia path/txt para save_sequences_file()
    char fnameaux[50]="";
    strcat(fnameaux,"./data/");
    strcat(fnameaux,fname);
    strcat(fnameaux,".txt");

    destino = open(path_save_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (destino == -1)
    {
        perror ("Opening Destination File");
        exit(1);
    }


    int i=0;
    for(i=0; i<hand->hand_size; i++)
    {
        (aux_hand+i)->pfirst=NULL;
        (aux_hand+i)->hand_size=0;
        //lê do ficheiro e aloca espaço para os blocos
        *(aux_hand+i) = *read_hand_from_file((aux_hand+i), path_file_hand);

        (seq+i)->pfirst = NULL;
        (seq+i)->size_of_sequence = 0;
        paux_hand = hand->pfirst;

        //Apontador para primeiro bloco alocado em aux_hand
        pauxAuxhand = (aux_hand+i)->pfirst;

        //Copia as peças da mão original, ciclo aux_hand+i
        while(paux_hand!=NULL)
        {
            pauxAuxhand->left_side=paux_hand->left_side;
            pauxAuxhand->right_side=paux_hand->right_side;
            pauxAuxhand=pauxAuxhand->pnext;
            paux_hand=paux_hand->pnext;
        }

        //rodar a mao
        //Troca de peca
        BLOCK * paux = NULL;
        BLOCK * paux2 = NULL;
        paux = hand->pfirst;
        paux2 = hand->pfirst;
        while(paux->pnext != NULL)
            paux = paux->pnext;

        //primeira disponivel
        hand->pfirst->available = 1;
        //Rodar
        hand->pfirst=paux;
        hand->pfirst->pnext=paux2;
        paux->pprev->pnext=NULL;


        //Enviar para o seq
        BLOCK *pnew = (BLOCK* ) malloc(sizeof(BLOCK));
        pnew->left_side = (aux_hand+i)->pfirst->left_side;
        pnew->right_side = (aux_hand+i)->pfirst->right_side;
        pnew->available = 0;
        pnew->pnext = NULL;
        pnew->pprev = NULL;
        (aux_hand+i)->pfirst->available = 0;
        (seq+i)->pfirst=pnew;
        (seq+i)->pfirst->pnext = NULL;
        (seq+i)->pfirst->pprev = pnew;
        (seq+i)->size_of_sequence = 1;
        paux_hand=hand->pfirst;
    }


    //buffer das seq
    bufferarray = (SEQSTRING*)malloc(sizeof(SEQSTRING)*M);
    bufferarray->size_of_sequence = 0;
    bufferarray->pnext = NULL;

    // Produtor Consumidor
    N = hand->hand_size;
    sem_init(&semProd,0,N);
    sem_init(&semCons,0,0);

    pthread_t p[N];
    pthread_t c[M];

    // Criar thread por consumidor
    for(i=0; i<M; i++)
        pthread_create(&c[i], NULL, &consumidor, (void*)(intptr_t)i);

    // Criar thread por produtor
    for (i=0; i<N; i++)
        pthread_create(&p[i], NULL, &produtor, (void *)(intptr_t)i);

    // Recolha dos returns dos threads produtores
    for(i=0; i<hand->hand_size; i++)
        pthread_join(p[i], NULL);

    gettimeofday(&tmp2, NULL);
    printf ("Tempo total = %f seconds\n",
    (double) (tmp2.tv_usec - tmp1.tv_usec) / 1000000 +(double) (tmp2.tv_sec - tmp1.tv_sec));

    return 0;
}




/**
 * create_deck - Função responsável por criar o baralho de peças do domino.
 * @param pblock - Apontador para uma lista de peças inicialmente vazia.
 * @return lista de peças do domino.
 */
BLOCK *create_deck(BLOCK *pblock)
{
    short i = 0, valMax = 6, valMin = 0, blocks = NUM_BLOCKS;
    for (valMax = 6; valMax >= 0; valMax--)
    {
        valMin = 0;
        for (i = 0; i <= valMax; i++)
        {
            pblock = insert(pblock, valMax, valMin, 1);
            blocks--;
            valMin++;
        }
    }
    return pblock;
}

/**
 * create_hand - Função responsável por distribuir peças do baralho para a mão do jogador.
 * @param phand - Apontador para a mão do jogador.
 * @param pdeck - Apontador para o baralho de peças.
 * @param num_blocks_hand - Número de peças a serem alocada à mão do jogador.
 * @return Apontador para a primeira peça da mão do jogador.
 */
HAND *create_hand(HAND *phand, DECK *pdeck, short num_blocks_hand)
{
    int index = 0;
    short i = 0, upLim = pdeck->num_blocks_available;
    HAND *pprim = phand;
    BLOCK *paux = NULL;
    for (i = 0; i < num_blocks_hand; i++)
    {
        index = uniform_index_block(0, upLim);
        paux = get_block(pdeck, index);
        phand->pfirst = insert(phand->pfirst, paux->left_side, paux->right_side, paux->available);
        upLim--;
        pdeck->num_blocks_available = upLim;
    }
    pprim->hand_size = num_blocks_hand;
    return pprim;
}

/**
 * uniform_index_block - Gera um valor aleatório entre um intervalo de valores.
 * @param val_min - valor mais baixo do intervalo.
 * @param val_max - valor mais alto do intervalo.
 * @return valor gerado.
 */
int uniform_index_block(short val_min, short val_max)
{
    return val_min + rand() % (val_max - val_min + 1);
}

/**
 * get_block - devolve uma peça.
 * @param pdeck - Apontador para o baralho de peças.
 * @param index - Posição da peça a returnar.
 * @return Uma peça do baralho.
 */
BLOCK *get_block(DECK *pdeck, int index)
{

    BLOCK *pblock = pdeck->pfirst;
    BLOCK *paux = NULL, *pant = NULL;
    int i = 0;

    if (pblock == NULL)
    {
        printf("Empty\n");
        return NULL;
    }
    paux = pblock;

    while (paux != NULL && i != index)
    {
        pant = paux;
        paux = paux->pnext;
        i++;
    }

    if (pant == NULL)
    {
        pdeck->pfirst = paux->pnext;
        paux->pnext = NULL;
        return paux;
    }

    if (paux != NULL)
    {
        pant->pnext = paux->pnext;
        paux->pnext = NULL;
        return paux;
    }
    return paux;
}

/**
 * read_hand_from_file - Cria a mão de peças do jogador atrevés da leitura de um ficheiro.
 * @param phand - Apontador para a mão do jogador.
 * @param pathFile - Caminho para o ficheiro.
 * @return Apontador para a primeira peça da mão do jogador.
 */
HAND *read_hand_from_file(HAND *phand, char *pathFile)
{
    short i = 0, num_blocks = 0, leftSide = 0, rightSide = 0;;
    HAND *pprim = phand;
    FILE *fp = NULL;
    fp = fopen(pathFile, "r");
    if (fp != NULL)
    {
        fscanf(fp, "%hi", &num_blocks);
        for (i = 0; i < num_blocks; i++)
        {
            fscanf(fp, "%*c %*c %hi %*s %hi %*c", &leftSide, &rightSide);
            phand->pfirst = insert(phand->pfirst, leftSide, rightSide, 1);
        }
    }
    fclose(fp);
    pprim->hand_size = num_blocks;
    return pprim;
}

/**
 * insert - Aloca memória para uma peça e inseri-a num lista de peças.
 * @param pblock - Apontador para uma lista de peças.
 * @param left_side - Valor esquerdo da peça.
 * @param right_side - Valor direito da peça.
 * @param avaiable - Valor que indica se a peça esta dispinivel. (0/1)
 * @return Apontador para uma lista de peças.
 */
BLOCK *insert(BLOCK *pblock, short left_side, short right_side, short avaiable)
{

    BLOCK *pnew = NULL;
    BLOCK *paux = pblock;

    pnew = (BLOCK *) malloc(sizeof(BLOCK));
    pnew->left_side = left_side;
    pnew->right_side = right_side;
    pnew->available = avaiable;
    pnew->pnext = pnew;
    pnew->pprev = pnew;

    if (paux == NULL)
    {
        return pnew;
    }

    paux->pprev->pnext = pnew;
    pnew->pprev = paux->pprev;
    paux->pprev = pnew;
    pnew->pnext = NULL;

    return paux;
}

/**
 * recursive_backtrack - Função recursiva responsável pela pesquisa exaustiva de sequências de peças.
 * @param phand - Apontador para a mão do jogador.
 * @param pseq - Apontador para a primeira peça da lista de sequência.
 * @param pall_sequences - Apontador para uma lista de todas as sequências geradas.
 * @param inserted - Número de peças inseridas na sequencia.
 * @param pos - id thread.
 * @return 0 - Quando termina a procura.
 */
int recursive_backtrack(HAND *phand, SEQUENCE *pseq, short inserted)
{

    int i = 0;
    BLOCK *block = NULL;
    block = phand->pfirst;

    for (i = 0; i < phand->hand_size; i++)
    {
        if (block->available == 1)
        {
            if (is_current_assignment_consistent(pseq, block, inserted) == ASSIGNMENT_CONSISTENT)
            {

                BLOCK *pnew = (BLOCK *) malloc(sizeof(BLOCK));
                pnew->left_side = block->left_side;
                pnew->right_side = block->right_side;
                pnew->available = 0;

                pnew->pnext = NULL;
                pnew->pprev = NULL;

                if(inserted == 0)
                {
                    pseq->pfirst = pnew;
                    pseq->pfirst->pnext = NULL;
                    pseq->pfirst->pprev = pnew;
                }
                else
                {
                    pnew->pprev = pseq->pfirst->pprev;
                    pnew->pprev->pnext = pnew;
                    pnew->pnext = NULL;
                    pseq->pfirst->pprev = pnew;
                }

                inserted++;
                pseq->size_of_sequence++;
                block->available = 0;

                if(pseq->size_of_sequence == phand->hand_size)
                {
                    save_sequence(pseq);
                }

                recursive_backtrack(phand, pseq, inserted);

                block->available = 1;
                BLOCK *paux = pseq->pfirst->pprev;
                pseq->pfirst->pprev = pseq->pfirst->pprev->pprev;
                pseq->pfirst->pprev->pnext = NULL;

                free(paux);
                pseq->size_of_sequence--;
                inserted--;
            }
        }
        block = block->pnext;
    }
    return 0;
}

/**
 * is_current_assignment_consistent - Função responsável por verificar se a próxima peça a inserir na sequência é
 * consistente ao não. Se necessário a função is_current_assignment_consistent invoca funções auxiliares de forma
 * a inverter peças tornando a sequência consistente.
 * @param pseq - Apontador para a primeira peça da lista de sequência.
 * @param pnew_block - Apontador para a nova peça a inserir na sequencia.
 * @param inserted - Número de peças inseridas na sequencia.
 * @return (1/0) se a sequênia é ou não consistente com a nova peça.
 */
int is_current_assignment_consistent(SEQUENCE *pseq, BLOCK *pnew_block, short inserted)
{
    if (inserted == 0)
    {
        return ASSIGNMENT_CONSISTENT;
    }
    else if (pnew_block->left_side == pseq->pfirst->pprev->right_side)
    {
        return ASSIGNMENT_CONSISTENT;
    }
    else if (pnew_block->right_side == pseq->pfirst->pprev->right_side)
    {
        invert_block(pnew_block);
        return ASSIGNMENT_CONSISTENT;
    }
    else if (inserted == 1 && pnew_block->left_side == pseq->pfirst->left_side )
    {
        invert_block_sequence(pseq);
        return ASSIGNMENT_CONSISTENT;
    }
    else if (inserted == 1 && pnew_block->right_side == pseq->pfirst->left_side )
    {
        invert_block_sequence(pseq);
        invert_block(pnew_block);
        return ASSIGNMENT_CONSISTENT;
    }
    else
    {
        return ASSIGNMENT_FAILURE;
    }
}

/**
 * invert_block - Inverte uma peça.
 * @param pblock - Peça a inverter.
 */
void invert_block(BLOCK *pblock)
{
    short aux = 0;
    aux = pblock->left_side;
    pblock->left_side = pblock->right_side;
    pblock->right_side = aux;
}

/**
 * invert_block_sequence - Inverte a primeira peça da sequencia.
 * @param pseq - Apontador para a primeira peça da lista de sequência.
 */
void invert_block_sequence(SEQUENCE *pseq)
{
    short aux = 0;
    aux = pseq->pfirst->left_side;
    pseq->pfirst->left_side = pseq->pfirst->right_side;
    pseq->pfirst->right_side = aux;
}

/**
 * save_sequence - Função responsável por guardar uma sequencia gerada na estrutura de dados que armazena todas
 * as sequencias (ALLSEQUENCES).
 * Antes de a guardar a função converte a lista de peças da sequência numa string e armazena-a
 * como string.
 * @param pall_sequences - Apontador para a lista de todas as sequências guardadas.
 * @param pSeq - Apontador para a sequência a guardar.
 */
void save_sequence(SEQUENCE *pSeq)
{
    int i = 0, size_sequence = 0;

    SEQSTRING *newSequence = (SEQSTRING*)malloc(sizeof(SEQSTRING));
    newSequence->size_of_sequence = pSeq->size_of_sequence;
    newSequence->pnext = NULL;

    BLOCK *paux = pSeq->pfirst;

    char *sequence = (char *) malloc(sizeof(char)* (newSequence->size_of_sequence * 2) + 1);
    *sequence = '\0';

    for (i = 0; i < newSequence->size_of_sequence && paux != NULL; i++)
    {
        size_sequence = strlen(sequence);
        *(sequence + size_sequence)     = '0' + paux->left_side;
        *(sequence + (size_sequence+1)) = '0' + paux->right_side;
        *(sequence + (size_sequence+2)) = '\0';
        paux = paux->pnext;
    }

    newSequence->sequence = sequence;

    sem_wait(&semProd);
    pthread_mutex_lock(&mutexprod);

    (bufferarray+pos_prod)->sequence = sequence;
    //posição seguinte
    pos_prod = (pos_prod+1) % M;

    pthread_mutex_unlock(&mutexprod);
    sem_post(&semCons);

}



void freeList(SEQSTRING **pfirst)
{
    SEQSTRING *tmp;
    while (*pfirst != NULL)
    {
        tmp = *pfirst;
        *pfirst = (*pfirst)->pnext;
        free(tmp);
    }
}
