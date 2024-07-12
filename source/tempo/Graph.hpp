#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <ios>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/detail/edge.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/container/flat_set.hpp>


struct TimeWindow : public std::pair<unsigned, unsigned>
{
  using std::pair<unsigned, unsigned>::operator=;
  using std::pair<unsigned, unsigned>::pair;


  TimeWindow& operator=(std::initializer_list<unsigned> list) {
    if (list.size() == 2) {
      auto it = list.begin();
      this->first = *it;
      ++it;
      this->second = *it;
    }
    return *this;
  }

  struct Iterator {
    int current;

    Iterator(int start) : current(start) {}

    int operator*() const { return current; }

    Iterator& operator++() { ++current; return *this; }

    bool operator!=(const Iterator& other) const { return current != other.current; }
  };

  Iterator begin() const { return Iterator(first); }

  Iterator end() const { return Iterator(second + 1); }
  
  unsigned length() const
  {
    return second-first;
  }

  TimeWindow operator++(int)
  {
    first++;
    second++;
    return *this;
  }

};


struct Vertex {
  size_t id;
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
    auto a = lambda.lower_bound(t.first)++;
    return *a>=t.first && *a<= t.second;
  }

  TimeWindow time(TimeWindow t) const
  {
    TimeWindow time;
    time.first=*lambda.lower_bound(t.first);
    time.second=*(--lambda.lower_bound(t.second));
    return time;
  }

  
  Edge() {}
  Edge(boost::container::flat_set<unsigned> && lambda) : lambda(std::move(lambda)) {}
};




using graph_p = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, unsigned, Edge>;



class Graph : public graph_p
{
  struct Debug {
    unsigned x,y, tempx, tempy;
  };

  Debug d;

  struct CommonNeighborhood {
    std::set<unsigned> vertices;
    TimeWindow time;
  };


  
  std::set<unsigned> exclude;
  
  
  TimeWindow time;

  void parse_dot_edge(std::string fileName);

  bool locally_c_closed(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2, unsigned c);

  unsigned min_locally_c_closed(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2);

  bool is_excluded(unsigned v) const;


  
public:

  void debug();
  
  Graph(std::string fileName);


  bool areAdjacent(unsigned u,unsigned v, TimeWindow t);
  bool areAdjacent(unsigned u,unsigned v);

  CommonNeighborhood commonNeighborhood(unsigned u, unsigned v, TimeWindow t);

  
  unsigned commonNeighborhoodSize(unsigned u, unsigned v, TimeWindow t);
  unsigned commonNeighborhoodSize(unsigned u, unsigned v);

  bool is_c_closed(unsigned delta_1, unsigned delta_2, unsigned c);

  unsigned min_c_closed(unsigned delta_1, unsigned delta_2);

  unsigned n_stability();

  unsigned n_pair_stability();

  unsigned n_pair_stability_v2();

  std::pair<unsigned,unsigned> min_gamma(unsigned delta_1, unsigned delta_2,unsigned max_c=0);

  unsigned min_weakly_c_closed(unsigned delta_1, unsigned delta_2);

  void write_stats(std::ofstream &file, std::string nameGraph);

  void write_in_file(std::string fileName);

  void display_gaps();
  
};



