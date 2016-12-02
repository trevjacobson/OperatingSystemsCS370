#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER 0
#define CLIENT 1

void printCritical();
void communicate(int serverDescriptor, int clientDescriptor, int *myVal, int *neighborValue);
int createServerConnection(char *socketName);
int createClientConnection(char *socketName);
char *getSocketName(int index1, int index2);


int id;

int main () {
	int clientDescriptor, serverDescriptor, neighborValue, totalNodes, value, serverID, clientID;
	char buff[20];
	char *leftNeighbor, *rightNeighbor;
	scanf("%d %d", &id, &totalNodes);
	
	//left neighbor
	if(id > 0) 
	    clientID = id - 1;
	//bottom
	else
	    clientID = totalNodes - 1;
	 
	serverID = (id + 1) % totalNodes;
	value = rand() % totalNodes;
	
	//bottom
	if (id == 0)
	{
	    rightNeighbor = getSocketName(id, serverID);
        leftNeighbor   = getSocketName(id, clientID);
        serverDescriptor = createServerConnection(rightNeighbor);
        clientDescriptor  = createServerConnection(leftNeighbor);
	}
	//last node
	else if(id == totalNodes - 1)
	{
	    rightNeighbor = getSocketName(serverID, id);
        leftNeighbor   = getSocketName(clientID, id);
        clientDescriptor  = createClientConnection(leftNeighbor);
        serverDescriptor = createClientConnection(rightNeighbor);
	}
	else 
	{   //middle nodes
        rightNeighbor = getSocketName(id, serverID);
        leftNeighbor   = getSocketName(clientID, id);
        clientDescriptor   = createClientConnection(leftNeighbor);
        serverDescriptor  = createServerConnection(rightNeighbor);
    }    

    //error creating client/server
    if ((clientDescriptor < 0 )||(serverDescriptor < 0 )) 
        return 0;
        
    while(1) {
        if(id == 0) {
            communicate(serverDescriptor, clientDescriptor, &value, &neighborValue);
            if(value == neighborValue) {
                printCritical();
                value = (value + 1) % totalNodes;
            }
        }
        else {
            communicate(serverDescriptor, clientDescriptor, &value, &neighborValue);
            if(value != neighborValue) {
                printCritical();
                value = neighborValue;
            }
        }
    }
	
	return 0;
}

char *getSocketName(int index1, int index2) {
    char *socketName = (char *)calloc(256, sizeof(char));
    char buff[20];
    
    sprintf(buff, "%d", index1);
    strcpy(socketName, buff);
    strcat(socketName, "sends to");
    sprintf(buff, "%d", index2);
    strcat(socketName, buff);
    return socketName;
}

int createClientConnection(char* socketName) {
	int descriptor;
	
	if ((descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		printf("Error creating socket [%s]\n", socketName);
		return -1;
	}

	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, socketName);
	unsigned int addressLength = sizeof(address);
//Information like IP address of the remote host and its port is bundled up in a structure and a call to function connect() is made which tries to connect this socket with the socket (IP address and port) of the remote host.
	if (connect(descriptor, (struct sockaddr*)&address, addressLength) < 0) {
		printf("Error connecting socket [%s]\n", socketName);
		return -1;
	}

	return descriptor;
}

int createServerConnection(char* socketName) {
	int descriptor;
	//To create endpoint for communication
	if ((descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		printf("Error creating socket [%s]\n", socketName);
		return -1;
	}
	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, socketName);
	unsigned int addressLength = sizeof(address);
  //To remove a socket if it exists. Its called before bind()

	unlink(socketName);
	
   //To assign a local socket address.
	if (bind(descriptor, (struct sockaddr*)&address, addressLength) < 0) {
		printf("Error binding socket [%s]\n", socketName);
		return -1;
	}
	
	//To instruct a socket to listen for incoming
    //connections. It specifies the number of pending connections
    //that can be queued for a server socket.

   //After the call to listen(), this socket becomes a fully functional listening socket.
	if (listen(descriptor, 1) < 0) {
		printf("Error listening to socket [%s]\n", socketName);
		return -1;
	}
	int connectedDescriptor;
	//To accept the connection request from a client
	if ((connectedDescriptor = accept(descriptor, (struct sockaddr*) &address, &addressLength)) < 0) {
		printf("Error accepting socket connection [%s]\n", socketName);
		return -1;
	}
	return connectedDescriptor;
}

void printCritical() {
    printf("#####################################\n");
    printf("\tIn Critical Section\n");
    printf("#####################################\n\n\n");
    sleep(1);  
}

void communicate(int serverDescriptor, int clientDescriptor, int* myVal, int* neighborValue) {
    //To send a message via socket
    send(serverDescriptor, myVal, sizeof(int), 0);

    //To receive message via socket
    recv(clientDescriptor, neighborValue, sizeof(int), 0);
}