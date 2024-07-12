#include <boost/container/flat_set.hpp>
#include <boost/xpressive/regex_constants.hpp>

#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
 
#include "Graph.hpp"
#include "options.hpp"

namespace fs = std::filesystem;


void scanFolder(const Options & options) {
    std::ofstream file(options.output, std::ios::trunc);
    file << "Name \t";
    file << "|V| \t";
    file << "|E| \t";
    file << "Lifetime \t";
    file << "MaxDegree \t";
    file << "(un)stability \t";
    file << "pair stability \t";
    file << "pair stability v2 \t";
    file << "c \t";
    file << "delta1=1, delta2=0 \t";
    file << "weakly \t";
    file << "delta1=1, delta2=5 \t";
    file << "weakly \t";
    file << "delta1=1, delta2=10 \t";
    file << "weakly \t";
    file << "delta1=5, delta2=10 \t";
    file << "weakly \t";
    file << "\n";
    
    if (!file.is_open()) {
      std::cerr << "Error : not able to open " << options.output << std::endl;
      return;
    }
    for (const auto& entry : fs::directory_iterator(options.folder)) {
      std::cout << entry.path() << std::endl;
      Graph g(entry.path());
      g.write_stats(file, entry.path());
    }
    file.close();
}


int main(int argc, char *argv[])
{
  Options options(argc,argv);
  
  scanFolder(options);  
  return 0;
}
