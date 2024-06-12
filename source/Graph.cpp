#include "Graph.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstdlib>
#include <iostream>
#include <limits.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/container/flat_set.hpp>


void Graph::read_dot(std::string fileName)
{
}


void Graph::parse_txt(std::string fileName)
{
  std::ifstream file(fileName.c_str());
  if(!file) {
    std::cout << "Error: could not open " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }
  std::map<std::pair<unsigned,unsigned>,boost::container::flat_set<unsigned>> lambda;
  unsigned u, v, t;
  while (file >> u >> v >> t) {
    //    t=t/(3600*24);
    lambda[{std::min(u,v),std::max(u,v)}].insert(t);
    time.first=std::min(t,time.first);
    time.second=std::max(t,time.second);
  }
  
  
  for(auto &e : lambda)
    boost::add_edge(e.first.first,e.first.second,std::move(e.second),*this);
  
}

Graph::Graph(std::string fileName) : graph_p(), time(UINT_MAX,0)
{
  parse_txt(fileName);
}


bool Graph::areAdjacent(unsigned u,unsigned v, TimeWindow t)
{
  auto e = boost::edge(u,v,*this);
  if(e.second){
    if((*this)[e.first].appears(t))
      return true;
  }
  return false;
}

bool Graph::areAdjacent(unsigned u,unsigned v)
{
  return areAdjacent(u,v,time);
}

unsigned Graph::commonNeighbours(unsigned u, unsigned v, TimeWindow t)
{
  std::set<unsigned> neighbours;
  
  for (auto e : make_iterator_range(out_edges(u, (*this)))){
    if((*this)[e].appears(t))
      neighbours.insert(boost::target(e,*this));
  }
  unsigned count=0;
  for (auto e : make_iterator_range(out_edges(v, (*this)))){
    if((*this)[e].appears(t) && neighbours.find(boost::target(e,*this))!=neighbours.end())
      count++;
  }
  return count;
}

unsigned Graph::commonNeighbours(unsigned u, unsigned v)
{
  return commonNeighbours(u,v,time);
}

bool Graph::locally_c_closed(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2, unsigned c)
{
  if(delta_1>time.length()) return false;
  TimeWindow delta1{1,delta_1}, delta2{1,delta_1+delta_2};
  do {
    if(!areAdjacent(u,v,delta2)){
      if(commonNeighbours(u,v,delta1)>c){
	return false;
      }
    }
    
    delta1++;
    delta2++;
 }while (delta1.second<=time.second);
  return true;
}

bool Graph::is_c_closed(unsigned delta_1, unsigned delta_2, unsigned c)
{
  for(size_t i = 0; i<m_vertices.size()-1;++i){
    for(auto j=i+1;j<m_vertices.size();++j){
      if(!locally_c_closed(i, j, delta_1, delta_2, c))
	return false;
    }
  }
  return true;
}

unsigned Graph::min_c_closed(unsigned delta_1, unsigned delta_2)
{
  unsigned c;
  delta_1=std::min(delta_1,time.length());
  delta_2=std::min(delta_2,time.length());
  for(c=0;!is_c_closed(delta_1, delta_2, c);c++);
  return c;
}

unsigned Graph::n_stability()
{
  unsigned max=0;
  for(auto v : make_iterator_range(this->vertex_set())){
    for(unsigned t=time.first;t<time.second;++t){
      unsigned diff=0;
      for (auto e : make_iterator_range(out_edges(v, (*this)))){
	diff+=((*this)[e].appears(t)!=(*this)[e].appears(t+1));
      }
      max=std::max(max,diff);
    }
  }
  return max;
}

void Graph::test_c_closed(unsigned maxDelta_1, unsigned maxDelta_2)
{
  for(unsigned i=1;i<=maxDelta_1;++i) {
    for(unsigned j=maxDelta_2+1;j>0;--j){
      if(i+j>time.length()) continue;
      std::cout << "Delta1=" << i << " Delta2=" << j-1;
      unsigned t=min_c_closed(i, j-1);
      std::cout << " c=" << t << std::endl;
      //      if(t>=c_closed) return;
    }
  }
}

void Graph::write_stats(std::ofstream & file, std::string nameGraph)
{
  file << nameGraph << "\t";//
  file << this->vertex_set().size() << "\t";
  file << this->m_edges.size() << "\t";
  file << "[" <<this->time.first << "," << this->time.second << "]" << "\t";
  unsigned maxDegree=0;
  for(auto v : make_iterator_range(this->vertex_set())){
    maxDegree = std::max(maxDegree,(unsigned)boost::degree(v,*this));
  }
  file << maxDegree << "\t";
 
  file << n_stability() << "\t";
  file << min_c_closed(time.length(), time.length()) << "\t";
  file << min_c_closed(1,5) << "\t";
  file << min_c_closed(1,10) << "\t";
  file << min_c_closed(5,10) << "\t";
  file << "\n";

}
