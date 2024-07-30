
#include "converter.cpp"
#include "options.hpp"
#include <filesystem>

void test(Options &options){
  
  Converter c;


  
  std::string format= "b	u	v	1/2";
  std::string formula = "#1+m(#2)";


  c.set_format(format);
  c.set_formula(formula);


  std::vector<std::string> lines = {
    "t	i	j	DateTime",
    "1560396500	ARIELLE	FANA	13/06/2019 05:28",
    "1560396500	ARIELLE	VIOLETTE	13/06/2019 05:28",
    "1560396520	FANA	HARLEM	13/06/2019 05:28",
    "1560396540	FELIPE	ANGELE	13/06/2019 05:29 ",
    "1560396540	ARIELLE	FANA	13/06/2019 05:29",
    "1560396580	BOBO	FELIPE	13/06/2019 05:29",
    "1560396620	EWINE	PIPO	13/06/2019 05:30",
    "1560396640	FANA	HARLEM	13/06/2019 05:30",
    "1560396660	BOBO	FEYA	13/06/2019 05:31"
  };
  

  // c.convert_file(file);
  // c.write_in_file("convert.edges");
  
  for(auto l : lines)
    c.parse_line(l,1);

  c.display();
  exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{

  
  Converter c;
  Options options(argc,argv);

  // std::cout << options.format << std::endl;
  // std::cout << options.formula << std::endl;
  
  //  test(options);
  

  
  
  if(options.input!="-1"){
    c.set_format(options.format);
    c.set_formula(options.formula);

    c.convert_file(options.input);
    c.write_in_file(options.output);
  }
  else {
    std::filesystem::path dirPath(options.in_folder);
    std::filesystem::path format_file = dirPath / "format.format";

    std::cout << format_file << std::endl;
    std::cout << std::filesystem::exists(format_file) << std::endl;

    std::ifstream file(format_file);
    if(!file) {
      std::cout << "Error: could not open " << format_file << std::endl;
      exit(EXIT_FAILURE);
    }
    std::string format="a",formula="-1";
    

    std::string line;
    while (std::getline(file, line)) {
      size_t equalPos = line.find('=');
      size_t quoteStart = line.find('"', equalPos);
      size_t quoteEnd = line.find('"', quoteStart + 1);

      if (equalPos != std::string::npos && quoteStart != std::string::npos && quoteEnd != std::string::npos) {
	std::string option = line.substr(0, equalPos);
	std::string value = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

       
	option.erase(option.find_last_not_of(" \t") + 1);
	option.erase(0, option.find_first_not_of(" \t"));

	std::cout << option << " => " << value << std::endl;
	if(option=="format"){
	  format=value;
	}
	else if(option=="formula"){
	  formula=value;
	}
	else {
	  std::cout << "Format file: option " << option << " not recognized\n";
	}
      }
    }
    if(format=="a" || formula=="-1"){
      std::cout << "Error formula or format not set in format file\n";
      exit(EXIT_SUCCESS);
    }
    c.set_format(format);
    c.set_formula(formula);

    if (!std::filesystem::exists(options.output)) {
      if (!std::filesystem::create_directory(options.output)){
	std::cout << "Unable to create " << options.output << std::endl;
	exit(EXIT_SUCCESS);
      }
    }
      
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
      if (entry.is_regular_file()) { 
	std::filesystem::path f = entry.path();
	if (f.filename() != "format.format") {
	  std::cout << "Convert: " << f.filename() << std::endl;

	  std::filesystem::path newFilePath = options.output / f.filename();
	  newFilePath.replace_extension(".edges");

	  c.convert_file(f);
	  c.write_in_file(newFilePath);

		
	}
      }
    }

    

    
    // std::string line;
    // while (std::getline(file, line)) {
    //   std::istringstream iss(line);
    //   std::string opt, value;
    //   if((iss >> opt  >> value )){
    // 	std::cout << opt << "=" << value << std::endl;
    //   }
      
    
  }
    
  
  

  return 0;
}

