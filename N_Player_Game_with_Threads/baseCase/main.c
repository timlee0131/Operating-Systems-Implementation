#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include "qew.h"

pthread_mutex_t lock;

int gameSignal = 0;     // 1 = start game, 0 = end game
int endGame = 0;        // init = 0, increment until endGame == 2 then end game
int* playerScores;
int Starget;
int winners[2];
struct Queue* queue;

// ----- helper functions -----

void sleepy(int num) {
    // src code: https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
    struct timespec tim, tim2;
    tim.tv_sec = 1;
    tim.tv_nsec = num * 1000000;

    if(nanosleep(&tim , &tim2) < 0 ) printf("Nano sleep system call failed \n");

    // printf("Nano sleep successfull \n");
}

void* runner(void* param) {
    int currentId = *((int*) param);
    int roundNumber = 1;    // init = 1; increment every round; roundNumber % 5 == 0 menas subtract
    bool finished = false;

    while(true) {
        while(gameSignal == 0 || finished);
        if(!isEmpty(queue)) {
            pthread_mutex_lock(&lock);
            int x = dequeue(queue);
            printf("Player %d delete %d : ", currentId ,x);
            printQueueAll(queue);
            pthread_mutex_unlock(&lock);
            
            sleepy(x);
            if(roundNumber % 5 != 0) {
                playerScores[currentId] += x;
                printf("Player %d score %d\n", currentId, playerScores[currentId]);
            } else {
                playerScores[currentId] -= x;
                printf("Player %d score %d\n", currentId, playerScores[currentId]);

                if(!isEmpty(queue)) {
                    enqueue(queue, x);
                    printf("Player %d insert %d : ", currentId, x);
                    printQueueAll(queue);
                }
            }
            roundNumber++;
        }
        if(playerScores[currentId] >= Starget) {
            printf("Player %d reaches the target score: %d\n", currentId,playerScores[currentId]);
            pthread_mutex_lock(&lock);
            endGame++;
            if(endGame == 1)    winners[0] = currentId;
            if(endGame == 2) {
                winners[1] = currentId;
                gameSignal == 0;
            }
            // else                printf("error: endGame incrementation\n");
            pthread_mutex_unlock(&lock);
            finished = true;
        }
    }

}

int main(int argc, char** argv) {
    srand(time(NULL));
    int NPlayers = strtol(argv[1], NULL, 10);   // number of players (N)
    int QSize = strtol(argv[2], NULL, 10);      // size of queue (Q)
    Starget = strtol(argv[3], NULL, 10);    // target score (S)
    
    playerScores = (int*)malloc(sizeof(int) * NPlayers);

    // Create all the threads
    pthread_t tid[NPlayers];
    pthread_attr_t attr;

    for(int i = 0; i < NPlayers; i++) {
        playerScores[i] = 0;

        int* x = (int*)malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), NULL, runner, (void*)x);
        printf("Parent: Thread created with ID: %d\n", tid[i]);
    }

    // random number generation 1 - 50
    // insert to queue until half full
    queue = createQueue(QSize);
    for(int i = 0; i < QSize / 2; i++) {
        int randNum = rand() % 50 + 1;
        enqueue(queue, randNum);
        printf("Dealer insert %d : ", randNum);
        printQueueAll(queue);
    }

    // START GAME
    pthread_mutex_init(&lock, NULL);
    gameSignal = 1;

    while(gameSignal == 1) {
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

        // check if the game should end
        if(endGame >= 2) {
            printf("End of game detected\n");
            gameSignal = 0;
        }
    }
  
    printf("Winner Player %d with score %d, Runner up Player %d with score %d\n", winners[0], playerScores[winners[0]], winners[1], playerScores[winners[1]]);

    return 0;
}