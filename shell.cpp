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
#include <string>
#include <sys/wait.h>

#include <unistd.h>

#define MAX_LINE 80 /* The maximum length command */

using namespace std;
void printArrayEx(char *array[]) {

  cout << "--ARRAY PRINT--" << endl;
  int i;
  for (i = 0; array[i] != NULL; i++) {
    cout << array[i] << endl;
  }

  if (array[i] == NULL) {
    cout << "NULL" << endl;
  }
  cout << endl;
}

void printArray(char *array[]) {

  int i;
  for (i = 0; array[i] != NULL; i++) {
    cout << array[i] << " ";
  }

  if (array[i] == NULL) {
    cout << endl;
  } else {
    cout << "INVALID PRINT: EXITING";
    exit(0);
  }
}
void handle_sigchld(int sig) {
  // Reap any child process to prevent zombie processes
  // found on stack overflow it solved my & problem of it messing uo my command prompts
  // due to zombie forks being created.
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}
void write(char *args[], int pos) {

  // int file = open(args[pos + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644); // Open file with write permissions

  int file = open(args[pos + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
  dup2(file, STDOUT_FILENO);
  // if (file < 0) {
  //   perror("INVALID: FILE CANT OPEN");
  //   exit(0);
  // }
  // dup2("file", STDOUT_FILENO); // Redirect standard output to the file
  close(file);
  // args[pos] = NULL; // Terminate the command before the `>`
  return;
}

void copyArray(char *args[], char *copyArgs[]) {
  int i;
  for (i = 0; args[i] != NULL; i++) {
    copyArgs[i] = args[i];
  }
  copyArgs[i] = NULL;
}

void readUI(string in, char *args[], char *copyArgs[]) {

  int maxL = 0;
  int counter = 0;
  string temp = in;

  // calculate max amount words
  stringstream stream(temp);
  string oneWord;

  while (stream >> oneWord) {
    ++maxL;
  }

  // convert the word to char* to store in *args[]

  istringstream iss(in);
  string word;

  // while there is stream to go in string before char conversion and we haven't reached string length
  while (iss >> word && counter <= maxL) {

    args[counter] = new char[word.length() + 1]; // create new space for word as a token of a char && +1 for the null terminator
    strcpy(args[counter], word.c_str());         // put token in args and end with null
    counter++;
  }
  args[counter] = NULL;

  copyArray(args, copyArgs);

  // printArray(args);
  // printArray(copyArgs);
}

int main(void) {

  signal(SIGCHLD, handle_sigchld);

  char *args[MAX_LINE / 2 + 1]; /* command line arguments */
  int should_run = 1;           /* flag to determine when to exit program */
  int andPos;
  int pos;
  string input = "";
  bool isFirstRun = true;
  bool hasAnd = false;
  bool hasPastCommand = false;

  bool readFromFile = false;
  bool writeToFile = false;

  char *copyArgs[MAX_LINE / 2 + 1];

  char *commands[] = {(char *)"!!", (char *)"|", (char *)"&", (char *)"<", (char *)">"};
  while (should_run) {
    *args[MAX_LINE / 2 + 1];

    printf("osh>");
    fflush(stdout);

    getline(cin, input);

    if (input == "exit") {
      should_run = 0;
      exit(0);
      break;
    }

    if (input.find("!!") != string::npos) {

      if (isFirstRun) {
        hasPastCommand = false;
        cout << "INVALID: No commands in history" << endl;
      } else {
        isFirstRun = false;
        hasPastCommand = true;
        copyArray(args, copyArgs);

        printf("osh>");
        fflush(stdout);
        printArray(copyArgs);
      }

    } else {
      readUI(input, args, copyArgs);
      isFirstRun = false;
    }

    // methodize &

    for (int i = 0; args[i] != NULL; i++) {
      string s1(args[i]);
      string s2(commands[2]);
      string s3(commands[3]);
      string s4(commands[4]);
      if (s1 == s2) { // & means no parent waiting
        hasAnd = true;
        andPos = i;
        args[andPos] = NULL;
        break;
      }
      if (s1 == s3) {        // take command from file and run it in osh
        readFromFile = true; // execlp(args[0], args[2], args[3]);
        cout << " has <" << endl;

      } else if (s1 == s4) { // write left command to file
        writeToFile = true;
        pos = i;
        break;
        //        cout << " has >" << endl; // execlp(args[0], args[2], args[3]);
      }
    }

    //  first determin command that doesnt work like & < > !! and redirtct to its seperate function DONE

    /**
     *
     * Instructions
     * 1. Creating the child process and executing the command in the child DONE
     * 2. Providing a history feature DONE
     * 3. Adding support of input and output redirection Detection done, Implementaion needed execlp(args[0], args[2], args[3]); left command (<,>) right command
     * 4. Allowing the parent and child processes to communicate via a pipe
     *
     * After reading user input, the steps are: DONE
     * (1) fork a child process using fork() DONE
     * (2) the child process will invoke execvp() DOne
     * (3) parent will invoke wait() unless command included & Done
     */

    // start with fork (1)
    int rc = fork();

    if (rc == 0) { // child
      if (writeToFile) {
        write(args, pos); // writing
        execlp(args[0], args[1], args[2], args[3]);
        cout << " Unrecognized Command" << endl;
        exit(0); // kill it
        return 0;
      }

      execvp(args[0], args);
      cout << " Unrecognized Command" << endl;

      exit(0); // kill it
      return 0;

    } else if (rc > 0) { // parent & means no parent waiting
      if (hasAnd) {
        fflush(stdout);
        hasAnd = false; // Reset the flag
      } else {
        wait(NULL);
        hasAnd = false;
      }
    }

    if (rc < 0) {
      cerr << "Fork failed";
      exit(0);
      return 0;
    }
  }
  return 0;
}
