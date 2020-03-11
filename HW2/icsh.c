#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int status;
int icsh_exit(char **args);
int icsh_echo(char **args);

char *my_fnStr[] = {
  "exit",
  "echo"
};

int (*my_fn[]) (char **) = {
  &icsh_exit,
  &icsh_echo
};

int icsh_process(char **args) {
    int fn_status;
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
    }
    if (pid == 0) {
        execvp(args[0], args);
        printf("Command not found: %s \n", args[0]);
        exit(EXIT_FAILURE);
  } else {
    wait(&fn_status); 
    if (WIFEXITED(fn_status))  {
      int exit_status = WEXITSTATUS(fn_status);  
      return exit_status;
    }
  } 
  return 1;
}

int icsh_exit(char **args) {
  return 93;
}

int exited (int retVal){
  if (retVal == 93) {
    return 1;
  } else {
    return 0;
  }
}

int icsh_echo(char **args){
  if (args[1] == NULL){
    printf("Wrong number of arguments");
  } else if (strcmp(args[1], "$?") == 0){
    printf("%d \n", status);
    return 0;
  }
  return icsh_process(args);
}

#define ICSH_BUFSIZE 1024

char *read_line(){
  size_t bufsize = ICSH_BUFSIZE;
  char *buffer = malloc(sizeof(char) * bufsize);
  int index = 0;

  if (!buffer) {
    printf("icsh: buffer allocation error\n");
    exit(EXIT_FAILURE);
  }

  getline(&buffer, &bufsize, stdin);
  return buffer;

  if (!buffer) {
    printf("icsh: buffer allocation error\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_FAILURE);
}

#define DELIM  " \r\t\n\a"

char **split_line(char *args){
  size_t bufsize = ICSH_BUFSIZE;
  char **tokens = malloc(sizeof(char*) * bufsize);
  char *token;
  int index = 0;

  if (!tokens) {
    printf("icsh: tokens allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(args, DELIM);

  while (token != NULL){
    tokens[index++] = token;
    token = strtok(NULL, DELIM);
  }

  if (!tokens) {
    printf("icsh: tokens allocation error\n");
    exit(EXIT_FAILURE);
  }
  tokens[index] = NULL;
  return tokens;
}

int execute_line(char **args, int status){
  if (args[0] == NULL){
    return 1;
  } 
  
  int fnNum = sizeof(my_fnStr) / sizeof(char *);

  for (int i = 0; i < fnNum; i++){
    if (strcmp(args[0], my_fnStr[i]) == 0) {
      return (* my_fn[i])(args);
    }
  }
  return icsh_process(args);
}

void crtlc_fn(){
  ;
}

void icsh_loop(void)
{
  char *line;
  char **args;
  do {
    signal(SIGINT, crtlc_fn);
    printf("icsh> ");
    line = read_line();
    args = split_line(line);
    status = execute_line(args, status);

    free(line);
    free(args);

    if (exited(status)) break;

  } while (1);
}



int main(int argc, char **argv)
{
  icsh_loop();
  return EXIT_SUCCESS;
}