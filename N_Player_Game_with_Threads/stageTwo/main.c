#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "qew.h"

#define BUFFER_SIZE 1024

pthread_mutex_t lock;

int gameSignal = 0;     // 1 = start game, 0 = end game
int* playerScores;
int Starget;
bool winnerFlag = false;
int winner = -1;
struct Queue* queue;

char** playersArray;
char** slotsArray;
int NSlots;

// ----- helper functions -----

/*
    void sleepy(int num)
    this function executes sleep for 1 second + num milliseconds
    int num is a parameter representing the number pulled from the queue
*/
void sleepy(int num) {
    // src code: https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
    struct timespec tim, tim2;
    tim.tv_sec = 1;
    tim.tv_nsec = num * 1000000;

    if(nanosleep(&tim , &tim2) < 0 ) printf("Nano sleep system call failed \n");
}

/*
    bool playerInSlots(int id)
    takes parameter id of type int which represents the currentId of the thread
    this function checks whether the id is present in the slots and returns true if the id is present in slots and false otherwise

    this function is used to check whether a thread should be running or waiting
*/
bool playerInSlots(int id) {
    for(int i = 0; i < NSlots; i++) {
        if(strcmp(playersArray[id], slotsArray[i]) == 0) {
            return true;
        }
    }
    return false;
}

/*
    int getSlotPosition(int id)
    takes parameter id of type int which represents the currentId of the thread
    returns the slot position of the current thread so that the main process can use the slot position to swap out the winner with the next player in line in the slot array
*/
int getSlotPosition(int id) {
    for(int i = 0; i < NSlots; i++) {
        if(strcmp(playersArray[id], slotsArray[i]) == 0)
            return i;
    }
    printf("ERROR in getSlotPosition");
    return -1;
}

void* runner(void* param) {
    int currentId = *((int*) param);
    int roundNumber = 1;    // init = 1; increment every round; roundNumber % 5 == 0 menas subtract
    bool finished = false;

    while(true) {
        while(!playerInSlots(currentId));
        while(gameSignal == 0 || finished);
        if(!isEmpty(queue)) {
            pthread_mutex_lock(&lock);
            int x = dequeue(queue);
            printf("%s delete %d : ",playersArray[currentId], x);
            printQueueAll(queue);
            pthread_mutex_unlock(&lock);
            
            sleepy(x);
            if(roundNumber % 5 != 0) {
                // pthread_mutex_lock(&lock);
                playerScores[currentId] += x;
                printf("%s score %d\n", playersArray[currentId], playerScores[currentId]);
                if(playerScores[currentId] >= Starget) {
                    pthread_mutex_lock(&lock);
                    printf("%s reaches the target score: %d\n", playersArray[currentId],playerScores[currentId]);
                    finished = true;
                    if(!winnerFlag) {
                        winner = currentId;
                        winnerFlag = true;
                    }
                    pthread_mutex_unlock(&lock);
                }

                // pthread_mutex_unlock(&lock);
            } else {
                // pthread_mutex_lock(&lock);
                playerScores[currentId] -= x;
                printf("%s score %d\n", playersArray[currentId], playerScores[currentId]);

                if(!isEmpty(queue)) {
                    enqueue(queue, x);
                    printf("%s insert %d : ", playersArray[currentId], x);
                    printQueueAll(queue);
                }
                // pthread_mutex_unlock(&lock);
            }
            roundNumber++;
        }
    }

}

int main(int argc, char** argv) {
    srand(time(NULL));

    // preprocessing and file IO
    FILE* fp;
    fp = fopen(argv[1], "r");

    if(fp == NULL)      printf("ERROR: file not found");
    else                printf("File Name: %s\n", argv[1]);

    int QSize, PTotal;
    char garbage;
    fscanf(fp, "%d", &NSlots);
    fscanf(fp, "%d", &QSize);
    fscanf(fp, "%d", &Starget);
    fscanf(fp, "%d", &PTotal);
    fscanf(fp, "%c", &garbage);
    printf("%d %d %d %d\n", NSlots, QSize, Starget, PTotal);

    playersArray = (char**)malloc(PTotal * sizeof(char*));
    slotsArray = (char**)malloc(NSlots * sizeof(char*));
    playerScores = (int*)malloc(sizeof(int) * PTotal);

    int playersArrayQueueCounter = 0;

    for(int i = 0; i < PTotal; i++) {
        char playerName[BUFFER_SIZE];
        fscanf(fp, "%s", playerName);
        int length = strlen(playerName);
        playersArray[i] = malloc(length * sizeof(char));
        strcpy(playersArray[i], playerName);
        
        if(i < NSlots) {
            slotsArray[i] = malloc(length * sizeof(char));
            strcpy(slotsArray[i], playerName);
            printf("%s\n", slotsArray[i]);
            playersArrayQueueCounter++;
        }
    }
    printf("arrayQcounter %d\n", playersArrayQueueCounter);

    // Create all the threads
    pthread_t tid[PTotal];
    pthread_attr_t attr;

    for(int i = 0; i < PTotal; i++) {
        int* x = (int*)malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), NULL, runner, (void*)x);
        printf("Parent: Thread created with ID: %d\n", tid[i]);
    }   printf("\n");

    // random number generation 1 - 50
    // insert to queue until half full
    queue = createQueue(QSize);
    for(int i = 0; i < QSize / 2; i++) {
        int randNum = rand() % 50 + 1;
        enqueue(queue, randNum);
        printf("Dealer insert %d : ", randNum);
        printQueueAll(queue);
    }

    for(int i = 0; i < PTotal; i++) {
        playerScores[i] = 0;
    }

    // START GAME
    printf("Players for the first game: ");
    for(int i = 0; i < NSlots; i++) {
        printf("%s ", slotsArray[i]);
    }   printf("\n");

    pthread_mutex_init(&lock, NULL);

    gameSignal = 1;

    while(playersArrayQueueCounter <= PTotal && gameSignal == 1) {

        int randNum = rand() % 50 + 1;
        if(!isFull(queue)) {
            enqueue(queue, randNum);
            printf("Dealer insert %d : ", randNum);
            printQueueAll(queue);
        }
        else {
            // sleep for 1 sec + randNum milliseconds
            sleepy(randNum);

            if(!isFull(queue)) {
                enqueue(queue, randNum);
                printf("Dealer insert %d : ", randNum);
                printQueueAll(queue);
            }

        }

        if(winner != -1) {
            int winnerSlotPosition = getSlotPosition(winner);

            if(playersArrayQueueCounter + 1 <= PTotal) {
                for(int i = 0; i < NSlots; i++) {
                    playerScores[i] = playerScores[i] / 2;
                }   playerScores[winnerSlotPosition] = 0;

                printf("%s replacing %s at slot %d\n", playersArray[playersArrayQueueCounter], slotsArray[winnerSlotPosition], winnerSlotPosition);

                strcpy(slotsArray[winnerSlotPosition], playersArray[playersArrayQueueCounter]);

                winnerFlag = false;
                winner = -1;
            } else {
                gameSignal = 0;
            }

            playersArrayQueueCounter++;
            winner = -1;
        }

    }

    return 0;
}