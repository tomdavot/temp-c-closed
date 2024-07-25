#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <functional>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/detail/edge.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/container/flat_set.hpp>


struct TimeWindow //: public std::pair<unsigned, unsigned>
{
  // using std::pair<unsigned, unsigned>::operator=;
  // using std::pair<unsigned, unsigned>::pair;

private:
  // we allow low_bound to have a negative value for convenience
  int low_bound, up_bound;
  bool has_been_init;

  template<typename t>
  void init(std::initializer_list<t> list)
  {
    if(list.size()!=2) {
      std::cout << "Error, TimeWindow must be initialized with a list of lenght 2\n";
      exit(EXIT_SUCCESS);
    }

    if (list.size() == 2) {
      auto it = list.begin();
      this->low_bound = (int)*it;
      ++it;
      this->up_bound = (int)*it;
    }
    if(low_bound>up_bound) {
      std::cout << "Error, lower bound bigger than upper bound in TimeWindow\n";
    }
  }

public:

  unsigned lower_bound() const
  {
    return low_bound < 0 ? 0 : low_bound;
  }

  unsigned upper_bound() const
  {
    return up_bound;
  }

  TimeWindow() : low_bound(0), up_bound(0), has_been_init(false) {}
  
  template<typename t>
  TimeWindow(std::initializer_list<t> list) :  has_been_init(true)
  {
    init(list);
  }

  template<typename t>
  TimeWindow& operator=(std::initializer_list<t> list) {
    init(list);
    return *this;
  }

  TimeWindow(int lower_bound, int upper_bound) :  has_been_init(true)
  {
    init({lower_bound,upper_bound});
  }


  bool increase(const TimeWindow &t)
  {
    if(!has_been_init){

      low_bound=t.lower_bound();
      up_bound=t.upper_bound();

      has_been_init=true;
      return true;
    }
    bool r=false;
    if(t.lower_bound()<low_bound){
      r=true;
      low_bound=t.lower_bound();
    }
    if(t.upper_bound()>up_bound){
      r=true;
      up_bound=t.upper_bound();
    }
    return r;
  }

  bool increase(unsigned t)
  {
    return increase(TimeWindow(t,t));
  }



  struct Iterator {
    int current;

    Iterator(int start) : current(start) {}

    int operator*() const { return current; }

    Iterator& operator++() { ++current; return *this; }

    bool operator!=(const Iterator& other) const { return current != other.current; }
  };

  Iterator begin() const { return Iterator(lower_bound()); }

  Iterator end() const { return Iterator(upper_bound() + 1); }
  
  unsigned length() const
  {
    return upper_bound()-lower_bound();
  }

  unsigned max_length() const
  {
    return upper_bound()-lower_bound();
  }


  TimeWindow operator++(int)
  {
    low_bound++;
    up_bound++;
    return *this;
  }

  TimeWindow operator--(int)
  {
    low_bound--;
    up_bound--;
    return *this;
  }

  void display() const {
    std::cout << "[" << low_bound << "," << up_bound << "]";
  }


};


struct Edge {
  boost::container::flat_set<unsigned> lambda;

public:
  // ****************************
  //          For Debug 
  // ****************************
  std::string label;
   
  const std::string & set_label();
  // ****************************

  bool appears(unsigned t) const
  {
    return std::binary_search(lambda.begin(),lambda.end(),t);
  }
  bool appears(TimeWindow t) const
  {
    auto a = lambda.lower_bound(t.lower_bound())++;
    if(a==lambda.end()) return false;
    return *a>=t.lower_bound() && *a<= t.upper_bound();
  }

  TimeWindow time(TimeWindow t) const
  {
    if(lambda.size()==1)
      return {(int)*lambda.begin(),(int)*lambda.begin()};
    if(t.lower_bound()==t.upper_bound()) return t; // we suppose that lambda necessarily intersects t 

    int l=*lambda.lower_bound(t.lower_bound());
    
    int u=*(--lambda.lower_bound(t.upper_bound()));
    TimeWindow time(l,u);

    return time;
  }
 
  
  Edge() {}
  Edge(boost::container::flat_set<unsigned> && lambda) : lambda(std::move(lambda)) {}
};



using Vertex = unsigned;
using graph_p = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex, Edge>;



class Graph : public graph_p
{
  struct Debug {
    unsigned x,y, tempx, tempy, u, v;
    bool display=false;
  };

  Debug d;

  struct CommonNeighborhood {
    std::set<Vertex> vertices;
    TimeWindow time;
  };
  
  std::set<Vertex> exclude;
  
  
  TimeWindow time;

  bool is_excluded(Vertex v) const;


  
public:

  unsigned max_locally_c_closed(Vertex u, Vertex v, unsigned delta_1, unsigned delta_2, unsigned opt=UINT_MAX);

  unsigned tmp_max_locally_c_closed(Vertex u, Vertex v, unsigned delta_1, unsigned delta_2);

  // unsigned max_locally_c_closed_v2(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2);

  void debug();
  
  Graph();
 
  bool parse_dot_edge(std::string fileName);

  bool areAdjacent(Vertex u, Vertex v, TimeWindow t);
  bool areAdjacent(Vertex u, Vertex v);

  CommonNeighborhood commonNeighborhood(Vertex u, Vertex v, TimeWindow t);

  unsigned value_c_closed(unsigned delta_1, unsigned delta_2);

  unsigned n_stability();

  unsigned n_pair_stability();
  unsigned tmp_n_pair_stability();

  unsigned n_pair_stability_v2();

  unsigned local_pair_stability(Vertex u,Vertex v);
 
  enum apply_type {MIN,MAX,MIN_MAX,MAX_MIN};

  void write_stats(std::string fileName, std::string graphName);

  void write_in_file(std::string fileName);

  void display_gaps();

  unsigned min_gamma_unstability_decomposition(unsigned delta_1, unsigned delta_2);

// *******************************
//        Generic functions
// ******************************

  template<typename Comp2 = std::less<>,typename Comp1 = std::greater<>>
  std::pair<unsigned,Vertex> apply_every_pair(std::function<unsigned(Vertex,Vertex)>score, std::function<bool(unsigned)> opti= [](unsigned){ return false;});
  

  template<typename C = std::greater<>>
  std::pair<unsigned,TimeWindow> apply_every_time_window(unsigned delta_1, unsigned delta_2, TimeWindow w, std::function<unsigned(TimeWindow w1,TimeWindow w2)>score,
							 std::function<bool(unsigned)> opti= [](unsigned){ return false;});

  std::vector<Vertex> degeneracy(std::function<unsigned(Vertex,Vertex)> score_func, unsigned &score);

  
};

