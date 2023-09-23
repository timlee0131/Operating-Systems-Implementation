#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int** fileArray;
int* roundChecker; // index value = thread already run this round? true/false
int message = 0; // 0 = run f1 1 = kill

int f1(int x, int a, int c, int m) {
    long int x1 = x * a + c;
    return (int)(x1 % m);
}

void* runner(void* param) {
    int currentId = *((int*) param);
    while(message != 1) {
        if(roundChecker[currentId] == 0) {
            int x_new = f1(fileArray[currentId][3], fileArray[currentId][0], fileArray[currentId][1], fileArray[currentId][2]);
            printf("Thread %d call f1() returns %d\n", currentId, x_new);
            fileArray[currentId][3] = x_new;
            roundChecker[currentId] = 1;
        }
    }
}

// helper function: min-max (modified version of minMax from https://www.geeksforgeeks.org/maximum-and-minimum-in-an-array/)
struct pair {
    int minDex;
    int maxDex;
};

struct pair minMax(int arr[], int n) {
    struct pair minmax;
    int currMax, currMin;
    int i;
    
    /*If there is only one element then return it as min and max both*/
    if (n == 1)
    {
        minmax.maxDex = 0;
        minmax.minDex = 0;    
        return minmax;
    }   
    
    /* If there are more than one elements, then initialize min
        and max*/
    if (arr[0] > arr[1]) 
    {
        currMax = arr[0];
        currMin = arr[1];

        minmax.maxDex = 0;
        minmax.minDex = 1;
    } 
    else
    {
        currMax = arr[1];
        currMin = arr[0];

        minmax.maxDex = 1;
        minmax.minDex = 0;
    }   
    
    for (i = 2; i<n; i++)
    {
        if (arr[i] >= currMax) {
            minmax.maxDex = i;
            currMax = arr[i];
        }
    
        else if (arr[i] <  currMin) {
            minmax.minDex = i;
            currMin = arr[i];
        }
    }
    
    return minmax;
}

int* overallWinner(int* arr, int n) {
    int winners[n];
    for(int i = 0; i < n; i++) {
        winners[i] = -1;
    }

    int currMax = 0;
    for(int i = 0; i < n; i++) {
        if(arr[i] >= currMax)
            currMax = arr[i];
    }

    for(int i = 0; i < n; i++) {
        if(arr[i] >= currMax) {
            winners[i] = i;
        }
    }

    return winners;
}

int main(int argc, char** argv) {

    // ---------- FILE IO ----------

    FILE *fp;
    fp = fopen(argv[1], "r");

    if(fp == NULL) {
        printf("ERROR: file not found");
    } else {
        printf("File Name: %s\n\n", argv[1]);
    }

    int numberThreads, numberRounds;
    char temp;
    fscanf(fp, "%d", &numberThreads);
    fscanf(fp, "%d", &numberRounds);
    fscanf(fp, "%c", &temp);

    fileArray = (int**)malloc(sizeof(int*) * numberThreads);
    for(int i = 0; i < numberThreads; i++) {
        fileArray[i] = (int*)malloc(sizeof(int) * 4);
    }

    int threadCounter = 0;
    int positionCounter = 0;
    char newLineFlag;
    // read format: a c m x
    while(threadCounter < numberThreads) {
        fscanf(fp, "%d%c", &fileArray[threadCounter][positionCounter], &newLineFlag);
        positionCounter++;
        if(newLineFlag == '\n') {
            threadCounter++;
            positionCounter = 0;
        }
    }
    fclose(fp);

    roundChecker = (int*)malloc(sizeof(int) * numberThreads);
    for(int i = 0; i < numberThreads; i++) {
        roundChecker[i] = 1;
    }

    // ---------- MULTITHREADING ----------

    int scoreKeeper[numberThreads]; // keep the scores
    for(int i = 0; i < numberThreads; i++) {
        scoreKeeper[i] = 0;
    }

    pthread_t tid[numberThreads];
    pthread_attr_t attr;

    for(int i = 0; i < numberThreads; i++) {
        int* x = (int*)malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), NULL, runner, (void*)x);
        printf("Parent: Thread created wih ID: %d\n", tid[i]);
    }
    printf("\n");

    for(int i = 1; i <= numberRounds; i++) {
        printf("Main process start round %d\n", i);
        message = 0;
        for(int i = 0; i < numberThreads; i++) {
            roundChecker[i] = 0;
        }
        sleep(1);
        int* roundValues = (int*)malloc(sizeof(int) * numberThreads);
        for(int i = 0; i < numberThreads; i++) {
            roundValues[i] = fileArray[i][3];
        }
        struct pair minmax = minMax(roundValues, numberThreads);
        scoreKeeper[minmax.maxDex] += 1;
        scoreKeeper[minmax.minDex] += 1;

        printf("Round %d finished. Winners are %d, %d\n", i, minmax.minDex, minmax.maxDex);
        printf("\n");

        free(roundValues);
    }
    message = 1;

    for(int i = 0; i < numberThreads; i++) {
        printf("Score for thread %d: %d\n", i, scoreKeeper[i]);
    }

    int* winners = overallWinner(scoreKeeper, numberThreads);

    printf("\n");
    printf("Overall winner: ");
    for(int i = 0; i < numberThreads; i++) {
        if(winners[i] != -1)
            printf("%d ", i);
    }   printf("\n");

    pthread_exit(NULL);

    return 0;
}