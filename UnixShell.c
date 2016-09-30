#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int PATH_MAX = 512;
int HISTORY_MAX = 10;
char arrowPrompt[3] = "->";
char dirPrompt[512];

void mergeFile(char *commands[]){                       //merges two files into a third new file
  FILE *fp1, *fp2, *fp3;
  char c;
  fp1 = fopen(commands[1], "r");
  fp2 = fopen(commands[2], "r");
  fp3 = fopen(commands[4], "w");
  if(fp1 == NULL || fp2 == NULL || fp3 == NULL){       //error opening one, print error and return
    printf("\nError opening files");
     fclose(fp1);
     fclose(fp2);
     fclose(fp3);
    return;
  }
  else{
    c = fgetc(fp1);                                   //cat the first file to the new file
    while(c != EOF){
      fputc(c, fp3);
      c = fgetc(fp1);
    }
    c = fgetc(fp2);                                  //cat the second file to the new file
    while(c != EOF){
      fputc(c, fp3);
      c = fgetc(fp2);
    }
    printf("\n");
  }                                                //close the files
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  return;
}

void commandExecute(char *commands[]){         //executes the users command with execvp
  int pid;
  int status;
  printf("\n");
  pid = fork();
  if(pid > 0){
    waitpid(pid, &status, WUNTRACED);
  }
  else {                                         //execute the command, if an error display a message
    int cmdError = execvp(commands[0], commands);
    if(cmdError == -1){
      printf("Command entered does not exist.\n");
      exit(1);
    }
  }
}

void tokenize(int *comCnt, int *pipeCheck, char *commands[], char inCommand[]){
  char *token;
  *comCnt = 0;
  *pipeCheck = 0;
  token = '\0';
  token = strtok(inCommand, " ");
  while(token) {                               //tokenize the command into a pointer array
    if(strcmp(token, "|") == 0){                //if the command contains the pipe split the commands
      *pipeCheck = 1;
    }
    commands[*comCnt] = token; 
    token = strtok(NULL," ");
    *comCnt = *comCnt + 1;
  }
}

void pipeProcess(char *commands[], int *pipeCheck, int comCnt){
  char *pipeCom1[10];
  char *pipeCom2[10];
  int pid, pidb, status, index;
  int pipefd[2];

  printf("\n");
  for(int i=0; i<comCnt; i++){                             //allows a pipe command with operate '|' to be used
    if(strcmp(commands[i], "|") == 0){                     //split the command into left and right of '|'
      index = i;
      *pipeCheck = 0;
    }
    else{
      if(*pipeCheck == 1){
	pipeCom1[i] = commands[i];
      }
      else if(*pipeCheck == 0){
	pipeCom2[i-index-1] = commands[i];
      }
    }
  }
  pipeCom1[index] = NULL;                                  //null terminate last element of command array
  pipeCom2[comCnt-index] = NULL;

  pipe(pipefd);
  pid = fork();

  if(pid == 0){
    close(0);
    dup(pipefd[0]);                      //child process, receives data from pipe for execution                                           
    close(pipefd[1]);
    execvp(pipeCom2[0], pipeCom2);
  }
  else{
    pidb = fork();
    if(pidb == 0){                  //other child process writes data to pipe for execution                                               
      close(1);
      dup(pipefd[1]);
      close(pipefd[0]);
      execvp(pipeCom1[0], pipeCom1);
    }
    else
      waitpid(0, &status, WUNTRACED); //parent process has to wait on children to finish before continuing execution
  }
  return;
}

void backspace(char inCommand[]){        //deletes the last character when pressed
  if(strlen(inCommand) != 0)
    {
      inCommand[strlen(inCommand) - 1] = '\0';
      printf("\b");
      printf(" ");
      printf("\b");
    }
  return;
}

int exitProgram(){                      //functionality for exiting the program 
  char exitCommand[PATH_MAX];
  int rePrompt = 1;
  int inChar = '\0';
  char cChar[2];

  memset(&exitCommand[0], 0, sizeof(exitCommand));

  while(rePrompt == 1){
    printf("\n%s%sDo you want to exit? y or n:", dirPrompt, arrowPrompt); //prompt user to exit, if yes exit
    while(inChar != '\n'){                                                //if no continue accepting commands                                      
      inChar = getchar();                                                 //if neither, reprompt them
      if(inChar == 0x7f)
	{
	  backspace(exitCommand);
	}
      else{
	printf("%c", inChar);
	sprintf(cChar, "%c", inChar);
	strcat(exitCommand, cChar);
      }
    }
    if(strcmp(exitCommand, "y\n") == 0){
      printf("goodbye");
      return 1;
    }
    else if(strcmp(exitCommand, "n\n") == 0){
      memset(&exitCommand[0], 0, sizeof(exitCommand));
      printf("%s%s", dirPrompt, arrowPrompt);
      inChar = '\0';
      break;
    }
    else{
      printf("Please enter a valid command to exit.");
      memset(&exitCommand[0], 0, sizeof(exitCommand));
      inChar = '\0';
    }
  }
  return 0;
}




int main(){
  int inChar = '\0';
  int arrowKey = 0;
  char inCommand[256] = "\0";
  char cChar[2];
  char temp[64];
  char *commandArr[128];
  char *cmdHistory[HISTORY_MAX];
  int comCnt;
  int exitState = 0;
  int cmdError = 0;
  int history_count = 0;
  int pipeCheck = 0;
  int upDownCount = 0;
  int charCount = 0;
  int last_press = 2;


  getcwd(dirPrompt, sizeof dirPrompt);
  struct termios origConfig, newConfig;
  tcgetattr(0, &origConfig);                             //save IO settings, set no echo, special chars, delete allowed
  newConfig = origConfig;
  newConfig.c_lflag &= ~(ICANON| ECHOE | ECHO);
  tcsetattr(0, TCSANOW, &newConfig);
  printf("%s%s", dirPrompt, arrowPrompt);
  memset(&inCommand[0], 0, sizeof(inCommand));
  
  for(int i=0; i<128; i++){
    commandArr[i] = NULL;
  }

  while(exitState == 0){
    inChar = getchar();
    charCount++;
    while(!((strcmp(inCommand, "exit") == 0) && (inChar == '\n')))   //continue to accept commands while user hasn't exited
      {
	if(inChar == '\n')          //command entered
	  {
	    if(inChar == '\n' && charCount == 1){
	      charCount = 0;
	    }
	    else{                                //command needs to be added to the history array
	      charCount = 0;
	      upDownCount = 0;
	      if(history_count == 0){
		history_count++;
		cmdHistory[0] = strdup(inCommand);
	      }
	      else{
		for(int i=history_count; i>0; i--){     //copy from the last element to the first so data isn't destroyed
		  if(i < 10){
		    cmdHistory[i] = strdup(cmdHistory[i-1]);
		  }
		}
		strcpy(cmdHistory[0], inCommand);
		if(history_count <11)
		  history_count++;
	      }
	      
	      tokenize(&comCnt, &pipeCheck, commandArr, inCommand);
	      commandArr[comCnt+1] = NULL;              //set the element after the last to null to indicate command ends
	      
	      if(strcmp(commandArr[0], "cd") == 0){           //if its a cd command execute chdir, print errors if chdir fails
		if(chdir(commandArr[1]) != 0){
		  fprintf(stderr, "\ncd to /%s failed. ", commandArr[1]);
		  perror("");
		}
		else{
		  getcwd(dirPrompt, sizeof dirPrompt);
		}
	      }
	      
	      else if(strcmp(commandArr[0], "merge") == 0){        //user entered a merge command call function
		strcpy(commandArr[0], "\0");
		strcpy(commandArr[0], "cat");
		mergeFile(commandArr);
	      }
	      
	      else{
		if(pipeCheck == 1){                            //pipe command enetered
		  pipeProcess(commandArr, &pipeCheck, comCnt);
		  printf("\x1B[A");
		}
		else{
		  commandExecute(commandArr);                  //use execvp to execute the command
		  printf("\x1B[A");
		}
	      }
	    }
	    
	    for(int i=0; i<20; i++)
	      commandArr[i] = NULL;
	    printf("\n%s%s", dirPrompt, arrowPrompt);
	    memset(&inCommand[0], 0, sizeof(inCommand));    //reset incoming command to empty
	  }
	else if(strlen(inCommand) == 255)                   //don't allow overflow of input
	  inChar = '\b';

	else if(inChar == 27)                               //special key pressed, ignore l/r, allow up/down
	  {                 
	    charCount = 0;                       //TO-DO ** make this a function
	    arrowKey = getchar();
	    arrowKey = getchar();

	    if(history_count > 0){                //user entered up key, check to see they have history, if they do display it
	      if(arrowKey == 65){        
		if(upDownCount >= 0 && upDownCount <= (history_count - 1)){
		  for(int i=0; i<sizeof(inCommand); i++)
		    backspace(inCommand);
		  upDownCount++;
		  strcpy(inCommand,cmdHistory[upDownCount-1]);            //once they reach the history end, nothing will happen
		  printf("%s", inCommand);
		  last_press = 1;
		}
	      }

	      else if(arrowKey == 66){                           //user entered the down key, check to see if they have history
		for(int i=0; i<sizeof(inCommand); i++)           //if they do display it.
		  backspace(inCommand);                          //once they reach the bottom of the history nothing happens
		
		if(upDownCount > 0 && upDownCount <= (history_count)){	
		  if(upDownCount == 1){
		    for(int j=0; j<sizeof(inCommand); j++)
		      backspace(inCommand);
		    upDownCount--;
		  }
		  else{
		    if(upDownCount > 1 && last_press == 1)
		      upDownCount = upDownCount - 1;
		    else if(upDownCount > 1 && last_press == 0)
		      upDownCount--;
		    strcpy(inCommand, cmdHistory[upDownCount-1]);
		    printf("%s", inCommand);
		  }
		  last_press = 0;
		}
	      }
	    }
	  }

	else  if(inChar == 0x7f)        //delete last character if delete/backspace pressed
	  {               
	    backspace(inCommand);
	    charCount--;
	  }

	else{
	  printf("%c", inChar);         //nothing else to be done, add the character to the command array
	  sprintf(cChar, "%c", inChar);
	  strcat(inCommand, cChar);
	}
	inChar = getchar();
	charCount++;
      }

    if(history_count == 0){                       //***TO_DO*** figure out how to make this work as a function
      history_count++;                            //for now adds exit command to history
      cmdHistory[0] = strdup(inCommand);
    }
    else{
      for(int i=history_count; i>0; i--){     //copy from the last element to the first so data isn't destroyed                                   
	if(i < 10){
	  cmdHistory[i] = strdup(cmdHistory[i-1]);
	}
      }
      strcpy(cmdHistory[0], inCommand);
      if(history_count <11)
	history_count++;
    }
    
    upDownCount = 0;
    charCount = 0;
    memset(&inCommand[0], 0, sizeof(inCommand));
    exitState = exitProgram();    //call function to see if user wants to exit
  }

  tcsetattr(0, TCSANOW, &origConfig);                        //reset the configuration of IO back to normal
  printf("\n");
  return 0;
}

