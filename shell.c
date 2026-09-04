#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define deliminator " "

static char* initPath = "/bin/";
static char* usrPath = "/usr/bin/";
const char* builtInFunction[] = {"pancake_exit", "[pancake_cd"};

void pancake_exit() {
  exit(0);
}

void pancake_cd(char** args) {
  if (args[2] != NULL) {
    printf("Too many arguments\n");
    return;
  }  
  if (chdir(args[1]) == -1) {
    printf("Path not found\n");
  }
  return;
}

// Parses given line and returns pointer of char pointers
char **parseLine(char *line) {
  int argsize = 10;
  char *temp;
  char **args = malloc(sizeof(char *) * argsize);
  if (args == NULL) {
    printf("malloc error");
    exit(1);
  }
  int i = 0;
  temp = strsep(&line, deliminator);
  while (temp != NULL) {
    args[i] = temp;
    // Realloc if theres not enough space
    if (i >= argsize) {
      argsize += argsize;
      args = realloc(args, sizeof(char *) * argsize);
      if (args == NULL) {
        printf("allocaiton error");
        exit(1);
      }
    }
    temp = strsep(&line, deliminator);
    i++;
  }
  return args;
}

// Reads standard input and returns char pointer
char *readLine() {
  size_t size = 0;
  char *buf = NULL;
  int temp;
  if ((temp = getline(&buf, &size, stdin)) == -1) {
    if (feof(stdin)) {
      exit(0);
    }
  }
  buf[temp - 1] = '\0';
  return buf;
}

// Execute given arguments
void executeArg(char **args) {
  pid_t pID = fork();
  if (pID < 0) {
    printf("fork failure");
    exit(1);
  } else if (pID == 0) { // Child
    execvp(args[0], args);
    printf("%s\n", strerror(errno));
  } else { // Parent
    waitpid(pID, NULL, WUNTRACED);
  }
}

// Searches if command executable exists
void searchExecutable(char** args) {
  char* temp = strdup(args[0]); 
  int len = strlen(temp);
  char* path1 = strdup(initPath);
  char* path2 = strdup(usrPath);
  strncat(path1, temp, len);
  strncat(path2, temp, len);

  if (access(path1, X_OK) == -1) {
    if (access(path2, X_OK) == -1) {
      printf("pancake: command not found: %s\n", temp);
      return;
    }
  }
  executeArg(args);
}

// Search if args contains built-in command
void searchBuiltIn(char** args) {
  if (strcmp(args[0], "exit") == 0) {
    pancake_exit();
    return;
  }
  if (strcmp(args[0], "cd") == 0) {
    pancake_cd(args);
    return; 
  }

  searchExecutable(args);
}

int main(int argc, char *argv[]) {
  char *line;
  char **args;
  bool temp = true;

  do {
    printf("pancake> ");
    line = readLine();
    args = parseLine(line);
    searchBuiltIn(args);

  } while (temp);

  free(line);
  free(args);
  return 0;
}
