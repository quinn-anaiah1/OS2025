#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024 //Maximum size of the input
#define MAX_TOKEN_SIZE 64 // Maximum size of a single token
#define MAX_NUM_TOKENS 64 // Maximum number of tokens in a command

/*
 *Tokenizes the input string by splitting on spaces, tabs and newlines
 * Splits the string by space and returns the array of tokens (char pointers)
*
*/
char **tokenize(char *line){
	// allocate memory for storing tokens
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  //Loop through each character in the input line

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    // If we hit a space, newline, or tab, it marks the end of a token
    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';// Null-terminate the token

      if (tokenIndex != 0){ //Ignore empty tokens
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; // Reset token index for next token
      }
    } else {
      token[tokenIndex++] = readChar; // append charater to token
    }
  }
 
  free(token); // free temporary token buffer
  tokens[tokenNo] = NULL ; // null-terminate the tokens array
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE]; // BUffer for storing user input
	char  **tokens;              // array of tokens (command + arguments)
	int i;


	while(1) {// Infinite loop to keep the shell running
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));// clear input buffer
		printf("minersh$ ");// Display shell prompt
		scanf("%[^\n]", line);// read input until a newline is encountered
		getchar(); // consume the new line character left in buffer

		//printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line); // tokenize the input string

		if(tokens[0] == NULL){ //ignoring empty input
			continue;
		}
		if(strcmp(tokens[0],"exit") == 0){ //handle exit command
			printf("Exiting shell..\n");
			free(tokens);
			break;
		}
		//detect cd command
		if(strcmp(tokens[0], "cd") ==0){
			if(tokens[1] == NULL){ // if no directory provided
				printf("Shell: Incorrect command\n");
			}
			else{
				if(chdir(tokens[1]) !=0){ // change directory
					perror("Shell"); // if change directory fails
				}
			}
			free(tokens);
			continue;// skip to next prompt
		}
   
		
		//loop through and detect redirection commands
		int inputI = -1;
		int outputI = -1;
		int pipeI = -1l;
		for(i=0;tokens[i]!=NULL;i++){
			if(strcmp(tokens[i], "<")==0){
				inputI=i; //store index of reidrection input
			}else if (strcmp(tokens[i], ">")==0){
				outputI=i; // store index of redirection output
			}else if(strcmp(tokens[i], "|") ==0){
				pipeI = i; // store index of pipe
			}
		
		}
		// impmenting pipes
		if (pipeI >= 0) { 
			char *leftCmd[MAX_NUM_TOKENS];  // Left-side command
			char *rightCmd[MAX_NUM_TOKENS]; // Right-side command
		
			// Copy tokens before '|' into leftCmd[]
			for (i = 0; i < pipeI; i++) {
				leftCmd[i] = tokens[i];
			}
			leftCmd[pipeI] = NULL; // mark end of command
		
			// Copy tokens after '|' into rightCmd[]
			int j = 0;
			for (i = pipeI + 1; tokens[i] != NULL; i++) {
				rightCmd[j++] = tokens[i];
			}
			rightCmd[j] = NULL; // mark end of commad


		}
		
		char *cmd[MAX_NUM_TOKENS];  // Store the actual command
		char *inputFile = NULL;      // Store input redirection file name
		char *outputFile = NULL;     // Store output redirection file name

		int cmdSize = 0;

		//get actual command copy tokens without the > or <
		for (int i = 0; tokens[i] != NULL; i++) {
			if (i == inputI || i == outputI) {
				  // Stop at '<' or '>'
				  i++; //skip
			}
			cmd[cmdSize++] = tokens[i]; //copy will iteration size of cmd
		}
		cmd[cmdSize] = NULL; // mark end of the command

		if (inputI != -1 && tokens[inputI + 1] != NULL) { //if input redirected and the file name isnt null
			inputFile = tokens[inputI + 1];//store file name
		}
		
		if (outputI != -1 && tokens[outputI + 1] != NULL) { //if output redirected and the file name isnt null
			outputFile = tokens[outputI + 1]; //store file name
			tokens[outputI+1] == NULL
		}
		
		//

		// creating new process
		pid_t pid =fork();

		if(pid<0){
			printf("Fork failed");
			perror("Fork failed");
		}else if (pid==0){
			
			if(inputFile!=NULL){ // if input dected
				int fd_in = open(inputFile, O_RDONLY);//open file, store file descriptor
				if(fd_in<0){//error dectection
					perror("Error opening input file");
					exit(1);
				}
				dup2(fd_in,STDIN_FILENO); //redirect standard input to file
				close(fd_in);//close file

			}
			if(outputFile!=NULL){ // if  output file detected // create if doesnt exist, if exist, truncate and overwrite
				int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);//| O_CREAT | O_TRUNC, 0666
				if(fd_out<0){//error dectection
					perror("Error opening output file");
					exit(1);
				}
				printf( "Re directing output to File descriptor: %d\n", fd_out);
				dup2(fd_out, STDOUT_FILENO);
				// if(dup2(fd_out, STDOUT_FILENO)> 0){
				// 	perror("Error redirecting file");
				// 	exit(1);

				// }
				 //redirect standard output to file
				//dup2(fd_out, STDERR_FILENO);
				close(fd_out);//close file

			}
			//debug print statement:
			// Debug: Print the command and arguments
			printf("Executing command: %s\n", cmd[0]);
			for (int i = 1; cmd[i] != NULL; i++) {
    			printf("  Argument[%d]: %s\n", i, cmd[i]);
			}
			// printf("Executing: %s\n", cmd[0]);
			if(execvp(cmd[0],cmd) == -1){//execute the commandA
				char error_message[256];
    			snprintf(error_message, sizeof(error_message), "Command '%s' failed: ", cmd[0]);
    			perror(error_message);
			}
			exit(0);// Exit child process

		}else {
			waitpid(pid, NULL, 0); // Parent waits for child
		}
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
