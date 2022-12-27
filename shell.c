#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>

#define RED "\033[1;31m"
#define GREEN "\033[1;92m"
#define CYAN "\033[0;96m"
#define RESET "\033[0m"

#define INPUT_SIZE 1000 // Maximum number of letters
#define COMMANDS 100 // Maximum number of commands

void printLogo();

void load_history(){
    FILE *ptr;
    char str[500];
    ptr = fopen("history.txt", "r");

    if (NULL == ptr) {
        return;
    }

    while (fgets(str, 500, ptr) != NULL) {
        str[strcspn(str, "\r\n")] = 0;
        add_history(str);
    }
    fclose(ptr);
}

void remove_comments(char* path) {
    FILE *ptr;
    char str[500];
    ptr = fopen(path, "r");

    if (NULL == ptr) {
        fprintf(stderr, RED"file can't be opened \n"RESET);
    }

    while (fgets(str, 500, ptr) != NULL) {
        if(str[0]!='#')
            printf("%s", str);
    }

    fclose(ptr);
}

void line_counter(char* file) {
    char cmd[100];
    sprintf(cmd,"wc -l %s | cut -d \" \" -f1",file);
    system(cmd);
}

void most_word(char* file) {
    char cmd[100];
    sprintf(cmd,"sort %s | uniq -c | sort -r | head -n1 | cut -d \" \" -f8",file);
    system(cmd);
}

void first_str(char* path) {
    FILE* ptr;
    char ch;
    int next = 1;
    int newline = 1;

    ptr = fopen(path, "r");

    if (NULL == ptr) {
        fprintf(stderr, RED"file can't be opened \nuse this command as follow:\n\tfs [path of your file]\n"RESET);
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

void remove_spaces(char* path) {
    FILE* ptr;
    char ch;

    ptr = fopen(path, "r");

    if (NULL == ptr) {
        fprintf(stderr, RED"file can't be opened \nuse this command as follow:\n\trs [path of your file]\n"RESET);
    }


    while (!feof(ptr)) {
        ch = fgetc(ptr);
        if (ch != ' ' && ch != '\n' && ch != '\t'  && ch!='\r' && ch!=EOF) {
          printf("%c", ch);
        }
    }
    fclose(ptr);
}

void first_ten(char* file){
    char cmd[100];
    sprintf(cmd,"head %s",file);
    system(cmd);
}

// clear everything and initialize shell
void init(){
    system("clear");
    printLogo();
    load_history();
    char* username = getenv("USER");
    printf("\n\nUsername: @%s", username);
    printf("\n");
}

// print Current Directory
void getDir(char* out) {
    char cwd[1024],host[1024],str[3*1024];
    getcwd(cwd, sizeof(cwd));
    gethostname(host, sizeof(host));
    char* user = getenv("USER");
    sprintf(out, "\n" GREEN "%s@%s" CYAN "(%s)" RESET ": ", user, host, cwd);
}

// save command in history file
void save_command(char* com){
    FILE* file= fopen("history.txt","a");
    if (NULL == file) {
        fprintf(stderr, RED"file can't be opened \n"RESET);
    }
    fprintf(file,"%s\n",com);
    fclose(file);
}

// get input from user
int getInput(char* str){
    char* history;
    char com[2000];
    getDir(com);
    history = readline(com);
    if (strlen(history) != 0) {
        save_command(history);
        add_history(history);
        strcpy(str, history);
        return 0;
    } else {
        return 1;
    }
}

// execute command
void executeCom(char** command) {
    // forking a child
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, RED"\nFailed forking child.."RESET);
        return;
    } else if (pid == 0) {
        if (execvp(command[0], command) < 0) {
            fprintf(stderr, RED"\nCould not execute command.."RESET);
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}

// execute piped commands
void executeComPiped(char** command, char** commandpipe) {
 
    int pipes[2];
    pid_t pid1, pid2;

    if (pipe(pipes) < 0) {
        fprintf(stderr, RED"\nPipe could not be initialized"RESET);
        return;
    }
    pid1 = fork();
    if (pid1 < 0) {
        fprintf(stderr, RED"\nCould not fork"RESET);
        return;
    }

    if (pid1 == 0) {
        // child 1
        close(pipes[0]);
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]);
        if (execvp(command[0], command) < 0) {
            fprintf(stderr, RED"\nCould not execute command 1.."RESET);
            exit(0);
        }
    } else {
        // parent
        pid2 = fork();

        if (pid2 < 0) {
            fprintf(stderr, RED"\nCould not fork"RESET);
            return;
        }

        // child 2
        if (pid2 == 0) {
            close(pipes[1]);
            dup2(pipes[0], STDIN_FILENO);
            close(pipes[0]);
            if (execvp(commandpipe[0], commandpipe) < 0) {
                fprintf(stderr, RED"\nCould not execute command 2.."RESET);
                exit(0);
            }
        } else {
            // parent wait for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

// help
void helpMe() {
    printf(CYAN"\n***********WELCOME TO OUR LINUX SHELL***********"
        GREEN"\nYou can use Linux commands and some additional commands in our shell."
        "\nThe new commands are as follows:"
        "\n\tfs => print the first string of each line of input file"
        "\n\tmw => print the most frequent word in file"
        "\n\trs => remove empty spaces from file"
        "\n\trmc => print uncommented lines of file"
        "\n\tlc => print the number of file lines"
        "\n\tft => print first ten lines of file"
        CYAN"\n*************************************************\n"RESET); 

    return;
}

int chooseCommand(char **command) {
    int nCom = 10;
    int i;
    int type = 0;
    char *commandList[nCom];

    commandList[0] = "exit";
    commandList[1] = "cd";
    commandList[2] = "help";
    commandList[3] = "hello";
    commandList[4] = "fs";
    commandList[5] = "mw";
    commandList[6] = "rs";
    commandList[7] = "rmc";
    commandList[8] = "lc";
    commandList[9] = "ft";

    for (i = 0; i < nCom; i++) {
        if (strcmp(command[0], commandList[i]) == 0) {
            type = i + 1;
            break;
        }
    }
    return type;
}

// execute additional commands
void handler(char** command,int flag) {
    char* username;
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, RED"\nFailed forking child.."RESET);
        return;
    }else if (pid == 0) {
        switch (flag) {
            case 1:
                printf("\nGoodbye\n");
                exit(1);
            case 2:
                if (chdir(command[1]) != 0)
                    fprintf(stderr, RED"chdir failed"RESET);
                exit(2);
                break;
            case 3:
                helpMe();
                break;
            case 4:
                username = getenv("USER");
                printf("\nHello %s.\n", username);
                break;
            case 5:
                first_str(command[1]);
                break;
            case 6:
                most_word(command[1]);
                break;
            case 7:
                remove_spaces(command[1]);
                break;
            case 8:
                remove_comments(command[1]);
                break;
            case 9:
                line_counter(command[1]);
                break;
            case 10:
                first_ten(command[1]);
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
            chdir(command[1]);
    }
}

// find pipe for pipelined commands
int findPipe(char* str, char** piped) {
    int i;
    for (i = 0; i < 2; i++) {
        piped[i] = strsep(&str, "|");
        if (piped[i] == NULL)
            break;
    }

    if (piped[1] == NULL)
        // no pipe found
        return 0; 
    else {
        return 1;
    }
}

// get command words
void getCommandWords(char* str, char** command) {
    int i;
    for (i = 0; i < COMMANDS; i++) {
        command[i] = strsep(&str, " ");

        if (command[i] == NULL)
            break;
        if (strlen(command[i]) == 0)
            i--;
    }
}

// proccess string for pipelined commands
int processString(char* str, char** command, char** commandpipe) {

    char* pipes[2];
    int piped = 0;

    piped = findPipe(str, pipes);

    if (piped) {
        getCommandWords(pipes[0], command);
        getCommandWords(pipes[1], commandpipe);

    }else {
        getCommandWords(str, command);
    }
    int type = chooseCommand(command);
    if (type > 0) {
        handler(command,type);
        return 0;
    }else
        return 1 + piped;
}

void handle_sigint(int sig) {
    rl_reset_line_state();
    rl_replace_line("", 0);
    rl_redisplay();
}

int main() {
    char input[INPUT_SIZE];
    char *commandParams[COMMANDS];
    char* commandParamsPiped[COMMANDS];
    int flag = 0;
    signal(SIGINT, handle_sigint);
    init();
    while (1) {
        if (getInput(input))
            continue;
        flag = processString(input, commandParams, commandParamsPiped);
        
        // simple command
        if (flag == 1)
            executeCom(commandParams);
        // piped command 
        if (flag == 2)
            executeComPiped(commandParams, commandParamsPiped);
    }
    return 0;
}

void printLogo(){
    printf("\033[49m                                                                          \033[m\n"
           "\033[49m                                                       \033[38;2;0;0;0;49m▄\033[38;2;41;41;40;49m▄\033[38;2;107;107;107;48;2;32;32;32m▄\033[38;2;105;105;105;48;2;124;124;124m▄\033[38;2;103;103;103;48;2;100;100;100m▄\033[38;2;101;100;100;48;2;55;55;55m▄\033[38;2;38;38;38;48;2;0;0;0m▄\033[38;2;0;0;0;49m▄\033[49m           \033[m\n"
           "\033[49m   \033[38;2;0;0;0;49m▄▄▄▄▄▄\033[49m     \033[48;2;0;0;0m   \033[38;2;0;0;0;49m▄\033[49m                                     \033[38;2;19;19;19;48;2;41;41;41m▄\033[38;2;41;40;39;48;2;75;75;75m▄\033[38;2;37;37;36;48;2;72;72;72m▄\033[38;2;35;34;33;48;2;67;67;67m▄\033[38;2;33;33;32;48;2;67;67;67m▄\033[38;2;30;29;28;48;2;64;64;63m▄\033[38;2;29;28;27;48;2;60;60;60m▄\033[38;2;4;4;4;48;2;12;12;11m▄\033[38;2;0;0;0;49m▄\033[49m          \033[m\n"
           "\033[49m     \033[48;2;0;0;0m   \033[49m      \033[49;38;2;0;0;0m▀▀▀\033[49m                               \033[38;2;0;0;0;49m▄▄▄\033[49m   \033[38;2;0;0;0;49m▄\033[38;2;103;102;102;48;2;14;14;14m▄\033[38;2;189;187;186;48;2;82;82;81m▄\033[38;2;118;117;117;48;2;12;12;12m▄\033[38;2;38;38;38;48;2;9;9;9m▄\033[38;2;125;123;123;48;2;116;115;115m▄\033[38;2;109;109;109;48;2;161;160;160m▄\033[38;2;72;72;72;48;2;10;10;10m▄\033[48;2;0;0;0m  \033[49m          \033[m\n"
           "\033[49m     \033[48;2;0;0;0m   \033[49m     \033[38;2;0;0;0;49m▄\033[48;2;0;0;0m   \033[49m \033[38;2;0;0;0;49m▄\033[48;2;0;0;0m        \033[38;2;0;0;0;49m▄\033[49m \033[48;2;0;0;0m    \033[49m \033[38;2;0;0;0;49m▄\033[48;2;0;0;0m   \033[49m \033[49;38;2;0;0;0m▀\033[48;2;0;0;0m    \033[49m \033[48;2;0;0;0m   \033[49;38;2;0;0;0m▀▀\033[49m     \033[38;2;82;62;25;48;2;104;103;103m▄\033[38;2;234;165;36;48;2;33;30;28m▄\033[38;2;240;180;39;48;2;224;169;64m▄\033[38;2;240;180;39;48;2;212;162;45m▄\033[38;2;242;181;37;48;2;110;88;63m▄\033[38;2;215;159;41;48;2;32;32;32m▄\033[38;2;95;66;27;48;2;87;86;86m▄\033[48;2;0;0;0m  \033[49m          \033[m\n"
           "\033[49m     \033[48;2;0;0;0m   \033[49m      \033[48;2;0;0;0m   \033[49m  \033[48;2;0;0;0m   \033[49m   \033[48;2;0;0;0m   \033[49m  \033[48;2;0;0;0m   \033[49m   \033[48;2;0;0;0m  \033[49m   \033[49;38;2;0;0;0m▀\033[48;2;0;0;0m    \033[49;38;2;0;0;0m▀\033[49m       \033[38;2;0;0;0;49m▄\033[38;2;41;30;23;48;2;202;140;30m▄\033[38;2;250;183;30;48;2;251;184;28m▄\033[38;2;252;185;28;48;2;242;179;31m▄\033[38;2;252;185;28;48;2;241;178;32m▄\033[38;2;251;185;29;48;2;250;184;29m▄\033[38;2;242;182;50;48;2;251;184;29m▄\033[38;2;159;158;157;48;2;213;148;47m▄\033[48;2;0;0;0m  \033[49m          \033[m\n"
           "\033[49m     \033[48;2;0;0;0m   \033[49m   \033[38;2;0;0;0;49m▄▄\033[49m \033[48;2;0;0;0m   \033[49m  \033[48;2;0;0;0m   \033[49m   \033[48;2;0;0;0m   \033[49m  \033[48;2;0;0;0m   \033[49m   \033[48;2;0;0;0m  \033[49m    \033[48;2;0;0;0m    \033[38;2;0;0;0;49m▄\033[49m      \033[38;2;0;0;0;49m▄\033[48;2;0;0;0m \033[38;2;104;104;104;48;2;85;84;84m▄\033[38;2;225;224;224;48;2;215;209;200m▄\033[38;2;229;229;229;48;2;234;202;150m▄\033[38;2;229;229;229;48;2;234;198;140m▄\033[38;2;229;229;229;48;2;228;216;189m▄\033[38;2;228;228;228;48;2;221;221;221m▄\033[38;2;206;205;204;48;2;194;194;194m▄\033[38;2;71;70;70;48;2;4;4;4m▄\033[48;2;0;0;0m \033[38;2;1;1;0;48;2;0;0;0m▄\033[38;2;8;8;4;49m▄\033[49m        \033[m\n"
           "\033[49m     \033[48;2;0;0;0m   \033[49m   \033[48;2;0;0;0m  \033[49m \033[48;2;0;0;0m   \033[49m  \033[48;2;0;0;0m   \033[49m   \033[48;2;0;0;0m   \033[49m  \033[48;2;0;0;0m   \033[38;2;0;0;0;49m▄▄▄\033[48;2;0;0;0m  \033[49m  \033[38;2;0;0;0;49m▄\033[48;2;0;0;0m  \033[49;38;2;0;0;0m▀▀\033[48;2;0;0;0m  \033[38;2;0;0;0;49m▄\033[49m    \033[38;2;36;36;36;48;2;0;0;0m▄\033[38;2;4;4;4;48;2;32;31;31m▄\033[38;2;15;15;15;48;2;0;0;0m▄\033[38;2;224;224;224;48;2;182;181;180m▄\033[38;2;224;224;224;48;2;216;216;215m▄\033[38;2;224;224;224;48;2;232;232;232m▄\033[38;2;224;224;224;48;2;231;231;231m▄\033[38;2;224;224;224;48;2;232;232;232m▄\033[38;2;224;224;224;48;2;217;216;216m▄\033[38;2;224;224;224;48;2;216;215;214m▄\033[38;2;224;224;224;48;2;141;140;140m▄\033[38;2;12;12;12;48;2;0;0;0m▄\033[38;2;146;146;146;48;2;0;0;0m▄\033[38;2;1;1;1;48;2;21;20;19m▄\033[38;2;22;22;22;48;2;19;19;19m▄\033[49m       \033[m\n"
           "\033[49m   \033[49;38;2;0;0;0m▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀\033[49m \033[49;38;2;0;0;0m▀▀▀▀▀\033[49m  \033[49;38;2;0;0;0m▀▀▀▀\033[49m \033[49;38;2;0;0;0m▀▀▀▀▀▀▀\033[49m \033[49;38;2;0;0;0m▀▀▀▀▀\033[49m  \033[38;2;57;57;57;48;2;8;8;8m▄\033[38;2;49;49;49;48;2;85;85;85m▄\033[38;2;9;9;9;48;2;12;12;12m▄\033[38;2;234;234;234;48;2;173;173;173m▄\033[38;2;240;240;240;48;2;233;233;233m▄▄▄▄▄▄▄▄\033[38;2;237;237;237;48;2;154;154;154m▄\033[38;2;0;0;0;48;2;5;5;5m▄\033[38;2;134;134;134;48;2;15;15;15m▄\033[38;2;8;8;8;48;2;27;26;26m▄\033[38;2;41;41;41;48;2;6;6;6m▄\033[49m      \033[m\n"
           "\033[49m                    \033[38;2;241;132;36;49m▄\033[38;2;236;118;41;49m▄\033[38;2;236;120;41;49m▄\033[38;2;237;115;42;49m▄\033[38;2;236;112;42;49m▄\033[38;2;236;107;42;49m▄\033[38;2;234;103;42;49m▄\033[38;2;235;101;43;49m▄\033[38;2;235;93;43;49m▄\033[38;2;233;93;44;48;2;235;98;39m▄\033[38;2;233;87;43;48;2;240;90;45m▄\033[38;2;234;85;43;48;2;243;85;49m▄\033[38;2;233;81;43;48;2;224;82;41m▄\033[38;2;232;77;43;48;2;224;71;41m▄\033[38;2;232;75;43;48;2;234;74;42m▄\033[38;2;232;70;43;48;2;224;71;41m▄\033[38;2;232;68;42;48;2;224;61;41m▄\033[38;2;232;66;42;48;2;234;64;42m▄\033[38;2;232;64;42;48;2;230;83;45m▄\033[38;2;231;62;42;48;2;232;73;42m▄\033[38;2;231;59;42;48;2;232;65;42m▄\033[38;2;231;58;42;48;2;233;67;40m▄\033[38;2;231;55;42;48;2;232;66;43m▄\033[38;2;231;54;42;48;2;232;59;42m▄\033[38;2;231;54;42;48;2;232;59;41m▄\033[38;2;231;53;42;48;2;232;58;42m▄\033[38;2;231;53;42;48;2;230;62;42m▄\033[38;2;231;55;42;48;2;237;59;42m▄\033[38;2;233;55;44;49m▄\033[49m \033[38;2;7;7;6;48;2;0;0;0m▄\033[38;2;68;67;67;48;2;43;42;42m▄\033[38;2;38;38;38;48;2;115;115;115m▄\033[38;2;249;249;249;48;2;124;124;123m▄\033[38;2;250;250;250;48;2;245;245;245m▄▄▄▄▄▄▄\033[38;2;250;250;250;48;2;246;246;246m▄▄▄\033[38;2;233;233;233;48;2;132;132;132m▄\033[38;2;4;4;3;48;2;2;2;2m▄\033[38;2;107;107;107;48;2;54;54;53m▄\033[38;2;41;40;40;48;2;45;45;45m▄\033[38;2;1;1;1;48;2;0;0;0m▄\033[49m     \033[m\n"
           "\033[49m     \033[38;2;247;207;4;49m▄\033[38;2;248;203;7;49m▄\033[38;2;247;192;13;49m▄\033[38;2;247;189;16;49m▄\033[38;2;246;185;19;49m▄\033[38;2;245;181;21;48;2;247;175;24m▄\033[38;2;245;175;23;48;2;243;171;26m▄\033[38;2;244;170;26;48;2;242;169;25m▄\033[38;2;243;164;28;48;2;242;163;27m▄\033[48;2;242;158;30m \033[38;2;241;153;32;48;2;242;154;32m▄\033[38;2;241;148;35;48;2;241;149;34m▄\033[38;2;240;143;37;48;2;240;144;37m▄\033[38;2;239;139;37;48;2;239;138;37m▄\033[48;2;238;134;39m \033[48;2;238;129;39m \033[48;2;237;124;40m \033[48;2;237;120;41m \033[48;2;236;115;42m \033[48;2;235;111;42m \033[48;2;235;107;42m \033[48;2;235;103;42m \033[48;2;235;100;43m \033[48;2;234;95;43m \033[38;2;233;91;43;48;2;233;92;43m▄\033[48;2;233;88;43m \033[48;2;233;84;43m \033[48;2;233;81;43m \033[38;2;232;77;42;48;2;232;77;43m▄\033[38;2;231;76;43;48;2;232;75;43m▄\033[38;2;232;71;43;48;2;232;69;43m▄\033[38;2;233;67;42;48;2;232;68;42m▄\033[38;2;231;65;42;48;2;232;65;42m▄\033[38;2;232;71;43;48;2;232;64;42m▄\033[38;2;216;78;39;48;2;231;62;42m▄\033[49;38;2;231;60;42m▀\033[49;38;2;231;59;42m▀▀\033[49;38;2;232;57;42m▀\033[49;38;2;230;54;42m▀\033[49;38;2;232;60;42m▀\033[49;38;2;230;68;44m▀\033[49m  \033[48;2;0;0;0m \033[38;2;7;7;6;48;2;21;21;21m▄\033[38;2;67;64;62;48;2;72;72;72m▄\033[38;2;204;204;204;48;2;139;138;138m▄\033[38;2;254;254;254;48;2;252;252;252m▄\033[38;2;254;254;254;48;2;253;253;253m▄▄▄▄▄▄▄▄▄▄\033[38;2;102;101;101;48;2;247;247;247m▄\033[38;2;90;90;90;48;2;190;189;189m▄\033[38;2;118;118;118;48;2;134;134;134m▄\033[38;2;176;176;176;48;2;41;41;41m▄\033[48;2;1;1;1m \033[49m     \033[m\n"
           "\033[49m   \033[38;2;250;213;1;49m▄\033[38;2;250;212;3;48;2;250;211;4m▄\033[48;2;249;207;6m \033[48;2;249;202;8m \033[48;2;248;197;12m \033[48;2;247;191;15m \033[48;2;246;186;18m \033[48;2;245;180;21m \033[48;2;243;175;25m \033[48;2;244;170;26m \033[38;2;242;166;28;48;2;241;166;29m▄\033[48;2;242;159;30m \033[38;2;242;155;32;48;2;241;153;33m▄\033[48;2;241;148;35m \033[48;2;240;143;37m \033[38;2;240;140;37;48;2;239;139;37m▄\033[48;2;238;134;39m \033[48;2;238;129;39m \033[48;2;237;124;40m \033[38;2;236;123;40;48;2;237;120;41m▄\033[38;2;235;114;42;48;2;236;115;42m▄\033[38;2;236;112;42;48;2;235;111;42m▄\033[38;2;232;102;42;48;2;236;108;42m▄\033[38;2;232;104;35;48;2;235;103;42m▄\033[49;38;2;235;101;42m▀\033[49;38;2;234;95;42m▀\033[49;38;2;233;86;42m▀\033[49;38;2;231;89;43m▀\033[49;38;2;223;112;48m▀\033[49m                \033[38;2;196;113;65;49m▄\033[38;2;245;200;17;48;2;164;100;55m▄\033[38;2;250;225;69;48;2;240;190;11m▄\033[38;2;247;227;83;48;2;255;213;1m▄\033[38;2;241;209;42;48;2;163;109;61m▄\033[38;2;4;4;4;48;2;250;250;250m▄\033[38;2;176;176;176;48;2;255;255;255m▄\033[48;2;255;255;255m        \033[38;2;243;195;39;48;2;244;216;180m▄\033[38;2;171;156;60;48;2;29;29;28m▄\033[38;2;5;5;5;48;2;79;79;78m▄\033[38;2;37;31;14;48;2;25;25;24m▄\033[38;2;209;174;6;48;2;44;44;43m▄\033[38;2;247;194;11;48;2;15;12;6m▄\033[38;2;224;139;40;49m▄\033[49m    \033[m\n"
           "\033[49m   \033[38;2;250;213;2;48;2;251;215;1m▄\033[48;2;250;212;3m \033[48;2;249;207;6m \033[48;2;249;202;8m \033[48;2;248;197;12m \033[48;2;247;191;15m \033[48;2;246;186;18m \033[38;2;245;181;21;48;2;245;180;22m▄\033[38;2;245;175;23;48;2;244;175;24m▄\033[38;2;243;170;25;48;2;243;170;27m▄\033[38;2;241;165;30;48;2;243;164;28m▄\033[38;2;244;156;28;48;2;242;158;30m▄\033[49;38;2;242;156;32m▀\033[49;38;2;240;146;35m▀\033[49;38;2;239;142;37m▀\033[49;38;2;240;139;38m▀\033[49;38;2;238;124;40m▀\033[49m                           \033[38;2;240;183;19;48;2;227;161;35m▄\033[38;2;254;213;6;48;2;254;212;4m▄\033[38;2;244;224;68;48;2;248;226;75m▄\033[38;2;243;222;52;48;2;245;225;74m▄\033[38;2;240;219;36;48;2;244;223;62m▄\033[38;2;238;217;20;48;2;243;220;43m▄\033[38;2;248;210;5;48;2;189;139;22m▄\033[38;2;56;38;11;48;2;0;0;0m▄\033[38;2;15;15;15;48;2;177;177;177m▄\033[48;2;255;255;255m      \033[38;2;248;245;243;48;2;255;255;255m▄\033[38;2;248;215;12;48;2;250;217;19m▄\033[38;2;240;219;31;48;2;243;223;59m▄\033[38;2;240;219;33;48;2;238;218;64m▄\033[38;2;241;221;42;48;2;245;225;69m▄\033[38;2;246;217;26;48;2;248;222;56m▄\033[48;2;255;212;0m \033[38;2;231;162;32;48;2;228;169;31m▄\033[49m    \033[m\n"
           "\033[49m   \033[38;2;250;214;2;48;2;250;213;2m▄\033[48;2;250;212;3m \033[48;2;249;207;6m \033[48;2;249;202;8m \033[38;2;248;199;10;48;2;248;197;12m▄\033[38;2;245;188;17;48;2;247;191;15m▄\033[49;38;2;246;184;19m▀\033[49;38;2;243;182;20m▀\033[49;38;2;240;165;30m▀\033[49m                                   \033[38;2;245;195;11;48;2;244;197;11m▄\033[48;2;255;212;0m \033[38;2;255;212;0;48;2;252;213;8m▄\033[38;2;250;213;5;48;2;238;217;22m▄\033[38;2;254;212;1;48;2;238;217;18m▄\033[38;2;255;212;0;48;2;250;213;4m▄\033[48;2;255;212;0m \033[38;2;255;212;0;48;2;245;203;1m▄\033[38;2;242;193;67;48;2;195;192;190m▄\033[38;2;254;254;254;48;2;255;255;255m▄\033[48;2;255;255;255m    \033[38;2;225;225;225;48;2;255;255;255m▄\033[38;2;188;159;38;48;2;239;198;167m▄\033[38;2;255;212;0;48;2;250;213;4m▄\033[38;2;255;212;0;48;2;243;214;12m▄\033[38;2;255;212;0;48;2;238;217;18m▄\033[38;2;255;212;0;48;2;238;217;19m▄\033[48;2;255;212;0m   \033[38;2;237;174;25;48;2;243;194;13m▄\033[49m   \033[m\n"
           "\033[49m   \033[49;38;2;250;206;5m▀\033[38;2;247;203;8;48;2;250;212;3m▄\033[49;38;2;249;206;6m▀\033[49;38;2;247;205;8m▀\033[49m                                        \033[38;2;235;164;28;48;2;251;198;7m▄\033[38;2;253;211;2;48;2;255;212;0m▄\033[38;2;253;210;2;48;2;255;212;0m▄\033[38;2;251;202;6;48;2;255;212;0m▄\033[38;2;255;211;1;48;2;255;212;0m▄\033[48;2;255;212;0m   \033[38;2;241;174;21;48;2;252;203;6m▄\033[38;2;0;0;0;48;2;19;18;18m▄\033[38;2;0;0;0;48;2;150;150;150m▄\033[38;2;0;0;0;48;2;155;155;155m▄\033[38;2;0;0;0;48;2;154;154;154m▄\033[38;2;0;0;0;48;2;63;63;63m▄\033[48;2;0;0;0m \033[38;2;250;194;10;48;2;223;172;13m▄\033[48;2;255;212;0m   \033[38;2;254;209;2;48;2;255;212;0m▄\033[38;2;240;170;25;48;2;255;212;0m▄\033[49;38;2;239;179;21m▀\033[49;38;2;226;149;40m▀\033[49m    \033[m\n"
           "\033[49m                                                  \033[49;38;2;207;120;56m▀\033[49;38;2;219;147;41m▀\033[49;38;2;242;175;23m▀\033[49;38;2;253;210;1m▀\033[49;38;2;242;170;25m▀\033[49;38;2;197;128;55m▀\033[49m      \033[38;2;195;105;60;48;2;241;170;24m▄\033[38;2;232;147;37;48;2;255;211;0m▄\033[38;2;241;188;18;48;2;255;212;0m▄\033[38;2;209;128;55;48;2;255;212;0m▄\033[49;38;2;236;160;30m▀\033[49m       \033[m"
           );
}