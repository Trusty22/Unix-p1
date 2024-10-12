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

void handle_sigchld(int sig) {
  // Reap any child process to prevent zombie processes
  // found on stack overflow it solved my & problem of it messing uo my command prompts
  // due to zombie forks being created.
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

void copyArray(char *args[], char *copyArgs[]) {
  int i;
  for (i = 0; args[i] != NULL; i++) {
    copyArgs[i] = args[i];
  }
  copyArgs[i] = NULL;
}

void print(char *args[]) {
  int i;
  for (i = 0; args[i] != NULL; i++) {
    cout << args[i] << " ";
  }
  if (args[i] == NULL) {
    cout << "NULL" << endl;
  }
}

int main(void) {
  signal(SIGCHLD, handle_sigchld);

  char *args[MAX_LINE / 2 + 1];     /* command line arguments */
  char *copyArgs[MAX_LINE / 2 + 1]; /* history of commands */
  int should_run = 1;               /* flag to determine when to exit program */
  bool isfirstRun = true;
  bool hasPastCommand = false;

  while (should_run) {

    printf("osh> ");
    fflush(stdout);

    char input[MAX_LINE / 2 + 1];

    fgets(input, MAX_LINE / 2 + 1, stdin);
    input[strcspn(input, "\n")] = '\0';

    /* Check for exit command */
    if (strcmp(input, "exit") == 0) {
      exit(0);
    }
    int pos = 0;
    if (strcmp(input, "") == 0) {
      pos = 0;
    } else {
      pos = 1;
    }
    if (isfirstRun) {
      copyArray(args, copyArgs);
    }
    if (strcmp(input, "!!") == 0) {
      copyArray(args, copyArgs);

      string s1(args[0]);

      if (!isfirstRun) {
        isfirstRun = false;
        hasPastCommand = true;
        copyArray(args, copyArgs);

        printf("osh> ");
        fflush(stdout);
        print(args);

      } else {
        hasPastCommand = false;
        cout << "No commands in history" << endl;
      }
    } else {
      // tokenize
      pos = 0;
      char *token = strtok(input, " ");
      while (token != NULL) {
        args[pos] = token;
        pos++;
        token = strtok(NULL, " ");
      }
      args[pos] = NULL;
    }

    // for history
    if (pos >= 1) {

      /* Check if the last argument is & (for hasAnd execution) */
      int hasAnd = 0;
      string s2(args[pos - 1]);
      if (s2 == "&") {
        hasAnd = 1;
        args[pos - 1] = NULL; /* the & from the arguments */
      }
      isfirstRun = false;
      cout << pos << endl;
      /* Fork a child process */
      int pid = fork();
      if (pid < 0) {
        /* Error occurred during fork */
        perror("ERROR FORK DIDNT OPEN");
      } else if (pid == 0) {
        /* Child process: Handle input/output redirection */
        for (int i = 0; i < pos; i++) {

          if (strcmp(args[i], ">") == 0) {
            // Output redirection
            FILE *output_file = fopen(args[i + 1], "w");
            if (output_file == NULL) {
              perror("fopen");
              exit(1);
            }
            dup2(fileno(output_file), STDOUT_FILENO);
            fclose(output_file);
            args[i] = NULL; // Remove output redirection from args
          } else if (strcmp(args[i], "<") == 0) {
            // Input redirection
            FILE *input_file = fopen(args[i + 1], "r");
            if (input_file == NULL) {
              perror("fopen");
              exit(1);
            }
            dup2(fileno(input_file), STDIN_FILENO);
            fclose(input_file);
            args[i] = NULL; // Remove input redirection from args
          }
        }

        //  Execute the command
        print(args);
        execvp(args[0], args);
        exit(0);
      } else {
        /* Parent process */
        if (!hasAnd) {
          /* Wait for the child to finish if not running in background */
          wait(NULL);
        }
      }
    }
  }

  return 0;
}