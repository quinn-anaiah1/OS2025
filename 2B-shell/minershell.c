/*
 * File: minershell.c
 * Author: Anaiah Quinn
 * Date: 20254-2-22 
 * Description: Minershell implementation for assignment 2B-shell
 * now inlcudes output redirection and basic pipe functionality
 */

#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

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
   
		
		//loop through and detect pipe command
		
		int pipeI = -1;
		for(i=0;tokens[i]!=NULL;i++){
			if(strcmp(tokens[i], "|")==0){
				pipeI=i; //store index of pipe
			}
		
		}
		// impmenting pipes
		if (pipeI >= 0) { // split tokens into left and right commands
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

			// //debugging 
			// printf("Left command: ");
			// for (i = 0; leftCmd[i] != NULL; i++) {
    		// 	printf("%s ", leftCmd[i]);
			// }
			// printf("\n");

			// printf("Right command: ");
			// for (i = 0; rightCmd[i] != NULL; i++) {
   	 		// 	printf("%s ", rightCmd[i]);
			// }
			// printf("\n");

			int pipe_fds[2];//array to hold pipe file descritors
			// pipe_fds[0] == read, [1] for write
			//create pipe
			if(pipe(pipe_fds) ==-1){
				perror("pipe");
				exit(1);
			}

			//left command
			pid_t pid_L = fork();
			
			if(pid_L<0){
				perror("Left fork failed");
			}else if (pid_L ==0){
				close(pipe_fds[0]);//close read in of pipe
				dup2(pipe_fds[1],STDOUT_FILENO);//redirect standard out to pipe write end
				close(pipe_fds[1]);//closse write end after redirecting

				printf("Executing left command: %s\n", leftCmd[0]);
				if (execvp(leftCmd[0], leftCmd)==-1){
					perror("execvp"); // If execvp fails
    				_exit(1);
				}
    			
			}
			// right command
			pid_t pid_R = fork();
			
			if(pid_R<0){
				perror("Right fork failed");
			}else if (pid_R ==0){
				close(pipe_fds[1]);//close write end in of pipe
				dup2(pipe_fds[0],STDIN_FILENO);//redirect standard input to pipe read end
				close(pipe_fds[0]);//closse wred end after redirecting
				
				// try cleaning string
				// Remove quotes from rightCmd arguments before executing
			for (int k = 0; rightCmd[k] != NULL; k++) {
    			int len = strlen(rightCmd[k]);
    			if ((rightCmd[k][0] == '"' && rightCmd[k][len - 1] == '"') || 
        			(rightCmd[k][0] == '\'' && rightCmd[k][len - 1] == '\'')) {
        			memmove(rightCmd[k], rightCmd[k] + 1, len - 2);  // Shift left to remove first char
        			rightCmd[k][len - 2] = '\0';  // Remove last char
    			}
			}

				if(execvp(rightCmd[0], rightCmd)== -1){
					perror("execvp failed for write command"); // If execvp fails
    				_exit(1);
				};

    			
			}
			close(pipe_fds[0]); // Close both ends in parent
            close(pipe_fds[1]);

            waitpid(pid_L, NULL, 0); // Wait for first child to finish
            waitpid(pid_R, NULL, 0); // Wait for second child to finish



		}else{// handling non pipe commands

		// creating new process
		pid_t pid =fork();

		if(pid<0){
			printf("Fork failed");
			perror("Fork failed");
		}else if (pid==0){

			int i = 0;
			bool redirect = false;
			char *outputFile = NULL;
		
			// Look for ">" in tokens
			for(i=0;tokens[i]!=NULL;i++){
				if (strcmp(tokens[i], ">") == 0) {
					redirect = true;
					outputFile = tokens[i + 1];
					tokens[i] = NULL;  // Remove ">" 
					break;
				}
			}		
			// If redirection is true and ouput file !=NULL
			if (redirect && outputFile) {
				int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd_out < 0) {
					perror("open");
					exit(1);
				}
				dup2(fd_out, STDOUT_FILENO);
				dup2(fd_out, STDERR_FILENO);
				close(fd_out);
			}
			if(execvp(tokens[0],tokens) == -1){//execute the commandA
				char error_message[256];
				snprintf(error_message, sizeof(error_message), "Command '%s' failed: ", tokens[0]);
				perror(error_message);
			}
			exit(0);// Exit child process
	
		
			
			
			
			// if(inputFile!=NULL){ // if input dected
			// 	int fd_in = open(inputFile, O_RDONLY);//open file, store file descriptor
			// 	if(fd_in<0){//error dectection
			// 		perror("Error opening input file");
			// 		exit(1);
			// 	}
			// 	dup2(fd_in,STDIN_FILENO); //redirect standard input to file
			// 	close(fd_in);//close file

			// }
			// if(outputFile!=NULL){ // if  output file detected // create if doesnt exist, if exist, truncate and overwrite
			// 	int fd_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);//| O_CREAT | O_TRUNC, 0666
			// 	if(fd_out<0){//error dectection
			// 		perror("Error opening output file");
			// 		exit(1);
			// 	}
			// 	printf( "Re directing output to File descriptor: %d\n", fd_out);
			// 	dup2(fd_out, STDOUT_FILENO);
			// 	// if(dup2(fd_out, STDOUT_FILENO)> 0){
			// 	// 	perror("Error redirecting file");
			// 	// 	exit(1);

			// 	// }
			// 	 //redirect standard output to file
			// 	//dup2(fd_out, STDERR_FILENO);
			// 	close(fd_out);//close file

			// }
			//debug print statement:echo ""
			// Debug: Print the command and arguments
			// printf("Executing command: %s\n", cmd[0]);
			// for (int i = 1; cmd[i] != NULL; i++) {
    		// 	printf("  Argument[%d]: %s\n", i, cmd[i]);
			// }
			// // printf("Executing: %s\n", cmd[0]);
			// 
		}else {
			waitpid(pid, NULL, 0); // Parent waits for child
		}
	}
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
