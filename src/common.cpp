
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

#include "common.hpp"

namespace UTIL{
  unsigned int DEBUG_LEVEL = 0;
  void stringFromFile(const std::string & fileName, std::string & s){
    size_t fileSize;
    struct stat st;
    if(stat(fileName.c_str(), &st)){
      std::cerr << "failed to stat file: " << fileName << std::endl;
      return;
    }
    std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!ifs){
      std::cerr << "failed to read file: " << fileName << std::endl;
      return;
    }
    fileSize = st.st_size;
    if(fileSize != static_cast<size_t>(static_cast<size_t>(fileSize))){
      std::cerr << "ERROR: The file size is too big to fit in int. Cannot process." << std::endl;
      return;
    }
    s.resize(fileSize);
    ifs.read(reinterpret_cast<char*>(&s[0]), fileSize);
  }


  void stringFromFile(const std::string & fileName, std::vector<unsigned char> & s){
    size_t fileSize;
    struct stat st;
    if(stat(fileName.c_str(), &st)){
      std::cerr << "failed to stat file: " << fileName << std::endl;
      return;
    }
    std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!ifs){
      std::cerr << "failed to read file: " << fileName << std::endl;
      return;
    }
    fileSize = st.st_size;
    if(fileSize != static_cast<size_t>(static_cast<int>(fileSize))){
      std::cerr << "ERROR: The file size is too big to fit in int. Cannot process." << std::endl;
      return;
    }
    s.resize(fileSize);
    ifs.read(reinterpret_cast<char*>(&s[0]), fileSize);
  }


  double gettime(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((double)tv.tv_sec)
            + ((double)tv.tv_usec)*1e-6);
  }

  std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }
  std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
  }

  void runCmd(std::string cmd) {
    std::vector<std::string> argv = split(cmd, ' ');
    char *ex_argv[argv.size() + 1];
    ex_argv[argv.size()] = NULL;
    char *ex_env[] = {NULL};
    for (unsigned int i = 0; i < argv.size(); i++){
      ex_argv[i] = (char *) argv[i].c_str();
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      exit(-1);
    } else if (pid == 0) {
      // run child process
      execve(ex_argv[0], ex_argv, ex_env);
      exit(-1);
    }

    int status;
    pid_t r = waitpid(pid, &status, 0); // waites until the child process terminates
    if (r < 0){
      perror("waitpid");
      exit(-1);
    }
    if (WIFEXITED(status)) {
    } else {
      printf("child status=%04x\n", status);
    }
  }
}
