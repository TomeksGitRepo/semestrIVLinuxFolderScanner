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

//Include for file dirPath
#include <vector>
#include <string>
#include <iostream>
#include <regex>

//Added for logging
#include <syslog.h>

//Added for signals
#include <signal.h>
#include <curses.h> // Need to install apt-get install libncurses5-dev libncursesw5-dev

using namespace std;

#define BUFFERSIZE 1024
#define COPYMORE 0644

int copyFiles(const char*, const char*);
void checkFile(const char* , const char*);
void checkDirectory(const char* );

vector<string> sourcePathScannedFilesPath;


////////////////Signal handler ////////////////////////
static void sigint_handler(int signo) {
  printf("Caught SIGUSR1!\n");
  exit(EXIT_SUCCESS);
}

void scandirOneLevel(const char* sourceDir,const char* destinationDir, int depth = 0) {
  cout << "IN FUNCTION scandirOneLevel" << endl;
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
      string pathToFileToAdd(sourceFilePath);
      sourcePathScannedFilesPath.push_back(pathToFileToAdd);
      //TODO just for debugin to see add paths
      for (vector<string>::const_iterator iter = sourcePathScannedFilesPath.begin(); iter != sourcePathScannedFilesPath.end(); ++iter)
      cout << *iter << endl;


      char destinationFilePath [250];
      strcpy(destinationFilePath, destinationDir);
      strcat(destinationFilePath, "/");
      strcat(destinationFilePath, entry->d_name);
      //printf("Coping Filename: %*s%s\n", depth, "", entry->d_name);
      checkFile(sourceFilePath, destinationFilePath);


      openlog("slog", LOG_PID, LOG_USER);
      syslog(LOG_INFO, "File copied");
      closelog();
      //TODO call funciton to copy to destination if file was modified
      //scandir(entry->d_name, depth+4);
    } else if (S_ISDIR(statbuffer.st_mode)) {
      //If it is directory in scanned folder add to check array
      char sourceFilePath [250];
      strcpy(sourceFilePath, sourceDir);
      strcat(sourceFilePath, "/");
      strcat(sourceFilePath, entry->d_name);
      string pathToFileToAdd(sourceFilePath);
      sourcePathScannedFilesPath.push_back(pathToFileToAdd);
    }
    // } else if(S_ISDIR(statbuffer.st_mode)) {
    //   printf("DirName: %*s%s\n", depth, "", entry->d_name);
    // }
    //chdir("..");
    //closedir(dirPath);
  }
}


///////////////////////Function to scan folders recursively ///////////////////////

void scandirRecursevly(const char* sourceDir,const char* destinationDir, int depth = 0) {
  cout << "IN FUNCTION scandirRecursevly" << endl;
  DIR* dirPath;
  struct dirent* entry;
  struct stat statbuffer;

  dirPath = opendir(sourceDir); //Already checked in calling function
  chdir(sourceDir);
  while((entry = readdir(dirPath)) != NULL) {
    lstat(entry->d_name, &statbuffer);
    if(S_ISREG(statbuffer.st_mode)) {
      char sourceFilePath [250];
      strcpy(sourceFilePath, sourceDir);
      strcat(sourceFilePath, "/");
      strcat(sourceFilePath, entry->d_name);
      string pathToFileToAdd(sourceFilePath);
      sourcePathScannedFilesPath.push_back(pathToFileToAdd);
      cout << "File added to scan: " << pathToFileToAdd << endl;
      //TODO just for debugin to see add paths
      for (vector<string>::const_iterator iter = sourcePathScannedFilesPath.begin(); iter != sourcePathScannedFilesPath.end(); ++iter)
      cout << *iter << endl;


      char destinationFilePath [250];
      strcpy(destinationFilePath, destinationDir);
      strcat(destinationFilePath, "/");
      strcat(destinationFilePath, entry->d_name);

      printf("Coping Filename: %*s%s\n", depth, "", entry->d_name);
      checkFile(sourceFilePath, destinationFilePath);
      continue;
      //TODO call funciton to copy to destination if file was modified
      //scandir(entry->d_name, depth+4);
    } else if (S_ISDIR(statbuffer.st_mode)) {
      //If it is directory in scanned folder add to check array
      if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
        continue;
      string pathToNewDirectory(sourceDir);
      pathToNewDirectory.append("/");
      string fileName(entry->d_name);
      pathToNewDirectory.append(fileName);

      string pathToNewDestinationDirectory(destinationDir);
      pathToNewDestinationDirectory.append("/");
      pathToNewDestinationDirectory.append(entry->d_name);
      sourcePathScannedFilesPath.push_back(pathToNewDestinationDirectory);


      checkDirectory(pathToNewDestinationDirectory.c_str());
      scandirRecursevly(pathToNewDirectory.c_str(), pathToNewDestinationDirectory.c_str());
    }
  }
  chdir("..");
  closedir(dirPath);
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
    ///////////////////Report coping ///////////////////////////
    openlog("slog", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "File copied");
    closelog();
  } else if (sFileBuffer.st_mtime > dFileBuffer.st_mtime) {
    //Copy file if it was modified
    copyFiles(sourceFilePath, destinationFilePath);
    ///////////////////Report coping when source modification is newer///////////////////////////
    openlog("slog", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "File copied because source was modified");
    closelog();
  }
printf("Coping funciton finshed\n\n");
}


///////////////////////End of function to check end copy

////////////////////////Function to check and copy ///////////////////////
void checkDirectory(const char* destinationFilePath)
{
  struct stat dFileBuffer;

  if(-1 == stat(destinationFilePath, &dFileBuffer) ) {
    mkdir(destinationFilePath, 0777);
    ///////////////////Report making of new directory///////////////////////////
    openlog("slog", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "New directory created");
    closelog();
  }

printf("Making directory finished\n\n");
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
    ///////////////////Report error on opening source directory///////////////////////////
    openlog("slog", LOG_PID, LOG_USER);
    syslog(LOG_ERR, "Error opening source file");
    closelog();
    printf("Cannot open %s", source);
  }


  if( (out_fd=creat(destination, COPYMORE)) == -1 )
  {
    ///////////////////Report error///////////////////////////
    openlog("slog", LOG_PID, LOG_USER);
    syslog(LOG_ERR, "Error on creation of destination");
    closelog();
    printf("Cannot creat %s ", destination);
  }


  /* copy files */
  while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0 )
  {
    if( write(out_fd, buf, n_chars) != n_chars )
    {
      ///////////////////Report error ///////////////////////////
      openlog("slog", LOG_PID, LOG_USER);
      syslog(LOG_ERR, "Write error on destination");
      closelog();
      printf("Write error to %s", destination);
    }


    if( n_chars == -1 )
    {
      ///////////////////Report error ///////////////////////////
      openlog("slog", LOG_PID, LOG_USER);
      syslog(LOG_ERR, "Read error on source");
      closelog();
      printf("Read error from %s", source);
    }
  }


    /* close files */
    if( close(in_fd) == -1 || close(out_fd) == -1 )
    {
      ///////////////////Report error ///////////////////////////
      openlog("slog", LOG_PID, LOG_USER);
      syslog(LOG_ERR, "Read error on close of destintation file");
      closelog();
      printf("Error closing files");
    }


    return 1;
}

vector<string> changeFromSourceToDestinationPath (vector<string> vectorToChange, const char* sourcePath, const char* destinationPath) {
  vector<string> vectorToReturn;


  for (vector<string>::const_iterator iter = vectorToChange.begin(); iter != vectorToChange.end(); ++iter)
  {
    //TODO uncoment cout for debugging
    string sourcePathString(sourcePath);
    string tempString = *iter;
    //cout << "tempString=" << tempString << endl;
    //Check if path is not already changed to destination path
    string destinationPathString(destinationPath);

    //cout << "tempString=" << tempString << " and destinationPathString=" << destinationPathString << endl;

    bool found = strcmp(tempString.c_str(), destinationPathString.c_str()) != 0;
    if(found)
    {
      tempString = regex_replace(tempString, regex(sourcePathString), destinationPathString);
      //cout << "tempString after regex replacement:" << tempString << endl;
      //cout << "tempString after modification=" << tempString << endl;
      vectorToReturn.push_back(tempString);
    } else {
      string tempString = *iter;
      //cout << "tempString not changed modification=" << destinationPathString << endl;
      vectorToReturn.push_back(tempString);
    }

  }
  return vectorToReturn;
}


///////////////////////End function to copy files ////////////////////////

void checkIfDestinationFilesHaveSourceExisting(vector<string> sourceFilePaths, const char* sourceFilePath, const char* destinationFilePath) {
    string sourceFilePathString(sourceFilePath);
    string destinationFilePathString(destinationFilePath);
    //TODO debugg cout for debugging
    //cout << "sourceFilePathString=" << sourceFilePathString << endl;
    //cout << "destinationFilePathString=" << destinationFilePathString << endl;

    DIR* dirPath;
    struct dirent* entry;
    struct stat statbuffer;

    vector<string> destinationFilePaths = changeFromSourceToDestinationPath(sourceFilePaths, sourceFilePath, destinationFilePath );
    //cout <<  " Pure destinationFilePaths:" << endl;
    // for (vector<string>::const_iterator iter = destinationFilePaths.begin(); iter != destinationFilePaths.end(); ++iter)
    // {
    //   cout << *iter << endl;
    // }
    dirPath = opendir(destinationFilePath);


    while((entry = readdir(dirPath)) != NULL) {
      chdir(destinationFilePath);
      lstat(entry->d_name, &statbuffer);
      if(S_ISREG(statbuffer.st_mode)) {
        string dirPathString(destinationFilePath);
        string fullPathToDestinationFile = dirPathString + "/" + entry->d_name;
        // cout << "File to found in array fullPathToDestinationFile:"<< fullPathToDestinationFile << endl;
        // cout << "destinationFilePaths=======================" << endl;
        // for (vector<string>::const_iterator iter = destinationFilePaths.begin(); iter != destinationFilePaths.end(); ++iter)
        // {
        //   cout << *iter << endl;
        // }
        // cout << "destinationFilePaths=======================" << endl;

        int flagToLeave = 0;
        for (vector<string>::const_iterator iter = destinationFilePaths.begin(); iter != destinationFilePaths.end(); ++iter)
        {
          string tmp(*iter);

          // cout << "Compering: fullPathToDestinationFile:" << fullPathToDestinationFile << " tmp: " << tmp << endl;
          if(strcmp(fullPathToDestinationFile.c_str(), tmp.c_str()) == 0)
          {
            // cout << "Substring found in:" <<  destinationFilePath << endl;
            flagToLeave = 1;

          }
        }
        if(!flagToLeave)
        {
          // cout << "Removing file: " <<  fullPathToDestinationFile << endl;
           remove(fullPathToDestinationFile.c_str());
        }

      } else if (S_ISDIR(statbuffer.st_mode)) {
        if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
        {
          continue;
        }

        string dirPathString(destinationFilePath);
        string fullPathToDestinationDir = dirPathString + "/" + entry->d_name;
        //cout << "Dir to found in array fullPathToDestinationDir:"<< fullPathToDestinationDir << endl;
        // cout << "fullPathToDestinationDir DIRECTORY=" << fullPathToDestinationDir << endl;
        size_t found;
        int flagToLeaveDir = 0;
        for (vector<string>::const_iterator iter = destinationFilePaths.begin(); iter != destinationFilePaths.end(); ++iter)
        {
          string tmp(*iter);
          // cout << "Compering: fullPathToDestinationDir:" << fullPathToDestinationDir << " tmp: " << tmp << endl;
          if(strcmp(fullPathToDestinationDir.c_str(), tmp.c_str()) == 0)
          {
            //cout << "Substring found in:" <<  destinationFilePath << endl;
            flagToLeaveDir = 1;
          }
        }
        if(!flagToLeaveDir)
        {
          //cout << "Removing dir: " <<  fullPathToDestinationDir << endl;
           remove(fullPathToDestinationDir.c_str());
        }

        checkIfDestinationFilesHaveSourceExisting(destinationFilePaths, sourceFilePath, fullPathToDestinationDir.c_str());

        //checkIfDestinationFilesHaveSourceExisting(sourceFilePaths, sourceFilePath, fullPathToDestinationDir.c_str());

      }
    }
    closedir(dirPath);
}



int main(int argc, char ** argv) {

  /////////////////Register signal handler ////////////////////////////
  if(signal(SIGUSR1, sigint_handler) == SIG_ERR) {
    fprintf(stderr, "Cannot handle SIGUSR1!\n");
    exit(EXIT_FAILURE);
  }

  ////////////////Logging start of program ///////////////////////////
  openlog("slog", LOG_PID, LOG_USER);
  syslog(LOG_INFO, "Scanner program started");
  closelog();

  ///////////////////////////Start parsing arguments ////////////////////
  size_t i;
  const char* sourceFolder;
  const char* destinationFolder;
  int recursiveFlag = 0;

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
        recursiveFlag = 1;
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

  // scandirOneLevel(sourceFolder, destinationFolder); //TODO uncoment it works for one level of deepnes if this word exists :D

  // scandirRecursevly(sourceFolder, destinationFolder); //TODO uncoment for recursive function test

  // checkIfDestinationFilesHaveSourceExisting(sourcePathScannedFilesPath, sourceFolder, destinationFolder); //TODO uncoment to start folder syncronization function

  // //TODO just to test function to change vector
  //
  // //Push some valeues to vector
  // sourcePathScannedFilesPath.push_back("/home/tom/studiaIVSemestr/test/epic");
  // sourcePathScannedFilesPath.push_back("/home/tom/studiaIVSemestr/test/epic/fileToSave");
  // vector<string> returnedVector = changeFromSourceToDestinationPath(sourcePathScannedFilesPath, sourceFolder, destinationFolder);
  // cout << "Vector after changes:" << endl;
  // for (vector<string>::const_iterator iter = returnedVector.begin(); iter != returnedVector.end(); ++iter)
  // {
  //   cout << *iter << endl;
  // }
  // string destinationFolderString(destinationFolder);
  // string newDestinationFolder =  destinationFolderString + "/epic";
  // cout << "Before 2nd pass" << endl;
  // returnedVector = changeFromSourceToDestinationPath(returnedVector, sourceFolder, newDestinationFolder.c_str());
  // cout << "Vector after 2nd change:" << endl;
  // for (vector<string>::const_iterator iter = returnedVector.begin(); iter != returnedVector.end(); ++iter)
  // {
  //   cout << *iter << endl;
  // }


//  return 0;

  //////////////////////////END parsing arguments //////////////////////////

  //TODO uncoment for deamon to start
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


  //Close out the standard file descriptor
  // close(STDIN_FILENO);
  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);

  //initialization finished

  //Specific initialization goes here

  //The BIG LOOP

  while(1) {
    //Do some task here
    if (!recursiveFlag) {
      scandirOneLevel(sourceFolder, destinationFolder);
      checkIfDestinationFilesHaveSourceExisting(sourcePathScannedFilesPath, sourceFolder, destinationFolder);
    } else {
      scandirRecursevly(sourceFolder, destinationFolder);
      checkIfDestinationFilesHaveSourceExisting(sourcePathScannedFilesPath, sourceFolder, destinationFolder);
    }
    sleep(30);
  }
  exit(EXIT_SUCCESS);
}
