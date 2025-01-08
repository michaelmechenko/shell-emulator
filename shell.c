#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 1024

char last_command[MAX_INPUT_LENGTH] = "";

// execute non-builtins
void execute_command(char **argv) {
  int fds[2];
  pid_t pid1, pid2;
  int status;
  int pipe_present = 0, redirect_out = 0;
  char *output_file = NULL;
  char **cmd1 = argv;
  char **cmd2;
  // check for redirection
  for (int i = 0; argv[i] != NULL; i++) {
    if (strcmp(argv[i], ">") == 0) {
      argv[i] = NULL;
      output_file = argv[i + 1];
      redirect_out = 1;
      break;
    }
  }

  // check for pipe present
  for (int i = 0; argv[i] != NULL; i++) {
    if (strcmp(argv[i], "|") == 0) {
      argv[i] = NULL;
      cmd2 = &argv[i + 1];
      pipe_present = 1;
      break;
    }
  }

  if (pipe_present) {
    if (pipe(fds) == -1) {
      perror("pipe failed");
      exit(1);
    }
    pid1 = fork();
    if (pid1 == 0) {
      close(fds[0]);
      dup2(fds[1], STDOUT_FILENO);
      execvp(cmd1[0], cmd1);
      perror("exec failed");
      exit(1);
    } else if (pid1 < 0) {
      perror("fork failed");
      exit(1);
    }
    pid2 = fork();
    if (pid2 == 0) {
      close(fds[1]);
      dup2(fds[0], STDIN_FILENO);
      if (redirect_out) {
        FILE *fp = fopen(output_file, "w");
        if (fp == NULL) {
          perror("Failed to open output file");
          exit(1);
        }
        dup2(fileno(fp), STDOUT_FILENO);
        fclose(fp);
      }
      execvp(cmd2[0], cmd2);
      perror("exec failed");
      exit(1);
    } else if (pid2 < 0) {
      perror("fork failed");
      exit(1);
    }
    close(fds[0]);
    close(fds[1]);
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
  } else {
    pid1 = fork();
    if (pid1 < 0) {
      perror("fork failed");
      exit(1);
    } else if (pid1 == 0) {
      execvp(cmd1[0], cmd1);
      perror("exec failed");
      exit(1);
    }
    waitpid(pid1, &status, 0);
  }
}

// execute builtin commands
void execute_builtin(char **argv) {
  // cd command
  if (strcmp(argv[0], "cd") == 0) {
    if (chdir(argv[1]) != 0) {
      perror("cd failed");
    }
  } else if (strcmp(argv[0], "source") == 0) {
    // source command
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
      perror("source failed");
      return;
    }
    char line[MAX_INPUT_LENGTH];
    while (fgets(line, sizeof(line), file)) {
      if (line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
      }
      TokenList tokens = tokenize(line);
      char *args[64];
      for (int i = 0; i < tokens.count; i++) {
        args[i] = tokens.tokens[i];
      }
      args[tokens.count] = NULL;
      execute_command(args);
    }
    fclose(file);
  } else if (strcmp(argv[0], "prev") == 0) {
    // prev command
    printf("%s\n", last_command);
    // duplicate last command without modifying original
    char *prev_command = strdup(last_command);
    char *command_part = strtok(prev_command, ";");
    while (command_part != NULL) {
      TokenList tokens = tokenize(command_part);
      char *args[64];
      for (int i = 0; i < tokens.count; i++) {
        args[i] = tokens.tokens[i];
      }
      args[tokens.count] = NULL;
      execute_command(args);
      command_part = strtok(NULL, ";");
    }
    free(prev_command);
  } else if (strcmp(argv[0], "help") == 0) {
    // help command
    printf("Built-in Commands:\n");
    printf("cd <directory>:  Change the current directory\n");
    printf("source <filename>: Execute a script given a filename\n");
    printf("prev: Print and execute the previous command again\n");
    printf("help: Explains the built-in commands available\n");
    printf("exit: Exit the program\n");
  }
}

int main(void) {
  char input[MAX_INPUT_LENGTH];
  char *argv[64];

  printf("Welcome to mini-shell.\n");

  while (1) {
    // prompt input
    printf("shell $ ");
    if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL) {
      printf("Bye bye.\n");
      break;
    }

    // remove \n
    if (input[strlen(input) - 1] == '\n') {
      input[strlen(input) - 1] = '\0';
    }

    // save last command
    if (strcmp(input, "prev") != 0) {
      strcpy(last_command, input);
    }

    // execute exit
    if (strcmp(input, "exit") == 0) {
      printf("Bye bye.\n");
      break;
    }

    // sequencing using ;
    char *command = strtok(input, ";");
    while (command != NULL) {
      TokenList tokens = tokenize(command);
      for (int i = 0; i < tokens.count; i++) {
        argv[i] = tokens.tokens[i];
      }
      argv[tokens.count] = NULL;
      // execute commands
      if (strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "source") == 0 ||
          strcmp(argv[0], "prev") == 0 || strcmp(argv[0], "help") == 0) {
        execute_builtin(argv);
      } else {
        execute_command(argv);
      }
      command = strtok(NULL, ";");
    }
  }

  return 0;
}
