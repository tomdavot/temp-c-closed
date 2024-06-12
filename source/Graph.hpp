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

  bool appears(unsigned t)
  {
    return std::binary_search(lambda.begin(),lambda.end(),t);
  }
  bool appears(TimeWindow t)
  {
    auto a = lambda.lower_bound(t.first)++;
    return *a>=t.first && *a<= t.second;
  }

  
  Edge() {}
  Edge(boost::container::flat_set<unsigned> && lambda) : lambda(std::move(lambda)) {}
};




using graph_p = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex, Edge>;



class Graph : public graph_p
{
  TimeWindow time;

  void parse_txt(std::string fileName);
  
  void read_dot(std::string fileName);

  bool locally_c_closed(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2, unsigned c);

  
public:

  void debug();
  
  Graph(std::string fileName);


  bool areAdjacent(unsigned u,unsigned v, TimeWindow t);
  bool areAdjacent(unsigned u,unsigned v);


  unsigned commonNeighbours(unsigned u, unsigned v, TimeWindow t);
  unsigned commonNeighbours(unsigned u, unsigned v);

  bool is_c_closed(unsigned delta_1, unsigned delta_2, unsigned c);

  unsigned min_c_closed(unsigned delta_1, unsigned delta_2);

  unsigned n_stability();

  void test_c_closed(unsigned maxDelta_1, unsigned maxDelta_2);

  void write_stats(std::ofstream &file, std::string nameGraph);
  
};



