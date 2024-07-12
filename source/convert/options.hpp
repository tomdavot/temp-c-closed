#include <climits>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <getopt.h>



struct  Options {

  void display_help() const {
    std::cout << "Available options:\n";
    std::cout << "--file         input file\n";
    std::cout << "--folder       input folder\n";
    std::cout << "--out          output file or folder\n\n";
    std::cout << "--format       describe format for a line\n";
    std::cout << "--formula      describe formula to compute timesteps\n";
    
    std::cout << "+----------------------------------------------------+\n";
    std::cout << "|                   FORMAT                           |\n";
    std::cout << "+----------------------------------------------------+\n";
    std::cout << "|    - [u,v]:       indicate the presence of an      |\n";
    std::cout << "|                   alphanum string that will be     |\n";
    std::cout << "|                   captured for vertices u and v    |\n";
    std::cout << "|                   The format must contains both    |\n";
    std::cout << "|                   u and v in this order            |\n";
    std::cout << "|                   (ie ..v....u.. won't work)       |\n";
    std::cout << "|    - integer i:   capture an integer for the value |\n";
    std::cout << "|                   of the variable #i in the formula|\n";
    std::cout << "|    - punctuation,                                  |\n";
    std::cout << "|      space or,                                     |\n";
    std::cout << "|      tabulation:  indicate the type of separator   |\n";
    std::cout << "|                   between two elements             |\n";
    std::cout << "|    - [b]:         indicate the presence of an      |\n";
    std::cout << "|                   alphanum string that will be     |\n";
    std::cout << "|                   ignored                          |\n";
    std::cout << "+----------------------------------------------------+\n";
    std::cout << std::endl;
    std::cout << "+----------------------------------------------------+\n";
    std::cout << "|                   FORMULA                          |\n";
    std::cout << "+----------------------------------------------------+\n";
    std::cout << "|    - [+,-,*,/]:   operator                         |\n";
    std::cout << "|    - integer:     constant of the formula          |\n";
    std::cout << "|    - #integer:    will be substitued by the value  |\n";
    std::cout << "|                   of the variable                  |\n";
    std::cout << "|    - m(expr):     convert a month into a number    |\n";
    std::cout << "|                   days                             |\n";
    std::cout << "+----------------------------------------------------+\n";
    std::cout << std::endl;
    exit(EXIT_SUCCESS);
  }

  
  std::string input, output,in_folder,format,formula;
  
  int opt;

  Options(int argc, char * argv[]) : input("-1"), output("-1"), in_folder("-1"), format("-1"), formula("#1")
  {
    option long_arg[] = {
      {"file", required_argument, nullptr, 'i'},
      {"folder", required_argument,nullptr, 'f'},
      {"out", required_argument,nullptr, 'o'},	
      {"format", required_argument,nullptr, 'l'},	
      {"formula", required_argument,nullptr, 'm'},	
    };

    std::string tmp;
    if(argc==1){
      std::cout << "No argument provided\n";
      display_help();
    }
    while((opt = getopt_long(argc, argv, "i:f:o:l:m:h",long_arg,nullptr)) !=-1) {
      switch (opt)
	{
	case 'i':
	  input=optarg;
	  break;
	case 'f':
	  in_folder=optarg;
	  break;
	case 'o':
	  output=optarg;
	  break;
	case 'l':
	  format=optarg;
	  break;
	case 'm':
	  formula=optarg;
	  break;
	case 'h':
	default:
	  display_help();
	  break;
	}
    }
    if(input=="-1" && in_folder=="-1"){
      std::cout << "Error, no input file or folder provided\n";
      exit(EXIT_SUCCESS);
    }
    if(output=="-1"){
      std::cout << "Error, no output file or folder provided\n";
      exit(EXIT_SUCCESS);
    }
    if(format=="-1"){
      std::cout << "Error, no format string provided\n";
      exit(EXIT_SUCCESS);
    }


      
  }
};
