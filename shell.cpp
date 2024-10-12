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
    copyArgs[i] = (char *)malloc(strlen(args[i]) + 1);
    if (copyArgs[i] != NULL) {
      strcpy(copyArgs[i], args[i]);
    }
  }
  copyArgs[i] = NULL;
}

void print(char *args[]) {
  int i;
  // for (i = 0; args[i] != NULL; i++) {
  //   cout << args[i] << " ";
  // }

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
    *args[MAX_LINE / 2 + 1];

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

    if (hasPastCommand && !isfirstRun) {
      hasPastCommand = true;
      *args = *copyArgs;
    }

    if (strcmp(input, "!!") == 0) {

      string s1(args[0]);

      if (!isfirstRun) {
        isfirstRun = false;
        hasPastCommand = true;

        int count;
        for (int i = 0; copyArgs[i] != NULL; i++) {
          count++;
        }

        if (count > 1) {
          char dirL[1] = {'<'};
          char dirR[1] = {'>'};

          dirL[strcspn(dirL, "\n")] = '\0';
          dirR[strcspn(dirL, "\n")] = '\0';

          string s3(copyArgs[1]);

          if (s3 == ">") {
            copyArgs[1] = strtok(dirR, "");
            args[1] = copyArgs[1];
          } else if (s3 == "<") {
            copyArgs[1] = strtok(dirL, "");
            args[1] = copyArgs[1];
          }
        }
        *args = *copyArgs;

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

    if (pos >= 1) {
      if (!hasPastCommand || !isfirstRun) {
        copyArray(args, copyArgs);
      }

      /* Check if the last argument is & (for hasAnd execution) */
      int hasAnd = 0;
      string s2(args[pos - 1]);
      if (s2 == "&") {
        hasAnd = 1;
        args[pos - 1] = NULL; /* the & from the arguments */
      }
      isfirstRun = false;

      /* Fork a child process */
      int pid = fork();
      if (pid < 0) {
        /* Error occurred during fork */
        perror("ERROR FORK DIDNT OPEN");
      } else if (pid == 0) {
        /* Child process: Handle input/output redirection */
        copyArray(args, copyArgs);
        for (int i = 0; i < pos; i++) {
          string s1(args[i]);

          if (s1 == ">" || s1 == "'>'") {
            // Output redirection
            FILE *output_file = fopen(args[i + 1], "w");
            if (output_file == NULL) {
              perror("fopen");
              exit(1);
            }
            dup2(fileno(output_file), STDOUT_FILENO);
            fclose(output_file);

            args[i] = NULL; // Remove output redirection from args
          } else if (s1 == "<" || s1 == "'<'") {
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