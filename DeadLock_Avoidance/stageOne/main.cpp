#include <pthread.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <queue>

#include <chrono>

#include "graph.h"

pthread_mutex_t graphLock, printLock, randLock;
bool gameSignal = false;
bool endOfGame = false;

int A, N, K; // A = total # of people N = # of threads concurrently running, K = # of resources

graph* avoidanceGraph;

struct processResource {
    std::string playerName;
    std::vector<int> resources;
};

// contains all resources (requesting or releasing) for each player
std::vector<processResource> playersResources;

// contains info on which thread has which process
int* threadToProcess; // if tToP[i] == -1, finished and waiting; else, tToP[i] = processName

void sleepy(int num) {
    // src code: https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
    struct timespec tim, tim2;
    tim.tv_sec = num;
    tim.tv_nsec = 0;

    if(nanosleep(&tim , &tim2) < 0 ) printf("Nano sleep system call failed \n");

    // printf("Nano sleep successfull \n");
}

bool request(int playerID, int resource) {
    pthread_mutex_lock(&printLock);
    std::cout << playersResources[playerID].playerName << " requests resource " << resource << std::endl;
    pthread_mutex_unlock(&printLock);

    // print graph
    pthread_mutex_lock(&printLock);
    avoidanceGraph->printGraph(playerID + 1 + K, resource);
    pthread_mutex_unlock(&printLock);

    bool isEdge = avoidanceGraph->validEdge(resource);

    avoidanceGraph->removeEdge(playerID + 1 + K, resource);
    avoidanceGraph->addEdge(resource, playerID + 1 + K);
    if(avoidanceGraph->isCyclic() || isEdge) {
        avoidanceGraph->removeEdge(resource, playerID + 1 + K);
        avoidanceGraph->addEdge(playerID + 1 + K, resource);
        pthread_mutex_lock(&printLock);
        std::cout << playersResources[playerID].playerName << " requests resource " << resource << ": denied" << std::endl;

        // // DEBUG
        // std::cout << std::endl;
        // avoidanceGraph->printGraph();
        // std::cout << std::endl;
        pthread_mutex_unlock(&printLock);

        return false;
    } else {
        pthread_mutex_lock(&printLock);
        std::cout << playersResources[playerID].playerName << " requests resource " << resource << ": accepted" << std::endl;

        // print graph
        avoidanceGraph->printGraph();

        // // DEBUG
        // std::cout << std::endl;
        // avoidanceGraph->printGraph();
        // std::cout << std::endl;
        pthread_mutex_unlock(&printLock);
        return true;
    }
}

void release(int playerID, int resource, int position) {
    int resourceActual = resource * -1;
    avoidanceGraph->removeEdge(resourceActual, playerID + 1 + K);

    pthread_mutex_lock(&printLock);
    std::cout << playersResources[playerID].playerName << " releases resource " << resource << std::endl;
    
    // print graph
    avoidanceGraph->printGraph();
    pthread_mutex_unlock(&printLock);

    for(int i = position + 1; i < playersResources[playerID].resources.size(); i++) {
        if(resourceActual == playersResources[playerID].resources[i]) {
            avoidanceGraph->addEdge(playerID + 1 + K, resourceActual);
            return;
        }
    }
    return;
}

void* runner(void* param) {
    // PRE PROCESSING
    int threadId = *((int*) param);
    // int currentId = threadToProcess[*((int*) param)];

    while(!endOfGame) {
        while(!endOfGame && !gameSignal);
        while(!endOfGame && threadToProcess[threadId] == -1);

        if(endOfGame) break;

        int currentId = threadToProcess[threadId];
        // game execution
        for(int i = 0; i < playersResources[currentId].resources.size(); i++) {
            int resource = playersResources[currentId].resources[i];
            
            // next item requesting resource
            if(resource > 0) {
                pthread_mutex_lock(&randLock);
                int q = rand() % 100;
                pthread_mutex_unlock(&randLock);

                pthread_mutex_lock(&graphLock);
                bool requestReturn = request(currentId, resource);
                pthread_mutex_unlock(&graphLock);

                if(requestReturn) {
                    sleepy(1 + q/100);
                    // sleepy(q/100);
                } else {
                    i--;
                    sleepy(q/100);
                    // sleepy(1 + q/100);
                }
            } else {
                pthread_mutex_lock(&graphLock);
                release(currentId, resource, i);
                pthread_mutex_unlock(&graphLock);
            }
        }
        pthread_mutex_lock(&randLock);
        int q = rand() % 100;
        pthread_mutex_unlock(&randLock);

        sleepy(1 + q/100);

        // release all resources still in holding
        for(int i = 0; i < playersResources[currentId].resources.size(); i++) {
            int resrc = playersResources[currentId].resources[i];
            if(resrc > 0) {
                pthread_mutex_lock(&graphLock);
                avoidanceGraph->removeEdge(resrc, currentId + 1 + K);
                pthread_mutex_unlock(&graphLock);
            }
        }
        pthread_mutex_lock(&printLock);
        avoidanceGraph->printGraph();
        std::cout << playersResources[currentId].playerName << " finishes" << std::endl;
        pthread_mutex_unlock(&printLock);

        threadToProcess[threadId] = -1;
    }
}

int main(int argc, char** argv) {
    pthread_mutex_init(&graphLock, NULL);
    pthread_mutex_init(&printLock, NULL);
    pthread_mutex_init(&randLock, NULL);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    srand(time(0));

    // PRE PROCESSING

    std::fstream fio(argv[1]);

    int nVertices;
    fio >> A >> N >> K;

    if(A < N) {
        A = N;
    }

    threadToProcess = new int [N];  // 
    std::queue<int> processQueue;

    nVertices = A + K;
    
    for(int i = 0; i < A; i++) {
        int numbersLength;
        playersResources.push_back(processResource());
        fio >> playersResources[i].playerName >> numbersLength;

        for(int j = 0; j < numbersLength; j++) {
            int number;
            fio >> number;
            playersResources[i].resources.push_back(number);
        }

        processQueue.push(i);
    }

    // initialize graph class with the proper # of vertices
    graph cons(nVertices, A, K);
    avoidanceGraph = &cons;

    // // prints playersResources for testing purposes
    // for(int i = 0; i < N; i++) {
    //     std::cout << playersResources[i].playerName << " ";
    //     for(int j = 0; j < playersResources[i].resources.size(); j++) {
    //         std::cout << playersResources[i].resources[j] << " ";
    //     }   std::cout << std::endl;
    // }

    // THREAD CREATION

    pthread_t tid[N];
    pthread_attr_t attr;

    for(int i = 0; i < N; i++) {
        // initialize threadToProcess with original N processes
        threadToProcess[i] = i;
        processQueue.pop();

        int* x = (int*)malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), NULL, runner, (void*)x);
        printf("Parent: Thread created with ID: %d\n", tid[i]);
    }

    // initial establishing of claim edges
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < playersResources[i].resources.size(); j++) {
            int resrc = playersResources[i].resources[j];
            if(resrc > 0) {
                avoidanceGraph -> addEdge(i + 1 + K, resrc);
            }
        }
    }

    // print graph
    avoidanceGraph->printGraph();

    // START GAME
    gameSignal = true;

    while(gameSignal && !endOfGame) {
        bool AllProcessFinished = true;

        for(int i = 0; i < N; i++) {
            if(threadToProcess[i] == -1) {
                if(!processQueue.empty()) {
                    int newPlayer = processQueue.front();
                    // initialize claim edges for the new player
                    pthread_mutex_lock(&graphLock);
                    for(int i = 0; i < playersResources[newPlayer].resources.size(); i++) {
                        int resrc = playersResources[newPlayer].resources[i];
                        if(resrc > 0) {
                            avoidanceGraph -> addEdge(newPlayer + 1 + K, resrc);
                        }
                    }
                    pthread_mutex_unlock(&graphLock);
                    threadToProcess[i] = newPlayer;

                    std::cout << playersResources[newPlayer].playerName << " assigned to a thread" << std::endl;
                    avoidanceGraph->printGraph();

                    processQueue.pop();
                }
            } else {
                AllProcessFinished = false;
            }
        }

        if(processQueue.empty() && AllProcessFinished) {
            endOfGame = true;
            // gameSignal = false;
        }
    }

    for(int i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }
    delete [] threadToProcess;

    // end = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed_seconds = end - start;
    // std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  
    // std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}