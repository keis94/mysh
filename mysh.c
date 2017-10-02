/* 1w130217-4 Keita Saito mail:kei_sai10@asagi.waseda.jp */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LEN (1024)
#define MAX_CMD_LEN (128)
#define MAX_PATH_LEN (2048)
#define MAX_CURRENT_DIR_LEN (512)

void cmdSplit(char* input, char** cmd);
void safePipe(int* fd);
pid_t safeFork();
void execCmd(char** cmd);
void cd(char* dir);
void intHandler(int child);

int main(){
	int status;
	char input[MAX_INPUT_LEN];
	char* cmd[MAX_CMD_LEN];
	int pathfile;
	char path[MAX_PATH_LEN];
	char cwd[MAX_CURRENT_DIR_LEN];
	pid_t pid;

	/* set environment */
	if((pathfile = open(".path", O_RDONLY|O_CREAT, 0666)) == -1){
		fprintf(stderr, "failed to open .path\n");
		exit(1);
	}
	if((read(pathfile, path, MAX_PATH_LEN)) == MAX_PATH_LEN){
		fprintf(stderr, "too long path setting\n");
	}
	setenv("PATH", path, 1);

	/* set signal(SIGINT) handler */
	signal(SIGINT, intHandler);

	while(1){
		printf("myprompt:%s> ", getcwd(cwd, MAX_CURRENT_DIR_LEN));

		/* get user input and split */
		fgets(input, MAX_INPUT_LEN, stdin);
		cmdSplit(input, cmd);
		
		/* if no input, continue immediately */
		if(cmd[0] == NULL) continue;

		/* if include embedded function, execute and continue */
		if(!strcmp(cmd[0], "cd")){
			cd(cmd[1]);
			continue;
		}
		if(!strcmp(cmd[0], "exit")) exit(0);

		/* otherwise, fork and execute command */
		pid = safeFork();
		if(pid == 0) execCmd(cmd);

		/* parent process */
		if(wait(&status) == (pid_t)-1){
			fprintf(stderr, "%s : No such file or directory\n",cmd[0]);
		}
		
	}

	return 0;
}

void cmdSplit(char* input, char** cmd){
	int pos = 0, vec = 0;

	while(input[pos] != '\0' && pos < MAX_INPUT_LEN && vec < MAX_CMD_LEN - 1){
		while(input[pos] == ' '){
			pos++;
			continue;
		}
		if(input[pos] == '\n') break;
		cmd[vec] = input + pos;
		vec++;
		while(input[pos] != ' ' && input[pos] != '\n') pos++;
		input[pos] = '\0';
		pos++;
	}
	cmd[vec] = NULL;
}

void safePipe(int* fd){
	if(pipe(fd) == -1){
		perror("pipe");
		exit(1);
	}
}

pid_t safeFork(){
	pid_t pid;
	
	if((pid = fork()) == -1){
		perror("fork");
		exit(1);
	}
	return pid;
}

void execCmd(char** cmd){
	int fd_redir;
	int fd_pipe[2];
	int head = 0;
	pid_t pid;

	/* search end of command */
	while(cmd[head] != NULL) head++;
	for(head--; ; head--){
		/* pipe */
		if(!strcmp(cmd[head], "|")){
			cmd[head] = NULL;
			safePipe(fd_pipe);
			pid = safeFork();
			
			/* parent process */
			if(pid != 0){
				/* pipe setting for input */
				dup2(fd_pipe[0], STDIN_FILENO);
				close(fd_pipe[0]);
				close(fd_pipe[1]);
				execvp(cmd[head+1], cmd + head+1);
				perror(cmd[head+1]);
				exit(1);
			}

			/* child process continues to search pipe or redirection
			/* pipe setting for input */
			dup2(fd_pipe[1], STDOUT_FILENO);
			close(fd_pipe[0]);
			close(fd_pipe[1]);
		
		/* output redirection */
		}else if(!strcmp(cmd[head], ">")){
			if((fd_redir = open(cmd[head+1], O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1){
				perror("open");
				exit(1);
			}
			dup2(fd_redir, STDOUT_FILENO);
			close(fd_redir);
			cmd[head] = NULL;
						
		/* input redirection */
		}else if(!strcmp(cmd[head], "<")){
			if((fd_redir = open(cmd[head+1], O_RDONLY)) == -1){
				perror("open");
				exit(1);
			}
			dup2(fd_redir, STDIN_FILENO);
			close(fd_redir);
			cmd[head] = NULL;

		/* finally exec from cmd[0] */
		}else if(head == 0){
			execvp(cmd[0], cmd);
			perror(cmd[0]);
			exit(1);
		}
		
	}
}

void cd(char* dir){
	if(chdir(dir) == -1){
		perror("cd");
	}
}

void intHandler(int child){
	kill(child, SIGINT);
}
