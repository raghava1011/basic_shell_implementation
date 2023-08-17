#include  <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int cur_fg;
/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void proc_kill(int sig)
{
	if(sig == SIGINT)
		kill(cur_fg,SIGKILL);
}

int main(int argc, char* argv[]) {
	signal(SIGINT,SIG_IGN);
	char  line[MAX_INPUT_SIZE]; 
	char  path[MAX_INPUT_SIZE];           
	char  **tokens;              
	int i,j;
	char *s = "/home";
	int count = 0,status;
	int bg_child[64];

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		getcwd(path,MAX_INPUT_SIZE);
		printf("%s:$ ",path);
		scanf("%[^\n]", line);
		getchar();

		//printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		char bg = line[strlen(line)-2]; // To read the & symbol, if present.
		tokens = tokenize(line);
		int len = 0;
		while(tokens[len++]); //Finding number of tokens.
		len = len-1;
		
		/* reaping all the completed background processes before a new instruction*/
		for(int i=0;i<count;i++)
		{
			int res = waitpid(bg_child[i],&status,WNOHANG);
			if(res == -1)
			{
				printf("Wait has failed\n");
				exit(1);
			}
			else if(res>0)
			{
				printf("Background process with pid:%d is finished\n",res);
				bg_child[i] = 0;
			}
		}
		for(i=0,j=0;j<count;j++)
		{
			if(bg_child[j]>0)
				bg_child[i++] = bg_child[j];
		}
		count = i;
		if(tokens[0]==NULL)
		{
			free(tokens);
			continue;
		}
		else if(!strcmp(tokens[0],"exit"))
			{
				for(int i=0;i<count;i++)
					kill(bg_child[i],SIGKILL);
				for(int i=0;i<count;i++)
					waitpid(bg_child[i],&status,WNOHANG);
				for(i=0;tokens[i]!=NULL;i++){
					free(tokens[i]);
				}
				free(tokens);
				break;
			}
		else if(strcmp(tokens[0],"cd")==0)
			{
				if(len>2)
					printf("Shell : Incorrect Command \n");
				else if(tokens[1] == NULL)
				{
					chdir(s);
				}
				else{
					int cd = chdir(tokens[1]);
					if(cd==-1)
						printf("Shell : Incorrect Command\n");	
				}
			}
		// Creating new child process.
		else{
			signal(SIGINT,proc_kill);
			int pid = fork();
			if(bg != '&')
				cur_fg = pid;
			if(pid<0)
			{
				printf("fork has failed\n");
				exit(1);
			}
			else if(pid==0){	
					if(bg =='&')
					{
						tokens[len-1] = NULL;
					}
					int exec = execvp(tokens[0],tokens);
					if(exec == -1)
						printf("Shell : %s Command not found\n",tokens[0]);
			}
			else{
				setpgid(pid,pid);
				if(bg!='&'){
					int wc=waitpid(pid,&status,0);
				}
				else
				{
					bg_child[count++] = pid;
				}
					
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

