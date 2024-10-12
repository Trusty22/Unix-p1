#include <bits/stdc++.h>
#include <csignal>
#include <cstring>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80     /* The maximum length command */
#define HISTORY_SIZE 10 /* Number of commands to remember in history */

using namespace std;

int main(void) {
  char *args[MAX_LINE / 2 + 1]; /* command line arguments */
  char *copy[MAX_LINE / 2 + 1]; /* history of commands */
  int should_run = 1;           /* flag to determine when to exit program */

  while (should_run) {
    printf("osh> ");
    fflush(stdout);

    /* Read user input */
    char input[MAX_LINE];

    fgets(input, MAX_LINE / 2 + 1, stdin);
    input[strcspn(input, "\n")] = '\0';

    /* Check for exit command */
    if (strcmp(input, "exit") == 0) {
      exit(0);
    }

    /* Check for !! (repeat last command) */
    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
      args[i] = token;
      cout << args[i] << endl;
      i++;
      token = strtok(NULL, " ");
    }
    args[i] = NULL; /* Null-terminate the args array */

    if (i == 1 && strcmp(args[0], "!!") == 0) {

      // Re-tokenize the input
      i = 0;
      token = strtok(input, " ");
      while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
      }
      args[i] = NULL; /* Null-terminate the args array */
    } else {
      // Add to history
    }

    /* Check if the last argument is & (for background execution) */
    int background = 0;
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
      background = 1;
      args[i - 1] = NULL; /* Remove the & from the arguments */
    }

    /* Fork a child process */
    pid_t pid = fork();
    if (pid < 0) {
      /* Error occurred during fork */
      printf("Fork failed\n");
    } else if (pid == 0) {
      /* Child process: Handle input/output redirection */
      for (int j = 0; j < i; j++) {
        if (strcmp(args[j], ">") == 0) {
          // Output redirection
          FILE *output_file = fopen(args[j + 1], "w");
          if (output_file == NULL) {
            perror("fopen");
            exit(1);
          }
          dup2(fileno(output_file), STDOUT_FILENO);
          fclose(output_file);
          args[j] = NULL; // Remove output redirection from args
        } else if (strcmp(args[j], "<") == 0) {
          // Input redirection
          FILE *input_file = fopen(args[j + 1], "r");
          if (input_file == NULL) {
            perror("fopen");
            exit(1);
          }
          dup2(fileno(input_file), STDIN_FILENO);
          fclose(input_file);
          args[j] = NULL; // Remove input redirection from args
        }
      }

      // Execute the command
      if (execvp(args[0], args) == -1) {
        perror("Error executing command");
      }
      exit(0);
    } else {
      /* Parent process */
      if (!background) {
        /* Wait for the child to finish if not running in background */
        wait(NULL);
      }
    }
  }

  return 0;
}