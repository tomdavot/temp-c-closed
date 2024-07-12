
#include "converter.cpp"
#include "options.hpp"

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

  std::cout << options.format << std::endl;
  std::cout << options.formula << std::endl;
  
  //  test(options);
  

  c.set_format(options.format);
  c.set_formula(options.formula);

  
  if(options.input!="-1"){
    c.convert_file(options.input);
    c.write_in_file(options.output);
  }
  else {
    //to do
  }
  

   return 0;
}

