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
    if (!file.is_open()) {
      std::cerr << "Error : not able to open " << options.output << std::endl;
      return;
    }

    // file << "Name \t";
    // file << "|V| \t";
    // file << "|E| \t";
    // file << "Lifetime \t";
    // file << "Max|Min Degree \t";
    // file << "#deg 1|2 \t";
    // file << "(un)stability \t";
    // file << "pair stability \t";
    // file << "pair stability v2 \t";
    // file << "c \t";
    // file << "delta1=0, delta2=0 \t";
    // file << "weak max(gamma,unstability) \t";
    // file << "delta1=0, delta2=5 \t";
    // file << "weak max(gamma,unstability) \t";
    // file << "delta1=0, delta2=10 \t";
    // file << "weak max(gamma,unstability) \t";
    // file << "delta1=5, delta2=10 \t";
    // file << "weak max(gamma,unstability) \t";
    // file << "\n";
    
    file.close();
    unsigned t=0;
    for (const auto& entry : fs::directory_iterator(options.folder)) {
      if(t==0){
	std::ofstream file(options.output, std::ios::app);
	t=25;
	file << "Name \t";
	file << "|V| \t";
	file << "|E| \t";
	file << "Lifetime \t";
	file << "Max|Min Degree \t";
	file << "#deg 1|2 \t";
	file << "(un)stability \t";
	file << "pair stability \t";
	file << "pair stability v2 \t";
	file << "c \t";
	file << "delta1=0, delta2=0 \t";
	file << "weak max(gamma,unstability) \t";
	file << "delta1=0, delta2=5 \t";
	file << "weak max(gamma,unstability) \t";
	file << "delta1=0, delta2=10 \t";
	file << "weak max(gamma,unstability) \t";
	file << "delta1=5, delta2=10 \t";
	file << "weak max(gamma,unstability) \t";
	file << "\n";
	file.close();
      }
      t--;
      std::cout << entry.path() << std::endl;
      std::cout << t << std::endl;
      Graph g;
      if(g.parse_dot_edge(entry.path()))
	g.write_stats(options.output, entry.path());
    }
}


int main(int argc, char *argv[])
{
  // Graph g;
  // g.parse_dot_edge("d/animal/insecta-ant-colony3-day11.edges");
  // exit(EXIT_SUCCESS);
  
  Options options(argc,argv);
  
  scanFolder(options);  
  return 0;
}
