#pragma once

#include <string>
#include <vector>

namespace UTIL{
  extern unsigned int DEBUG_LEVEL;
  void stringFromFile(const std::string & fileName, std::string & s);
  void stringFromFile(const std::string & fileName, std::vector<unsigned char> & s);
  std::vector<std::string> split(const std::string &s, char delim) ;
  void runCmd(std::string cmd);


  double gettime();
}

