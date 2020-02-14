#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include<signal.h>
#include<fcntl.h>

#define MAX_LINE 128
#define MAX_ARG 32 

#define CREATE_FLAGS1 (O_WRONLY | O_CREAT | O_APPEND | O_TRUNC)
#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_APPEND )
#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
void splitPATH();

char *PATHS[20];
char *history[10][MAX_ARG];
int historyIndex = 9;

typedef struct Path {
    char *data;
    struct Path *next;
} Path;
Path *head = NULL;
void nothing(int signum){



    pid_t pid;

    switch (pid = fork()) {

        case -1:
            // On error fork() returns -1.
            perror("fork failed");

        case 0:
            // On success fork() returns 0 in the child.

            // Add code for the child process here.

            exit(EXIT_SUCCESS);

        default:
            // On success fork() returns the pid of the child to the parent.
            wait(NULL);
            // Add code for the parent process here.


    }


}
void append(struct Path **head_ref, char *new_data) {
    struct Path *new_node = (struct Path *) malloc(sizeof(struct Path));
    Path *last = *head_ref;
    new_node->data = (char *) malloc(sizeof(char *) * 100);
    strcpy(new_node->data, new_data);
    new_node->next = NULL;

    if (*head_ref == NULL) { //ll nullsa
        *head_ref = new_node;
        return;
    }
    while (last->next != NULL)
        last = last->next;
    last->next = new_node;
}

void printList(Path *n) {

    while (n != NULL) {

        printf("%s:", n->data);
        n = n->next;
    }
    printf("\n");
}

void deleteNode(struct Path **head_ref, char *key) {
    // Store head node
    struct Path *temp = *head_ref, *prev;
    if (temp != NULL && strcmp(temp->data, key) == 0) {
        *head_ref = temp->next;   // Changed head
        free(temp);               // free old head
        return;
    }

    while (temp != NULL && strcmp(temp->data, key) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;
    prev->next = temp->next;
    free(temp);  // Free memory
}

void traverseFiles(const char *cmd, char *args[]) {
    struct dirent *de;
    DIR *dir;
    for (int i = 0; i < 20; i++) {
        if (PATHS[i]) {
            dir = opendir(PATHS[i]);

            if (!dir)  // opendir returns NULL if couldn't open directory
            {

            } else {

                char *path = malloc(sizeof(char) * 150);
                strcpy(path, PATHS[i]);
                char *checkpath = strcat(path, "/");
                checkpath = strcat(checkpath, cmd);
                while ((de = readdir(dir)) != NULL) {

                    if (strcmp(de->d_name, cmd) == 0) {
                        args[0] = (char *) calloc(100, sizeof(char *));

                        strcpy(args[0], checkpath);


                    }

                }


                closedir(dir);

            }


        }

    }


}


void splitPATH() {  //splits path and puts in PATH array.

    const char *paths = getenv("PATH");
    int i = 0;
    char *apath = malloc(sizeof(char) * 100);
    int j = 0, k = 0;
    for (i = 0; i < strlen(paths); i++) {

        if (paths[i] == ':') {
            PATHS[k] = apath;
            apath = "";
            apath = (char *) malloc(sizeof(char) * 100);
            k++;
            j = 0;
        } else {
            apath[j] = paths[i];
            j++;
        }
    }

}

int callExecute;
int isRed;
void writeHistory() {
    printf(" History ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", i);
        for (int j = 0; j < MAX_ARG; j++) {
            if (history[i][j]) {
                printf("%s ", history[i][j]);
            }
        }
        printf("\n");

    }
}

void handle_sigtsp(int sig) {
    printf("ctrl + z \n %d",sig);
}
int execute(int background, char *args[MAX_ARG]) {


    __pid_t childpid;
    childpid = fork();
    if (childpid == -1) {
        fprintf(stderr, "Fork failed"); 
        return 1;
    }

    if (childpid == 0) {  //child's process
        execv(args[0], args);
    } else {  //parent's process

        if (background == 0) {
            wait(NULL);
        } else {
            // go on
        }
    }
}

void setup(char inputBuffer[], char *args[], int *background, int *callExecute) {
    int length, /* # of characters in the command line */
            i,      /* loop index for accessing inputBuffer array */
            start,  /* index where beginning of next command parameter is */
            ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

    signal(SIGTSTP, handle_sigtsp);


    if ((length < 0) && (errno != EINTR)) {
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    printf(">>%s<<", inputBuffer);
    for (i = 0; i < length; i++) { /* examine every character in the inputBuffer */

        switch (inputBuffer[i]) {
            case ' ':
            case '\t' :               /* argument separators */
                if (start != -1) {
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1) {
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            default :             /* some other character */
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&') {
                    *background = 1;
                    inputBuffer[i - 1] = '\0';
                }
        } /* end of switch */
    }    /* end of for */

    args[ct] = NULL; /* just in case the input line was > 80 */

    if (strcmp(args[0], "history") != 0) {
        for (int j = 0; j < MAX_ARG; j++) {
            if (args[j] == '\0') {
                history[historyIndex][j] = '\0';
                break;

            }
            history[historyIndex][j] = (char *) calloc(sizeof(char *), 100);
            strcpy((history[historyIndex][j]), args[j]);

        }
        if (historyIndex <= 0) {
            historyIndex = 9;
        } else {
            historyIndex--;
        }
    }

    if (strcmp(args[0], "history") == 0) {

        if (args[1]) {
            if (strcmp(args[1], "-i") == 0) {

                int k = atoi(args[2]);
                printf("\n");
                for (int i = 0; i < MAX_ARG; i++) {
                    args[i] = (char *) calloc(100, sizeof(char *));
                }
                for (int i = 0; i < MAX_ARG; i++) {
                    if (history[k][i] == '\0') {
                        args[i] = '\0';
                        break;
                    }

                    strcpy(args[i], history[k][i]);

                }
                for (int j = 0; j < MAX_ARG; j++) {  //writes history
                    if (args[j] == '\0') {
                        history[historyIndex][j] = '\0';
                        break;

                    }
                    history[historyIndex][j] = (char *) calloc(sizeof(char *), 100);
                    strcpy((history[historyIndex][j]), args[j]);

                } //writes history
                if (historyIndex <= 0) {
                    historyIndex = 9;
                } else {
                    historyIndex--;
                }

            }

        } else {
            writeHistory();

        }

    }


    if (strcmp(args[0], "path") == 0) {

        *callExecute = 1;
        if (!args[1]) {
            printList(head);
        } else {
            if (strcmp(args[1], "+") == 0) {
                append(&head, args[2]);
            }
            if (strcmp(args[1], "-") == 0) {
                deleteNode(&head, args[2]);
            }

        }


    }
    if (strcmp(args[0], "exit") == 0) {

        *callExecute = 1;
        exit(0);

    }


} /* end of setup routine */


int isRedirection(char * args[MAX_ARG]){

    int a;
    for(int i=0 ; i < MAX_ARG ; i++){
        if(args[i] == '\0') break;
        if((strcmp(args[i],"2>") == 0 || strcmp(args[i],">>") == 0 || strcmp(args[i],"<") == 0 || strcmp(args[i],">") == 0) ){
            a=i;
            callExecute = 0;
            int fd;
            if(strcmp(args[i],">>") == 0){
                isRed=1;
               for(int k=0 ; k <MAX_ARG ; k++){
                    if(args[k] == '\0'){

                        fd=open(args[k-1], CREATE_FLAGS,CREATE_MODE);

                        if(fd == -1){
                            fprintf(stderr,"Error open file");
                        }
                        break;
                    }
                }
                int old = dup(1);
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    fprintf(stderr,"Failed to redirect standard output");
                    return 1;

                }
                if (close(fd) == -1) {
                    fprintf(stderr,"Failed to close the file");

                    return 1;

                }
                for(int q=a+1; q < MAX_ARG; q++){
                    args[q]=(char *) calloc(100, sizeof(char *));
                }
                args[a]='\0'; 
               // dup2(old,STDOUT_FILENO);
                return 0;

            }else if(strcmp(args[i],"2>") == 0){

                isRed=1;
                int k=0;
                for(k=0 ; k <MAX_ARG ; k++){
                    if(args[k] == '\0'){

                        fd=open(args[k-1], CREATE_FLAGS1,CREATE_MODE);

                        if(fd == -1){
                            fprintf(stderr,"Error open file");
                        }
                        break;
                    }
                }
                int old = dup(3);
                if (dup2(fd, STDERR_FILENO) == -1) {
                    fprintf(stderr,"Failed to redirect standard output");
                    return 1;

                }
                if (close(fd) == -1) {
                    fprintf(stderr,"Failed to close the file");
                    return 1;

                }
                for(int q=a+1; q < MAX_ARG; q++){
                    args[q]=(char *) calloc(100, sizeof(char *));
                }
                args[a]='\0'; // sonunda bu olmazsa çalışmıyor
                //dup2(old,STDERR_FILENO);
                return 0;
            }else if(strcmp(args[i],">") == 0){
                isRed=1;
                int k=0;
                for(k=0 ; k <MAX_ARG ; k++){
                    if(args[k] == '\0'){

                        fd=open(args[k-1], CREATE_FLAGS1,CREATE_MODE);

                        if(fd == -1){
                            fprintf(stderr,"Error open file");
                        }
                        break;
                    }
                }
                int old = dup(1);
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    fprintf(stderr,"Failed to redirect standard output");
                    return 1;

                }
                if (close(fd) == -1) {
                    fprintf(stderr,"Failed to close the file");

                    return 1;

                }
                for(int q=a+1; q < MAX_ARG; q++){
                    args[q]=(char *) calloc(100, sizeof(char *));
                }
                args[a]='\0'; 
               // dup2(old,STDOUT_FILENO);
                return 0;

            }else{ // strcmp(args[i],"<") == 0

                isRed=1;
                int k=0;
                for(k=0 ; k <MAX_ARG ; k++){
                    if(args[k] == '\0'){
                        fd = open(args[k-1], CREATE_FLAGS,CREATE_MODE);
                        if(fd == -1){
                            fprintf(stderr,"Error open file");
                        }
                        break;
                    }
                }
                int old = dup(0);
                if (dup2(fd, STDIN_FILENO) == -1) {
                    fprintf(stderr,"Failed to redirect standard output");
                    return 1;
                }

                if (close(fd) == -1) {
                    fprintf(stderr,"Failed to close the file");

                    return 1;

                }
                for(int q=a+1; q < MAX_ARG; q++){
                    args[q]=(char *) calloc(100, sizeof(char *));
                }
                args[a]='\0'; // sonunda bu olmazsa çalışmıyor
                dup2(STDIN_FILENO,old);
                return 0;
            }
            break;

        }
    }

}

void initHistory() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < MAX_ARG; j++) {
            history[i][j] = (char *) malloc(sizeof(char) * 100);
            history[i][j] = "";
        }

    }

}


int main(void) {
    signal(SIGTSTP, nothing);
    char inputBuffer[MAX_LINE];
    int background;
    char *args[MAX_ARG];

    for (int j = 0; j < MAX_ARG; j++) {
        args[j] = (char *) malloc(sizeof(char) * 4000);
    }


    splitPATH();
    initHistory();

    while (1) {
        background = 0;
        printf("myshell: ");
        setup(inputBuffer, args, &background, &callExecute);
        isRedirection(args);
        if (callExecute == 0) {
            traverseFiles(args[0], args);
            execute(background, args);
        }
        callExecute = 0;


    }


}
