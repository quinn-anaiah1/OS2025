#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

		printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line); // tokenize the input string
   
       //do whatever you want with the commands, here we just print them

		for(i=0;tokens[i]!=NULL;i++){
			printf("found token %s (remove this debug output later)\n", tokens[i]);
			// creating new process
			pid_t pid =fork();

			if(pid<0){
				printf("Fork failed");
				perror("Fork failed");
			}else if (pid==0){
				printf("Child Process created successfully!\n");
				if(execvp(tokens[0],tokens) == -1){//execute the command
					perror("Command failed")
				}
				exit(0);// Exit child process

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
