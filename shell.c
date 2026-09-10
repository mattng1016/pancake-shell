#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define deliminator " "

int numShellPath = 2;
static char** shellPath;
static int numArgs;
const char* builtInFunction[] = {"pancake_exit", "[pancake_cd", "pancake_path"};

// Exit command 
void pancake_exit() {
  exit(0);
}

// Change directory command
void pancake_cd(char** args) {
  if (numArgs > 2) {
    printf("Too many arguments\n");
    return;
  }  
  if (chdir(args[1]) == -1) {
    printf("Path not found\n");
  }
  return;
}

// Path command
void pancake_path(char** args) {
  if (numArgs == 1) {
    memset(shellPath, 0, sizeof(*shellPath)*numShellPath);
    return;
  } else if (numArgs > 1) {
    // Realloc if it needs more space
    if (numArgs - 1 > numShellPath) {
      shellPath = realloc(shellPath, sizeof(char*)*numArgs-1); 
      numShellPath = numArgs - 1;
    }
    // Set shellPath to given paths
    for (int i = 0; i < numArgs - 1; i++) {
      shellPath[i] = args[i+1];
    }
    // Set remaining paths to NULL
    memset(&shellPath[numArgs-1], 0, sizeof(char*)*(numShellPath-(numArgs-1)));
  }
  return;
}

void pancake_debug(char** args) {
  for (int i = 0; i < numShellPath; i++) {
    printf("%s\n", shellPath[i]);
  }
  printf("size of numPath: %d\n", numShellPath);
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
    numArgs++;
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

char* concatenatePath(const char* s1, const char* s2) {
  char* result = malloc(strlen(s1) + strlen(s2) + 2);
  if (result == NULL) {
    printf("Malloc error");
    exit(1);
  }
  strcpy(result, s1);
  strcat(result, "/");
  strcat(result, s2);

  return result;
}

// Searches if command executable exists
void searchExecutable(char** args) {
  bool found = false;

  for (int i = 0; i < numShellPath; i++) {
    if (shellPath[i] != NULL) {
      char* path = concatenatePath(shellPath[i], args[0]);
      //printf("path: %s\n", path);
      if (access(path, X_OK) == 0) {
        found = true;
      }
      free(path);
    }
  }

  if (found) {
    executeArg(args);
    return;
  } else {
    printf("pancake: command not found: %s\n", args[0]);
  }
}

void pancake_redirection(char** args, int pos) {
  // Input validation
  if (pos != (numArgs - 2)) {
    printf("Too many arguments: Usage: %s > <dir>\n", args[0]);
    return;
  }
  char** realArgs = malloc(sizeof(char*)*(numArgs - 2));
  for (int i = 0; i < pos; i++) {
    realArgs[i] = args[i];
    //printf("%d: %s\n", i, realArgs[i]);
  }
  char* fileName = args[pos+1];
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int output = dup(1);
  int fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, mode);
  if (fd == -1) {
    printf("%s: Permission error", fileName);
    return;
  } 
  fflush(stdout);
  if (dup2(fd, STDOUT_FILENO) == -1) {
    printf("File redirection error");
    return;
  }
  searchExecutable(realArgs);
  free(realArgs);
  close(fd);
  dup2(output, 1);
}

// Search if args contains built-in command
void searchBuiltIn(char** args) {
  for (int i = 0; i < numArgs; i++) {
    if (strcmp(args[i], ">") == 0) {
      pancake_redirection(args, i);
      return;
    }
  }
  if (strcmp(args[0], "exit") == 0) {
    pancake_exit();
    return;
  }
  if (strcmp(args[0], "cd") == 0) {
    pancake_cd(args);
    return; 
  }
  if (strcmp(args[0], "path") == 0) {
    pancake_path(args);
    return;
  }
  if (strcmp(args[0], "debug") == 0) {
    pancake_debug(args);
    return;
  }
  searchExecutable(args);
   
  return;
}

void searchSpecial(char** args) {
  for (int i = 0; i < numArgs; i++) {
    if (strcmp(args[i], ">") == 0) {
      pancake_redirection(args, i);
      return;
    }
  }
  searchBuiltIn(args);
}

int main(int argc, char *argv[]) {
  char *line;
  char **args;
  bool temp = true;
  shellPath = malloc(sizeof(char*)*numShellPath);
  shellPath[0] = "/bin";
  shellPath[1] = "/usr/bin";

  do {
    printf("pancake> ");
    numArgs = 0;
    line = readLine();
    args = parseLine(line);
    searchSpecial(args);

  } while (temp);

  free(line);
  free(args);
  free(shellPath);
  return 0;
}
