// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")
void line_counter(char* file);
void most_word(char* file);
void getPrompt(char*);
void save_command(char*);

void remove_commands(char* path) {
    FILE *ptr;
    char str[50];
    ptr = fopen(path, "a+");

    if (NULL == ptr) {
        fprintf(stderr,"file can't be opened \n");
    }

    while (fgets(str, 50, ptr) != NULL) {
        if(str[0]!='#')
            printf("%s", str);
    }

    fclose(ptr);
}

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
        if (next && ch != ' ' && ch != '\n' && ch!=EOF) {
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
        if (ch != ' ' && ch != '\n' && ch != '\t'  && ch!='\r' && ch!=EOF) {
          printf("%c", ch);
        }
    }
    fclose(ptr);
}

void com_g(char* file){
    char cmd[100];
    sprintf(cmd,"head %s",file);
    system(cmd);
}

// Greeting shell during startup
void init_shell(){
    system("clear");
    printf(BLUE "   _     _     _     _     _     _     _     _     _   \n"
           " _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ \n"
           "|_   _|_   _|_   _|_   _|_   _|_   _|_   _|_   _|_   _|\n"
           "  |_|   |_|   |_|   |_|   |_|   |_|   |_|   |_|   |_|  \n");
    printf(RED "    _    _     ___             _   _    _    ____ ___ \n"
           "   / \\  | |   |_ _|           | | | |  / \\  |  _ \\_ _|\n"
           "  / _ \\ | |    | |    _____   | |_| | / _ \\ | | | | | \n"
           " / ___ \\| |___ | |   |_____|  |  _  |/ ___ \\| |_| | | \n"
           "/_/   \\_\\_____|___|           |_| |_/_/   \\_\\____/___|\n");
    printf(BLUE "   _     _     _     _     _     _     _     _     _   \n"
           " _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ _| |_ \n"
           "|_   _|_   _|_   _|_   _|_   _|_   _|_   _|_   _|_   _|\n"
           "  |_|   |_|   |_|   |_|   |_|   |_|   |_|   |_|   |_|  \n\033[0m");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
}

// Function to take input
int takeInput(char* str){
    char* buf;
    char prompt[3000];
    getPrompt(prompt);
    buf = readline(prompt);
    if (strlen(buf) != 0) {
        save_command(buf);
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

void save_command(char* com){
    FILE* file= fopen("history.txt","a");
    if (NULL == file) {
        fprintf(stderr,"file can't be opened \n");
    }
    fputs(com,file);
    fputs("\n",file);
    fclose(file);

}

   // Function to print Current Directory.
   void getPrompt(char* output){
       char cwd[1024],host[1024],str[3*1024];
       getcwd(cwd, sizeof(cwd));
       gethostname(host, sizeof(host));
       char* user = getenv("USER");
       sprintf(output,"\n\033[1;92m%s@%s \033[0;96m(%s)\033[0m: ", user, host, cwd);
   }

   // Function where the system command is executed
   void execArgs(char** parsed)
   {
       // Forking a child
       pid_t pid = fork();

       if (pid == -1) {
           fprintf(stderr,"\nFailed forking child..");
           return;
       } else if (pid == 0) {
           if (execvp(parsed[0], parsed) < 0) {
               fprintf(stderr,"\nCould not execute command..");
           }
           exit(0);
       } else {
           // waiting for child to terminate
           wait(NULL);
           return;
       }
   }

   // Function where the piped system commands is executed
   void execArgsPiped(char** parsed, char** parsedpipe)
   {
       // 0 is read end, 1 is write end
       int pipefd[2];
       pid_t p1, p2;

       if (pipe(pipefd) < 0) {
           fprintf(stderr,"\nPipe could not be initialized");
           return;
       }
       p1 = fork();
       if (p1 < 0) {
           fprintf(stderr,"\nCould not fork");
           return;
       }

       if (p1 == 0) {
           // Child 1 executing..
           // It only needs to write at the write end
           close(pipefd[0]);
           dup2(pipefd[1], STDOUT_FILENO);
           close(pipefd[1]);

           if (execvp(parsed[0], parsed) < 0) {
               fprintf(stderr,"\nCould not execute command 1..");
               exit(0);
           }
       } else {
           // Parent executing
           p2 = fork();

           if (p2 < 0) {
               fprintf(stderr,"\nCould not fork");
               return;
           }

           // Child 2 executing..
           // It only needs to read at the read end
           if (p2 == 0) {
               close(pipefd[1]);
               dup2(pipefd[0], STDIN_FILENO);
               close(pipefd[0]);
               if (execvp(parsedpipe[0], parsedpipe) < 0) {
                   fprintf(stderr,"\nCould not execute command 2..");
                   exit(0);
               }
           } else {
               // parent executing, waiting for two children
               wait(NULL);
               wait(NULL);
           }
       }
   }

   // Help command builtin
   void openHelp()
   {
       puts("\n***WELCOME TO MY SHELL HELP***"
            "\nCopyright @ Suprotik Dey"
            "\n-Use the shell at your own risk..."
            "\nList of Commands supported:"
            "\n>cd"
            "\n>ls"
            "\n>exit"
            "\n>all other general commands available in UNIX shell"
            "\n>pipe handling"
            "\n>improper space handling");

       return;
   }

   int getCmdType(char **parsed) {
       int NoOfOwnCmds = 10, i, switchOwnArg = 0;
       char *ListOfOwnCmds[NoOfOwnCmds];

       ListOfOwnCmds[0] = "exit";
       ListOfOwnCmds[1] = "cd";
       ListOfOwnCmds[2] = "help";
       ListOfOwnCmds[3] = "hello";

       ListOfOwnCmds[4] = "b";
       ListOfOwnCmds[5] = "d";
       ListOfOwnCmds[6] = "f";
       ListOfOwnCmds[7] = "a";
       ListOfOwnCmds[8] = "c";
       ListOfOwnCmds[9] = "g";

       for (i = 0; i < NoOfOwnCmds; i++) {
           if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
               switchOwnArg = i + 1;
               break;
           }
       }
       return switchOwnArg;
   }

   // Function to execute builtin commands
   void ownCmdHandler(char** parsed,int flag){
       char* username;
       pid_t pid = fork();
       if (pid == -1) {
           fprintf(stderr, "\nFailed forking child..");
           return;
       }else if (pid == 0) {
           switch (flag) {
               case 1:
                   printf("\nGoodbye\n");
                   exit(1);
               case 2:
                   if (chdir(parsed[1]) != 0)
                       fprintf(stderr,"chdir failed");
                   exit(2);
                   break;
               case 3:
                   openHelp();
                   break;

               case 4:
                   username = getenv("USER");
                   printf("\nHello %s.\nMind that this is "
                          "not a place to play around."
                          "\nUse help to know more..\n",
                          username);
                   break;

               case 5:
                   most_word(parsed[1]);
                   break;
               case 6:
                   remove_commands(parsed[1]);
                   break;
               case 7:
                   line_counter(parsed[1]);
                   break;
               case 8:
                   com_a(parsed[1]);
                   break;
               case 9:
                   com_c(parsed[1]);
                   break;
               case 10:
                   com_g(parsed[1]);
                   break;

               default:
                   break;
           }
           exit(0);
       } else if(pid>0){
           int stat;
           wait(&stat);
           if(WEXITSTATUS(stat)==1)
               exit(0);
           else if(WEXITSTATUS(stat)==2)
               chdir(parsed[1]);
       }
   }

   // function for finding pipe
   int parsePipe(char* str, char** strpiped)
   {
       int i;
       for (i = 0; i < 2; i++) {
           strpiped[i] = strsep(&str, "|");
           if (strpiped[i] == NULL)
               break;
       }

       if (strpiped[1] == NULL)
           return 0; // returns zero if no pipe is found.
       else {
           return 1;
       }
   }

   // function for parsing command words
   void parseSpace(char* str, char** parsed)
   {
       int i;

       for (i = 0; i < MAXLIST; i++) {
           parsed[i] = strsep(&str, " ");

           if (parsed[i] == NULL)
               break;
           if (strlen(parsed[i]) == 0)
               i--;
       }
   }

   int processString(char* str, char** parsed, char** parsedpipe)
   {

       char* strpiped[2];
       int piped = 0;

       piped = parsePipe(str, strpiped);

       if (piped) {
           parseSpace(strpiped[0], parsed);
           parseSpace(strpiped[1], parsedpipe);

       } else {

           parseSpace(str, parsed);
       }
        int cmdType=getCmdType(parsed);
       if (cmdType>0) {
           ownCmdHandler(parsed,cmdType);
           return 0;
       }else
           return 1 + piped;
   }

void line_counter(char* file){
    char cmd[100];
    sprintf(cmd,"wc -l %s | cut -d \" \" -f1",file);
    system(cmd);
}

void most_word(char* file){
    char cmd[100];
    sprintf(cmd,"sort %s | uniq -c | sort -r | head -n1 | cut -d \" \" -f8",file);
    system(cmd);
}

void handle_sigint(int sig){
    rl_reset_line_state();
    rl_replace_line("", 0);
    rl_redisplay();
}

   int main()
   {
       char inputString[MAXCOM], *parsedArgs[MAXLIST];
       char* parsedArgsPiped[MAXLIST];
       int execFlag = 0;
       signal(SIGINT, handle_sigint);
       init_shell();

       while (1) {
           // print shell line


           // take input
           if (takeInput(inputString))
               continue;
           // process
           execFlag = processString(inputString,
                                    parsedArgs, parsedArgsPiped);
           // execflag returns zero if there is no command
           // or it is a builtin command,
           // 1 if it is a simple command
           // 2 if it is including a pipe.

           // execute
           if (execFlag == 1)
               execArgs(parsedArgs);

           if (execFlag == 2)
               execArgsPiped(parsedArgs, parsedArgsPiped);
       }
       return 0;
   }


