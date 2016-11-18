#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

sem_t forkArray[5];
sem_t room;

/*function that doesn't prevent deadlocks. No
room semaphore only forkArray semaphore*/
void *eatSpaghetti(void *position){
    int i = (intptr_t) position;
    int count = 0;
    
    while(count < 5){
        printf("Philosopher %d: is done thinking and now ready to eat.\n", i);
        
        sem_wait(&forkArray[i]);
        printf("Philosopher %d: taking left fork %d\n", i, i);
        sleep(1);
        
        if(i == 4){
            sem_wait(&forkArray[0]);
            printf("Philosopher %d: taking right fork 0\n", i);
        }
        
        else{
            sem_wait(&forkArray[i+1]);
            printf("Philosopher %d: taking right fork %d\n", i, i+1);
        }
        
        printf("Philosopher %d: EATING.", i);
        
        sem_post(&forkArray[i]);
        printf("Philosopher %d: putting down left fork %d\n", i, i);
        
        if(i == 4){
            sem_post(&forkArray[0]);
            printf("Philosopher %d: putting down right fork 0\n", i);
        }
        
        else{
            sem_post(&forkArray[i+1]);
            printf("Philosopher %d: putting down right fork %d\n", i, i+1);
        }
        
        count++;
    }
    return 0;
}

/*function that prevents deadlocks from occuring for
philosophers. Done by using room semaphore and forkArray 
semaphores*/
void * noDeadlockSpaghetti(void *position){
    int i = (intptr_t) position;
    int count = 0;
    
        while(count < 5){
        printf("Philosopher %d: is done thinking and now ready to eat.\n", i);
        
        sem_wait(&room);
        sem_wait(&forkArray[i]);
        printf("Philosopher %d: taking left fork %d\n", i, i);
        
        if(i == 4){
            sem_wait(&forkArray[0]);
            printf("Philosopher %d: taking right fork 0\n", i);
        }
        
        else{
            sem_wait(&forkArray[i+1]);
            printf("Philosopher %d: taking right fork %d\n", i, i+1);
        }
        
        printf("Philosopher %d: EATING.\n", i);
        
        sem_post(&forkArray[i]);
        printf("Philosopher %d: putting down left fork %d\n", i, i);
        
        if(i == 4){
            sem_post(&forkArray[0]);
            printf("Philosopher %d: putting down right fork 0\n", i);
        }
        
        else{
            sem_post(&forkArray[i+1]);
            printf("Philosopher %d: putting down right fork %d\n", i, i+1);
        }
        
        sem_post(&room);
        
        count++;
    }
   // pthread_exit(NULL);
    return 0;
}


int main(int argc, char *argv[]){
    pthread_t thread;
    int i;
    
    for(i=0; i<5; i++)
        sem_init(&forkArray[i], 1, 1);
        
    /*Deadlock strategy*/
    if(strcmp(argv[1], "1") == 0){
        for(i=0; i<5; i++)
            pthread_create(&thread, NULL, eatSpaghetti, (void *)(intptr_t) i);
    }
    
    /*No deadlock strategy*/
    else if(strcmp(argv[1], "0") == 0){
        sem_init(&room, 1, 4);
        for(i=0; i<5; i++)
            pthread_create(&thread, NULL, noDeadlockSpaghetti, (void *)(intptr_t) i);
    }
    else{
        printf("You have entered an incorrect argument. Enter 0 for no deadlock simulation or 1 for deadlock simulation.\n");
    }
    
    pthread_exit(NULL);
    return 0;
}