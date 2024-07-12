#include <boost/phoenix/core/reference.hpp>
#include <boost/phoenix/stl/container/container.hpp>
#include <boost/spirit/home/qi/numeric/real.hpp>
#include <boost/spirit/home/qi/numeric/uint.hpp>
#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/container/flat_set.hpp>


#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "calculator.cpp"

 
namespace parser
{
    namespace qi = boost::spirit::qi;
    namespace phoenix = boost::phoenix;


  template <typename Iterator>
  struct parser_line 
  {
    qi::rule<Iterator> start;


    std::map<unsigned,unsigned> variable_map;
    std::string u,v;
    
    parser_line() : start(qi::eps) {}

    void add_u()
    {
      //      std::cout << "add u " << std::endl;
      start= start.copy()
	>> +(qi::alnum[phoenix::push_back(phoenix::ref(u), qi::_1)]);
    }

    void add_v()
    {
      //std::cout << "add v " << std::endl;
      start= start.copy()
	>> +(qi::alnum[phoenix::push_back(phoenix::ref(v), qi::_1)]);
    }

    void add_char(char c)
    {
      //std::cout << "add char " << c << std::endl;
      start= start.copy()
      	>> qi::char_(c);
    }

    void add_bulk()
    {
      //std::cout << "add bulk " << std::endl;
      start=start.copy() >> *qi::alnum;
    }

    void add_variable(unsigned i)
    {
      //std::cout << "add variable " << i << std::endl;
      variable_map[i]=0;
      start=start.copy() >> qi::int_[phoenix::ref(variable_map)[i]=qi::_1];
    }

    bool parse_line(std::string line)
    {
      variable_map.clear();
      u.clear();
      v.clear();
      return parse(line.begin(),line.end(),start);
    
    }
  };

  template <typename Iterator>
  struct parser_format : qi::grammar<Iterator>
  {
  private:

    qi::rule<Iterator> u,v, separator, bulk, variable,between;
  public:
    
    qi::rule<Iterator> start;
    
    parser_format(parser_line<Iterator> &p) : parser_format::base_type(start)
    {
      u = qi::char_('u')[boost::bind(&parser_line<Iterator>::add_u,&p)];
      v = qi::char_('v')[boost::bind(&parser_line<Iterator>::add_v,&p)];

      variable = qi::int_[boost::bind(&parser_line<Iterator>::add_variable,&p,boost::placeholders::_1)];
       
      
      separator = (qi::punct | qi::blank) [boost::bind(&parser_line<Iterator>::add_char,&p,boost::placeholders::_1)];

      bulk = qi::char_('b')[boost::bind(&parser_line<Iterator>::add_bulk,&p)];
      
      between=*((variable | bulk) >> +separator);
      
      start= qi::eps >> between >> u >> +separator >> between >> v >> *(+separator >>(variable | bulk));
  
    }
  };
}

struct Converter
{
  parser::parser_line<std::string::iterator> p_line;
  parser::parser_format<std::string::iterator> p_format;

  calc::calculator<std::string::iterator> calc;
  ast::program program;
  ast::eval eval;

  std::map<std::string,unsigned> vertex_map;
  std::map<std::pair<unsigned,unsigned>,boost::container::flat_set<unsigned>> lambda;
  
  
  Converter() : p_format(p_line), eval(p_line.variable_map) {}

  void set_format(std::string format) {
    std::cout << "##############################################\n";
    std::cout << "#               Parse Format                 #\n";
    std::cout << "##############################################\n";
    auto it=format.begin();
    if(parse(it,format.end(),p_format.start) && it==format.end()){
      std::cout << "#                                            #\n";
      std::cout << "#        Format successfully parsed          #\n";
      std::cout << "#                                            #\n";
      std::cout << "##############################################\n";

    } else {
      std::cout << "#                                            #\n";
      std::cout << "#         Error, incorrect format            #\n";
      std::cout << "#                                            #\n";
      std::cout << "##############################################\n";
      std::cout << "Rest: " << std::string(it,format.end()) << std::endl;

      exit(EXIT_SUCCESS);
    }
  }

  void set_formula(std::string formula)
  {
    boost::spirit::ascii::space_type space;
    
    std::cout << "##############################################\n";
    std::cout << "#              Parse Formula                 #\n";
    std::cout << "##############################################\n";
    
    if(phrase_parse(formula.begin(),formula.end(),calc,space,program)){
      std::cout << "#                                            #\n";
      std::cout << "#       Formula successfully parsed          #\n";
      std::cout << "#                                            #\n";
      std::cout << "##############################################\n";

    } else {
      std::cout << "#                                            #\n";
      std::cout << "#        Error, incorrect formula            #\n";
      std::cout << "#                                            #\n";
      std::cout << "##############################################\n";

      exit(EXIT_SUCCESS);
    }
  }

  void parse_line(std::string line,size_t nb_line)
  {
    //    std::cout << "Line " << nb_line << " =>";
    if(p_line.parse_line(line)){
      // std::cout << "Success\n";
      // std::cout << "u:  "<< p_line.u << std::endl;
      // std::cout << "v:  "<< p_line.v << std::endl;
      // for(auto p : p_line.variable_map){
      // 	std::cout << "#" << p.first << ": " << p.second << std::endl; 
      // }
      // std::cout << "eval: " << eval(program) << std::endl;
      unsigned u = vertex_map.try_emplace(p_line.u,vertex_map.size()).first->second;
      unsigned v = vertex_map.try_emplace(p_line.v,vertex_map.size()).first->second;
      unsigned t=eval(program);
      lambda[{std::min(u,v),std::max(u,v)}].insert(t);
    }
    else {
      std::cout << "Line " << nb_line << " =>";
      std::cout << "Failed\n";
    }
  }

  void convert_file(std::string fileName)
  {
    std::cout << "Reading " << fileName << std::endl;
    std::ifstream file(fileName.c_str());
    if(!file) {
      std::cout << "Error: could not open " << fileName << std::endl;
      exit(EXIT_FAILURE);
    }
    std::string line;

    size_t i=0;
    while (std::getline(file, line)){
      i++;
      parse_line(line,i);
    }
      
    
    
  }


  void write_in_file(std::string fileName)
  {
    std::cout << "Writing graph " << fileName << std::endl;
    std::ofstream file(fileName);
    if (!file) {
      std::cerr << "Error : unable to create " << fileName << std::endl;
      return;
    }

    for(auto p1 : lambda) {
      file << p1.first.first << " " << p1.first.second << " ";
      for(auto t : p1.second)
	file << t << " ";
      file << std::endl;
    }
  
    file.close();
  }


  void display() const // for debug
  {
    for(auto p1 : lambda) {
      std::cout << p1.first.first << "-" << p1.first.second << ":";
      for(auto t : p1.second)
	std::cout << t << " ";
      std::cout << std::endl;
    }
  }

  

};   
