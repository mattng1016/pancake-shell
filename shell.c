#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define deliminator " /"

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

int main(int argc, char *argv[]) {
  char *line;
  char **args;
  bool temp = true;

  do {
    printf("pancake> ");
    line = readLine();
    args = parseLine(line);
    executeArg(args);

  } while (temp);

  free(line);
  free(args);
  return 0;
}
