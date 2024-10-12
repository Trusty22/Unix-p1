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
  // found on stack overflow it solved my & problem of it messing up my command prompts
  // due to zombie forks being created.
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}
// copys the data of args into copyargs
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

// helpful print for users
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

  char *args[MAX_LINE / 2 + 1]; /* command line arguments */
  char *copyArgs[MAX_LINE / 2 + 1];
  int should_run = 1; /* flag to determine when to exit program */
  bool isfirstRun = true;
  bool hasPastCommand = false;
  bool firstBang = true;

  while (should_run) {

    printf("osh> ");
    fflush(stdout);

    char input[MAX_LINE / 2 + 1];
    *args[MAX_LINE / 2 + 1];

    fgets(input, MAX_LINE / 2 + 1, stdin);
    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, "exit") == 0) {
      exit(0);
    }

    // position pointer
    int pos = 0;
    if (strcmp(input, "") == 0) {
      pos = 0;
    } else {
      pos = 1;
    }

    // copy becomes args if we have past command and not first run
    if (hasPastCommand && !isfirstRun) {
      hasPastCommand = true;
      firstBang = false;
      *args = *copyArgs;
    }

    if (strcmp(input, "!!") == 0) {

      if (!isfirstRun) {
        isfirstRun = false;
        hasPastCommand = true;

        // loop used for size of copyArgs not being trigered when there is a small array
        int count;
        for (int i = 0; copyArgs[i] != NULL; i++) {
          count++;
        }

        // deals with the reaplcing of <> beacuse
        // args has its <> removed after !!. I have no idea how to bring it back except
        // hard coding it in
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
        // no history
        hasPastCommand = false;
        firstBang = true;
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
      firstBang = false;
    }
    // copy if the commands size is >=1 we dont have a past commnd and this isnt first run
    if (pos >= 1) {
      if (!hasPastCommand || !isfirstRun) {
        if (!firstBang) {
          copyArray(args, copyArgs);
        }
      }

      // check for &
      int hasAnd = 0;
      string s2;
      if (pos <= 1) {

      } else {
        s2 = args[pos - 1];
      }

      if (s2 == "&") {
        hasAnd = 1;
        args[pos - 1] = NULL;
      }
      isfirstRun = false;

      // fork
      int pid = fork();
      if (pid < 0) {

        perror("ERROR FORK DIDNT OPEN");
      } else if (pid == 0) { // child
        copyArray(args, copyArgs);
        for (int i = 0; i < pos; i++) {

          if (!hasAnd) {
            string s1(args[i]);
            // check type of direction
            if (s1 == ">" || s1 == "'>'") {
              // Output redirection
              FILE *file = fopen(args[i + 1], "w");
              if (file == NULL) {
                perror("fopen");
                exit(1);
              }
              dup2(fileno(file), STDOUT_FILENO);
              fclose(file);

              args[i] = NULL; // Remove output redirection from args
            } else if (s1 == "<" || s1 == "'<'") {
              // Input redirection
              FILE *file = fopen(args[i + 1], "r");
              if (file == NULL) {
                perror("fopen");
                exit(1);
              }
              dup2(fileno(file), STDIN_FILENO);
              fclose(file);
              args[i] = NULL; // Remove input redirection from args
            }
          }
        }

        /**
         *
         * I ran out of time for pipe however my logic would be:
         * 1. First find the location of pipe before fork
         * 2. Split my command into two char* arrays (so split the commands in args[] into two) before fork
         * 3. fork()
         * 4. create the pipe and then dup2 in the first child
         * 5. fork() in the child
         * 6. dup2 in the second child and execute accordingly
         *    (so like execvp in the if-else statements of the second fork)
         *
         */

        //  Execute the command
        execvp(args[0], args);
        cout << "No commands in history" << endl;
        exit(0);
      } else { // parent
        if (!hasAnd) {
          // wait
          wait(NULL);
        }
      }
    }
  }

  return 0;
}