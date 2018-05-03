#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <fstream>
//Directory infos
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>

int main(int argc, char ** argv) {
  ///////////////////////////Start parsing arguments ////////////////////
  size_t i;
  const char* sourceFolder;
  const char* destinationFolder;

  //Geting files infos
  struct stat sb;

  if(argc < 3)
  {
    printf("Incorect usage of program\n");
    printf("Pass source directory and destination directory as arguments to program\n");
    printf("Use -R flag to scan folder recursively\n");
      exit(EXIT_FAILURE);
  }

  char rFlag[] = "-R";
  for(i = 0; i < argc; i++) {
    char const *option = argv[i];
    if(i == 1) {
      sourceFolder = option;
      printf("Source folder: %s\n", option);
    } else if(i == 2) {
      destinationFolder = option;
      printf("Destination folder: %s\n", option);
    } else if( i == 3) {

      if(!strcmp(option, rFlag)) {
        printf("Additional flag activated: %s\n", option);
      }

    }
  }

//Check if folders exits
  if(-1 == stat(sourceFolder, &sb) || !S_ISDIR(sb.st_mode)) {
    printf("sourceFolder error path is not existent or pointing to file.\nChange to directory path\n");
    exit(EXIT_FAILURE);
  } else if(-1 == stat(destinationFolder, &sb) || !S_ISDIR(sb.st_mode)) {
    printf("destinationFolder error path is not existent or pointing to file.\nChange to directory path\n");
    exit(EXIT_FAILURE);
  }

  return 0;

  //////////////////////////END parsing arguments //////////////////////////

  //TODO uncoment for deamon to start
  // //our process ID and Session ID
  // pid_t pid, sid;
  //
  // //Fork off the parent process
  // pid = fork();
  // if(pid < 0) {
  //   exit(EXIT_FAILURE);
  // }
  // //if we got a good PID, then we can exit the parent process
  // if (pid > 0) {
  //   exit(EXIT_SUCCESS);
  // }
  //
  // //Change the file mode mask
  // umask(0);
  //
  // //Open any logs here
  //
  // //Create a new SID for the child process
  // sid = setsid();
  // if (sid < 0) {
  //     //log any failure
  //     exit(EXIT_FAILURE);
  // }
  //
  // //Change the current working directory
  // // if ((chdir("/")) < 0) {
  // //   //Log any failure here
  // //   exit(EXIT_FAILURE);
  // // }
  //
  //
  // //Close out the standard file descriptor
  // // close(STDIN_FILENO);
  // // close(STDOUT_FILENO);
  // // close(STDERR_FILENO);
  //
  // //initialization finished
  //
  // //Specific initialization goes here
  //
  // //The BIG LOOP
  //
  // while(1) {
  //   //Do some task here
  //   std::ofstream myfile;
  //   myfile.open ("example.txt", std::ofstream::app);
  //   myfile << "Im working.\n";
  //   myfile.close();
  //   sleep(30);
  // }
  // exit(EXIT_SUCCESS);
}
