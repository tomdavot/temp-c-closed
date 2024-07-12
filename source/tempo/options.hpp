#include <climits>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <getopt.h>



struct  Options {

  void display_help() const {
    std::cout << "Available options:\n";
    std::cout << "--in       input file\n";
    std::cout << "--out      output file\n";
    std::cout << std::endl;
    exit(EXIT_SUCCESS);
  }

  
  std::string input, output;
  std::string folder;
  int opt;

  Options(int argc, char * argv[]) : input("-1"), output("-1")
  {
    option long_arg[] = {
      {"in", required_argument, nullptr, 'i'},
      {"out", required_argument,nullptr, 'o'},
      {"fold", required_argument,nullptr, 'f'},	
    };

    std::string tmp;
    if(argc==1){
      std::cout << "No argument provided\n";
      display_help();
    }
    while((opt = getopt_long(argc, argv, "i:o:f:h",long_arg,nullptr)) !=-1) {
      switch (opt)
	{
	case 'i':
	  input=optarg;
	  break;
	case 'o':
	  output=optarg;
	  break;
	case 'f':
	  folder=optarg;
	  break;
	case 'h':
	default:
	  display_help();
	  break;
	}
    }      
  }
};
