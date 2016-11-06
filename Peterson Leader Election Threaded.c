#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
typedef int bool;       //boolean definition
#define true 1
#define false 0
int total;
/*contains a queue of messages[], head is the head location of queue, msgCount is 
  total messages in queue, race condition semaphor and syncronize threads semaphor*/
struct channel{
  int message[128];
  int head;
  int tail;
  sem_t race;
  sem_t sync;
}*channelArray;
 
/*node for threads, contains uid for id of thread, phase for current phase in leader
election, position for order the node was read in, tempUid for leader election message,
active for node being active or passive, and a pointer for singly linked list*/
struct node{
  int uid;
  int phase;
  int position;
  int tempUid;
  bool active;
  struct node *next;
}*start = NULL;
/*write the message to the index location (i+1)*/
void writeChannel(int index, int value){
  int i=0;
  if(index == total)
    index = 0;
  sem_wait(&channelArray[index].race);
  channelArray[index].message[channelArray[index].tail] = value;
  channelArray[index].tail++;
  sem_post(&channelArray[index].race);
  sem_post(&channelArray[index].sync);
}
/*read the message from the channel denoted by index,
  return the message to node at position index*/
int readChannel(int index){
  sem_wait(&channelArray[index].sync);
  sem_wait(&channelArray[index].race);
  int value = channelArray[index].message[channelArray[index].head];
  channelArray[index].head = channelArray[index].head + 1;
  sem_post(&channelArray[index].race);
  return value;
}
//logic for each thread, writes and reads messages from other threads
//threads exit once a leader is found
void *sendMessage(void * messenger){
  struct node *threadNode = (struct node*)messenger;
  int oneHop;
  int twoHop;
  int threadTempId;
    
  while(true){
    if(threadNode->active == true){
      printf("[%d][%d][%d]\n", threadNode->phase, threadNode->uid, threadNode->tempUid);
            
      writeChannel(threadNode->position+1, threadNode->tempUid);
      oneHop = readChannel(threadNode->position);
      writeChannel(threadNode->position+1, oneHop);
      twoHop = readChannel(threadNode->position);
            
      if(oneHop == threadNode->tempUid){
	printf("leader: %d\n", threadNode->uid);
	writeChannel(threadNode->position+1, -1);       //broadcast to all other nodes the leader is found
	pthread_exit(NULL);
      }
      else if((oneHop > twoHop)&&(oneHop > threadNode->tempUid)){
	threadNode->tempUid = oneHop;
      }
      else{
	threadNode->active = false;
      }
    }
    else if(threadNode->active == false){
      threadNode->tempUid = readChannel(threadNode->position);
      writeChannel(threadNode->position+1, threadNode->tempUid);
      if(threadNode->tempUid == -1)
	pthread_exit(NULL);
      threadNode->tempUid = readChannel(threadNode->position);
      writeChannel(threadNode->position+1, threadNode->tempUid);
    }
    else{}
        
    threadNode->phase = threadNode->phase + 1;
  }
}
int main(){
  pthread_t *threadIds;
  int inputID;
  int i=0;
  scanf("%d", &total);                    //total # of processes
  threadIds = malloc(total * sizeof(pthread_t));
  channelArray = malloc(total * sizeof(struct channel));
    
  /*create a new node for every line of input*/
  while(scanf("%d", &inputID) == 1){
    struct node *new_node, *current;
    new_node=(struct node*)malloc(sizeof(struct channel));
    new_node->uid = inputID;
    new_node->tempUid = inputID;
    new_node->position = i;
    new_node->phase = 1;
    new_node->next=NULL;
    new_node->active = true;
        
    if(start==NULL){
      start=new_node;
      current=new_node;
    }
    else{
      current->next=new_node;
      current=new_node;
    }
        
    sem_init(&channelArray[i].race, 1, 1);      //initialize channel semaphores
    sem_init(&channelArray[i].sync, 1, 0);
    i++;
  }
    
  struct node *current;
  current=start;
        
  /*create the threads, traverse through them by the linked list*/
  for(i=0; i<total; i++){
    channelArray[i].head = 0;
    channelArray[i].tail = 0;
    //printf("Making threads\n");
    pthread_create(&threadIds[i], NULL, (void *)sendMessage, (void *)current);
    current = current->next;
  }
    
  for(i=0; i<total; i++)
    pthread_exit(NULL);
  return 0;
}
