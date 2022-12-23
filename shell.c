#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define clear() printf("\033[H\033[J")

void com_a(char* path) {
	FILE* ptr;
    char ch;
    int next = 1;
    int newline = 1;

    ptr = fopen(path, "r");
 
    if (NULL == ptr) {
        printf("file can't be opened \nuse this command as follow:\n\tfw [path of your file]\n");
    }

    while (!feof(ptr)) {
        ch = fgetc(ptr);
        if (ch == ' ') {
        	if (next) {
        		newline = 0;
        		printf("\n");
			}
        	next = 0;
		}else if (ch == '\n') {
			if (newline) {
				printf("\n");
			}
			newline = 1;
			next = 1;
		}
		if (next && ch != ' ' && ch != '\n') {
			printf("%c", ch);
		}
    }
    fclose(ptr);
}

void com_c(char* path) {
    FILE* ptr;
    char ch;

    ptr = fopen(path, "r");
 
    if (NULL == ptr) {
        printf("file can't be opened \nuse this command as follow:\n\trs [path of your file]\n");
    }

 
    while (!feof(ptr)) {
        ch = fgetc(ptr);
        if (ch != ' ' && ch != '\n' && ch != '\t') {
        	printf("%c", ch);
		}
    }
    fclose(ptr);
}

void init_shell() {
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****WELCOME TO OUR LINUX SHELL****");
    printf("\n\n\t-CREATED BY ALI && HADI-");
    printf("\n\n\n\n*******************"
        "***********************");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
    sleep(1);
    clear();
}

void parseSpace(char* str, char** parsed) {
    int i;
  
    for (i = 0; i < 100; i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

void printDir() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
	char* username = getenv("USER");
    printf("\n~%s %s:-$ ", username, cwd);
}

void execArgs(char** parsed) {
    // Forking a child
    pid_t pid = fork(); 
  
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
		if (strcmp(parsed[0], "cd") == 0) {
			chdir(parsed[1]);
		}else if (strcmp(parsed[0], "fw") == 0) {
			com_a(parsed[1]);
        }else if (strcmp(parsed[0], "rs") == 0) {
			com_c(parsed[1]);
        }else if (strcmp(parsed[0], "head") == 0) {
			if (execvp(parsed[0], parsed) < 0) {
				printf("\nCould not execute command..");
			}
		}else {
			if (execvp(parsed[0], parsed) < 0) {
				printf("\nCould not execute command..");
			}
		}
        exit(0);
    } else {
        wait(NULL); 
        return;
    }
}

void handle_sigint(int sig) {

}


int main() {

	char inputString[1000], *parsedArgs[100];
    char* parsedArgsPiped[100];
	init_shell();

    signal(SIGINT, handle_sigint);
  
    while (1) {
        printDir();
        gets(inputString);
        parseSpace(inputString, parsedArgs);
        execArgs(parsedArgs);
    }
	return 0;
}