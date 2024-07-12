#include <boost/spirit/home/qi/action/action.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <map>
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#if defined(_MSC_VER)
# pragma warning(disable: 4345)
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/foreach.hpp>
#include <boost/phoenix.hpp>

#include <iostream>
#include <string>


unsigned month_to_days(unsigned m) {
  unsigned r = 0;

  switch (m) {
  case 12: r += 30;
  case 11: r += 31;
  case 10: r += 30;
  case 9:  r += 31;
  case 8:  r += 31;
  case 7:  r += 30;
  case 6:  r += 31;
  case 5:  r += 30;
  case 4:  r += 31;
  case 3:  r += 28;
  case 2:  r += 31;
  case 1:  break;
  default:
    std::cout << "Unvalide month\n";
    return 0;
  }
  return r;
}

namespace ast
{
  struct nil {};
  struct signed_;
  struct program;
  struct func;
  struct variable;


  typedef boost::variant<nil,unsigned int,
			 boost::recursive_wrapper<signed_>,
			 boost::recursive_wrapper<program>,
			 boost::recursive_wrapper<func>,
			 boost::recursive_wrapper<variable>> operand;

  struct signed_
  {
    char sign;
    operand operand_;
  };

  struct operation
  {
    char operator_;
    operand operand_;
  };

  struct program
  {
    operand first;
    std::list<operation> rest;
  };

  struct func
  {
    char f_name;
    operand operand_;
  };

  struct variable
  {
    int d,id;
  };
  
}

BOOST_FUSION_ADAPT_STRUCT(ast::signed_,(char, sign)(ast::operand, operand_))

BOOST_FUSION_ADAPT_STRUCT(ast::operation,(char, operator_)(ast::operand, operand_))

BOOST_FUSION_ADAPT_STRUCT(ast::program,
                          (ast::operand, first)
			  (std::list<ast::operation>,rest))

BOOST_FUSION_ADAPT_STRUCT(ast::func,
                          (char, f_name)
			  (ast::operand,operand_))


BOOST_FUSION_ADAPT_STRUCT(ast::variable,(int, d)(int, id))

namespace ast
{
  struct eval
  {
    typedef int result_type;
      
    std::map<unsigned,unsigned> &vars;

    eval(std::map<unsigned,unsigned> &v) : vars(v) {}

    int operator()(nil) const { BOOST_ASSERT(0); return 0; }
    int operator()(unsigned int n) const { return n; }

    int operator()(operation const& x, int lhs) const
    {
      int rhs = boost::apply_visitor(*this, x.operand_);
      switch (x.operator_)
	{
	case '+': return lhs + rhs;
	case '-': return lhs - rhs;
	case '*': return lhs * rhs;
	case '/': return lhs / rhs;
	}
      BOOST_ASSERT(0);
      return 0;
    }

    int operator()(signed_ const& x) const
    {
      int rhs = boost::apply_visitor(*this, x.operand_);
      switch (x.sign)
	{
	case '-': return -rhs;
	case '+': return +rhs;
	}
      BOOST_ASSERT(0);
      return 0;
    }

    int operator()(program const& x) const
    {
      int state = boost::apply_visitor(*this, x.first);
      BOOST_FOREACH(operation const& oper, x.rest)
	{
	  state = (*this)(oper, state);
	}
      return state;
    }

    int operator()(func const& x) const
    {
      int state = boost::apply_visitor(*this, x.operand_);
      switch (x.f_name) {
      case 'm':
	return month_to_days(state);
	break;
      default:
	std::cout << "Unknown function name " << x.f_name << std::endl;
	exit(EXIT_SUCCESS);
	return 0;
	break;
      }
    }
        
    int operator()(variable const& x) const
    {
      auto it = vars.find(x.id);
      if (it != vars.end())
	return it->second;
      else
	throw std::runtime_error("Unknown variable: " + std::to_string(x.id));
    }
      
  };
}

namespace calc
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  template <typename Iterator>
  struct calculator : qi::grammar<Iterator, ast::program(), ascii::space_type>
  {
    calculator() : calculator::base_type(expression)
    {
      qi::uint_type uint_;
      qi::char_type char_;
      qi::string_type string_;
      qi::alpha_type alpha_;

      expression =
	term
	>> *(   (char_('+') >> term)
		|   (char_('-') >> term)
		)
	;

      term =
	factor
	>> *(   (char_('*') >> factor)
		|   (char_('/') >> factor)
		)
	;

      factor =
	uint_
	| variable
	| function
	|   '(' >> expression >> ')'
	|   (char_('-') >> factor)
	|   (char_('+') >> factor)
	;
      
	    
      variable =
	(char_('#') >>  uint_) ;

      function = alpha_ >> '(' >> expression >> ')';
	                    
    }

    qi::rule<Iterator, ast::program(), ascii::space_type> expression;
    qi::rule<Iterator, ast::program(), ascii::space_type> term;
    qi::rule<Iterator, ast::operand(), ascii::space_type> factor;
    qi::rule<Iterator, ast::variable(), ascii::space_type> variable;
    qi::rule<Iterator, ast::func(), ascii::space_type> function;
  };
}

int calc_main()
{
  std::cout << "/////////////////////////////////////////////////////////\n\n";
  std::cout << "Expression parser...\n\n";
  std::cout << "/////////////////////////////////////////////////////////\n\n";
  std::cout << "Type an expression...or [q or Q] to quit\n\n";

  typedef std::string::const_iterator iterator_type;
  typedef calc::calculator<iterator_type> calculator;
  typedef ast::program ast_program;
  typedef ast::eval ast_eval;

  std::string str;
  while (std::getline(std::cin, str))
    {
      if (str.empty() || str[0] == 'q' || str[0] == 'Q')
	break;

      calculator calc;        // Our grammar
      ast_program program;    // Our program (AST)

      std::map<unsigned, unsigned> map;
      ast_eval eval(map);          // Evaluates the program

      eval.vars[1]=10;
      eval.vars['#']=11;
	
      std::string::const_iterator iter = str.begin();
      std::string::const_iterator end = str.end();
      boost::spirit::ascii::space_type space;
      bool r = phrase_parse(iter, end, calc, space, program);

      if (r && iter == end)
        {
	  std::cout << "-------------------------\n";
	  std::cout << "Parsing succeeded\n";
	  //            print(program);
	  std::cout << "\nResult: " << eval(program) << std::endl;
	  std::cout << "-------------------------\n";
        }
      else
        {
	  std::string rest(iter, end);
	  std::cout << "-------------------------\n";
	  std::cout << "Parsing failed\n";
	  std::cout << "stopped at: \" " << rest << "\"\n";
	  std::cout << "-------------------------\n";
        }
    }

  std::cout << "Bye... :-) \n\n";
  return 0;
}

