#include "Graph.hpp"

#include <algorithm>
#include <boost/graph/filtered_graph.hpp>
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

std::set<unsigned> Graph::commonNeighborhood(unsigned u, unsigned v, TimeWindow t)
{
  
  std::set<unsigned> neighbours_tmp;
  
  for (auto e : make_iterator_range(out_edges(u, (*this)))){
    if((*this)[e].appears(t))
      neighbours_tmp.insert(boost::target(e,*this));
  }
  
  std::set<unsigned> neighbours;
  unsigned count=0;
  for (auto e : make_iterator_range(out_edges(v, (*this)))){
    if((*this)[e].appears(t) && neighbours_tmp.find(boost::target(e,*this))!=neighbours_tmp.end())
      neighbours.insert(boost::target(e, *this));
  }
  return neighbours;
}

unsigned Graph::commonNeighborhoodSize(unsigned u, unsigned v, TimeWindow t)
{
  return commonNeighborhood(u, v, t).size();
}

unsigned Graph::commonNeighborhoodSize(unsigned u, unsigned v)
{
  return commonNeighborhoodSize(u,v,time);
}

bool Graph::locally_c_closed(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2, unsigned c)
{
    if(delta_1>time.length()) return false;
  TimeWindow w1{time.first,time.first+delta_1};
  TimeWindow w2{time.first<delta_2? 0 : time.first-delta_2,time.first+delta_1+delta_2};
 
  do {
    if(!areAdjacent(u,v,w2)){
      if(commonNeighborhoodSize(u,v,w1)>c){
	return false;
      }
    }
    
    w1++;
    w2.second++;
    if(w2.first+delta_2==w1.first) w2.first++;
 }while (w1.second<=time.second);
  return true;
}

unsigned Graph::min_locally_c_closed(unsigned u, unsigned v, unsigned delta_1, unsigned delta_2)
{
  if(delta_1>time.length()){
    delta_1=time.length();
    delta_2=0;
  }
  
  TimeWindow w1{time.first,time.first+delta_1};
  TimeWindow w2{time.first<delta_2? 0 : time.first-delta_2,time.first+delta_1+delta_2};

  unsigned max_c=0;
  do {
    if(!areAdjacent(u,v,w2)) {
      unsigned tmp = std::max(max_c,commonNeighborhoodSize(u,v,w1));
      if(tmp>max_c){
	max_c=tmp;
	d.tempx=u;
	d.tempy=v;
      }
    }
    
    
    w1++;
    w2.second++;
    if(w2.first+delta_2==w1.first) w2.first++;
 }while (w1.second<=time.second);
  return max_c;
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
  unsigned max_c=0;
  for(size_t i = 0; i<m_vertices.size()-1;++i){
    for(auto j=i+1;j<m_vertices.size();++j){
      unsigned tmp =std::max(max_c,min_locally_c_closed(i, j, delta_1, delta_2));
      if(tmp>max_c){
	max_c=tmp;
	d.x=d.tempx;
	d.y=d.tempy;
      }
    }
  }
  return max_c;

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

unsigned Graph::n_pair_stability()
{
  unsigned max=0;
  for(size_t i = 0; i<m_vertices.size()-1;++i){
    for(auto j=i+1;j<m_vertices.size();++j){
      std::set<unsigned> previous = commonNeighborhood(i, j, {1,1});
      for(unsigned t=time.first+1;t<time.second;++t){
	unsigned diff=0;
	std::set<unsigned> current = commonNeighborhood(i, j, {t,t});
	for(auto u : current)
	  diff+=previous.erase(u)==0;
	diff+=previous.size();
	max=std::max(diff,max);
	previous=std::move(current);
      }
    }
  }

  return max;
}

unsigned Graph::n_pair_stability_v2()
{
  unsigned max=0;
  for(size_t i = 0; i<m_vertices.size()-1;++i){
    for(auto j=i+1;j<m_vertices.size();++j){
      std::set<unsigned> previous = commonNeighborhood(i, j, {1,1});
      for(unsigned t=time.first+1;t<time.second;++t){
	unsigned diff=0;
	std::set<unsigned> current = commonNeighborhood(i, j, {t,t});
	for(auto u : current)
	  diff+=previous.erase(u)==0;
	max=std::max({diff,max,(unsigned)previous.size()});
	previous=std::move(current);
      }
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
  file << n_pair_stability() << "\t";
  file << n_pair_stability_v2() << "\t";
  file << min_c_closed(time.length(), time.length()) << "\t";
  file << min_c_closed(0,0) << "\t";
  file << min_c_closed(0,5) << "\t";
  file << min_c_closed(0,10) << "\t";
  file << min_c_closed(5,10) << "\t";
  file << "\n";

}
