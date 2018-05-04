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
#include <string.h>

#define BUFFERSIZE 1024
#define COPYMORE 0644

int copyFiles(const char*, const char*);
void checkFile(const char* sourceFilePath, const char* destinationFilePath);


void scandirOneLevel(const char* sourceDir,const char* destinationDir, int depth = 0) {
  DIR* dirPath;
  struct dirent* entry;
  struct stat statbuffer;

  dirPath = opendir(sourceDir); //Already checked in calling function
  chdir(sourceDir);
  while((entry = readdir(dirPath)) != NULL) {
    lstat(entry->d_name, &statbuffer);
    if(S_ISREG(statbuffer.st_mode)) {
      // if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
      //   continue;
      char sourceFilePath [250];
      strcpy(sourceFilePath, sourceDir);
      strcat(sourceFilePath, "/");
      strcat(sourceFilePath, entry->d_name);

      char destinationFilePath [250];
      strcpy(destinationFilePath, destinationDir);
      strcat(destinationFilePath, "/");
      strcat(destinationFilePath, entry->d_name);

      printf("Coping Filename: %*s%s\n", depth, "", entry->d_name);
      checkFile(sourceFilePath, destinationFilePath);
      //TODO call funciton to copy to destination if file was modified
      //scandir(entry->d_name, depth+4);
    }
    // } else if(S_ISDIR(statbuffer.st_mode)) {
    //   printf("DirName: %*s%s\n", depth, "", entry->d_name);
    // }
    //chdir("..");
    //closedir(dirPath);
  }
}

////////////////////////Function to check and copy ///////////////////////
void checkFile(const char* sourceFilePath, const char* destinationFilePath)
{
  struct stat sFileBuffer;
  struct stat dFileBuffer;

  if(-1 == stat(sourceFilePath, &sFileBuffer)) {
    perror("sourceFilePath error path is not existent or pointing to missing file.\nRestart program\n");
    exit(EXIT_FAILURE);
  }
  if(-1 == stat(destinationFilePath, &dFileBuffer) ) {
    printf("No file in dest starting coping\n");
    copyFiles(sourceFilePath, destinationFilePath);
  } else if (sFileBuffer.st_mtime > dFileBuffer.st_mtime) {
    //Copy file if it was modified
    copyFiles(sourceFilePath, destinationFilePath);
  }//TODO add check for modification time and if source > destination copy files
printf("Coping funciton finshed\n\n");
}


///////////////////////End of function to check end copy


///////////////////Function to copy files /////////////////////////////
int copyFiles(const char *source, const char *destination)
{
  int in_fd, out_fd, n_chars;
  char buf[BUFFERSIZE];


  /* open files */
  if( (in_fd=open(source, O_RDONLY)) == -1 )
  {
    printf("Cannot open %s", source);
  }


  if( (out_fd=creat(destination, COPYMORE)) == -1 )
  {
    printf("Cannot creat %s ", destination);
  }


  /* copy files */
  while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0 )
  {
    if( write(out_fd, buf, n_chars) != n_chars )
    {
      printf("Write error to %s", destination);
    }


    if( n_chars == -1 )
    {
      printf("Read error from %s", source);
    }
  }


    /* close files */
    if( close(in_fd) == -1 || close(out_fd) == -1 )
    {
      printf("Error closing files");
    }


    return 1;
}



///////////////////////End function to copy files ////////////////////////


int main(int argc, char ** argv) {
  ///////////////////////////Start parsing arguments ////////////////////
  size_t i;
  const char* sourceFolder;
  const char* destinationFolder;

  //Geting files infos
  struct stat sFolderBuffer;
  struct stat dFolderBuffer;


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

//Check if args folders exits
  if(-1 == stat(sourceFolder, &sFolderBuffer) || !S_ISDIR(sFolderBuffer.st_mode)) {
    printf("sourceFolder error path is not existent or pointing to file.\nChange to directory path\n");
    exit(EXIT_FAILURE);
  } else if(-1 == stat(destinationFolder, &dFolderBuffer) || !S_ISDIR(dFolderBuffer.st_mode)) {
    printf("destinationFolder error path is not existent or pointing to file.\nChange to directory path\n");
    exit(EXIT_FAILURE);
  }

  scandirOneLevel(sourceFolder, destinationFolder);

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
