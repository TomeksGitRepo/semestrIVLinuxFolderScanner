#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <fstream>

int main(void) {
  //our process ID and Session ID
  pid_t pid, sid;

  //Fork off the parent process
  pid = fork();
  if(pid < 0) {
    exit(EXIT_FAILURE);
  }
  //if we got a good PID, then we can exit the parent process
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  //Change the file mode mask
  umask(0);

  //Open any logs here

  //Create a new SID for the child process
  sid = setsid();
  if (sid < 0) {
      //log any failure
      exit(EXIT_FAILURE);
  }

  //Change the current working directory
  // if ((chdir("/")) < 0) {
  //   //Log any failure here
  //   exit(EXIT_FAILURE);
  // }


  //Close out the standard file descriptor
  // close(STDIN_FILENO);
  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);

  //initialization finished

  //Specific initialization goes here

  //The BIG LOOP

  while(1) {
    //Do some task here
    std::ofstream myfile;
    myfile.open ("example.txt", std::ofstream::app);
    myfile << "Im working.\n";
    myfile.close();
    sleep(30);
  }
  exit(EXIT_SUCCESS);
}
