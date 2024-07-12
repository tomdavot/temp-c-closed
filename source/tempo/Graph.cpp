#include "Graph.hpp"

#include <algorithm>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/container/flat_set.hpp>

void Graph::parse_dot_edge(std::string fileName)
{
  std::ifstream file(fileName.c_str());
  if(!file) {
    std::cout << "Error: could not open " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }
  
  std::map<std::pair<unsigned,unsigned>,boost::container::flat_set<unsigned>> lambda;
  unsigned u, v, t;

  std::string line;
  
  while (std::getline(file, line)) {
    std::istringstream iss(line);

    if (!(iss >> u >> v)) {
      std::cerr << "Error : uncorrect format: " << line << std::endl;
      exit(EXIT_SUCCESS);
    }
    if(v<u) std::swap(u,v);
	
    while (iss >> t) {
      time.first=std::min(t,time.first);
      time.second=std::max(t,time.second);
      lambda[{std::min(u,v),std::max(u,v)}].insert(t);
    }
  }
  
  
  for(auto &e : lambda)
    boost::add_edge(e.first.first,e.first.second,std::move(e.second),*this);

}

void Graph::write_in_file(std::string fileName)
{
  std::ofstream file(fileName);
  if (!file) {
    std::cerr << "Error : unable to create " << fileName << std::endl;
    return;
  }

  std::map<unsigned,unsigned> m;
  
  for(auto v : make_iterator_range(this->vertex_set()))
    if((unsigned)boost::degree(v,*this)!=0)
      m[v]=m.size();
      
  for(auto e : this->m_edges){
    file << m[boost::source(e, *this)] << " ";
    file << m[boost::target(e, *this)];
    for(auto t : e.get_property().lambda)
      file << " " << t;
    file << std::endl;
  }

  file.close();
}



Graph::Graph(std::string fileName) : graph_p(), time(UINT_MAX,0)
{
  parse_dot_edge(fileName);
}

bool Graph::is_excluded(unsigned v) const
{
  return exclude.find(v)!=exclude.end(); 
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

Graph::CommonNeighborhood Graph::commonNeighborhood(unsigned u, unsigned v, TimeWindow t)
{

  std::set<unsigned> neighbours_tmp;

  for (auto e : make_iterator_range(out_edges(u, (*this)))){
    if(is_excluded(boost::target(e,*this))) continue;
    if((*this)[e].appears(t))
      neighbours_tmp.insert(boost::target(e,*this));
  }

  Graph::CommonNeighborhood neighbours;
  neighbours.time.first=INT_MAX;
  neighbours.time.second=0;
  
  unsigned count=0;
  for (auto e : make_iterator_range(out_edges(v, (*this)))){
    if((*this)[e].appears(t) && neighbours_tmp.find(boost::target(e,*this))!=neighbours_tmp.end()) {
      neighbours.vertices.insert(boost::target(e, *this));
      
      TimeWindow w=(*this)[e].time(t);
      
      neighbours.time.first=std::min(neighbours.time.first,w.first);
      neighbours.time.second=std::max(neighbours.time.second, w.second);

      w=(*this)[boost::edge(u, boost::target(e, *this), *this).first].time(t);
      neighbours.time.first=std::min(neighbours.time.first,w.first);
      neighbours.time.second=std::max(neighbours.time.second, w.second);

    }
  }
  return neighbours;
}

unsigned Graph::commonNeighborhoodSize(unsigned u, unsigned v, TimeWindow t)
{
  return commonNeighborhood(u, v, t).vertices.size();
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

  auto n = commonNeighborhood(u, v, time);
  
  TimeWindow w1{n.time.first,n.time.first+delta_1};
  TimeWindow w2{n.time.first<delta_2? 0 : n.time.first-delta_2,n.time.first+delta_1+delta_2};

  unsigned max_c=0;
  do {
    if(!areAdjacent(u,v,w2)) {
      unsigned common_n = std::count_if(n.vertices.begin(),n.vertices.end(),[&](unsigned n)
      {
	return this->areAdjacent(u,n,w1) && this->areAdjacent(v,n,w1);
      });
      max_c = std::max(max_c,common_n);
    }
    
    w1++;
    w2.second++;
    if(w2.first+delta_2==w1.first) w2.first++;
  }while (w1.second<=n.time.second);
  return max_c;
}

// <value,vertex>
std::pair<unsigned,unsigned> Graph::min_gamma(unsigned delta_1, unsigned delta_2,unsigned max_c) 
{
  std::vector<unsigned> min_gammas(this->vertex_set().size(),0);
  std::pair<unsigned,unsigned> min={UINT_MAX,0};
  
  for(size_t i = 1; i<m_vertices.size()-1;++i){
    if(is_excluded(i)) continue;
    for(auto j=i+1;j<m_vertices.size();++j){
      if(is_excluded(j)) continue;
      unsigned tmp_min=min_locally_c_closed(i, j, delta_1, delta_2);
      min_gammas[i]=std::max(min_gammas[i],tmp_min);
      min_gammas[j]=std::max(min_gammas[j],tmp_min);
    }
    if(min_gammas[i]<=max_c) return {min_gammas[i],i}; // small optimization: if in the order a vertex with a gamma bigger than i has been found -> return i
    if(min_gammas[i]<min.first)
      min={min_gammas[i],i};
  }
  return min;
}


unsigned Graph::min_weakly_c_closed(unsigned delta_1, unsigned delta_2)
{
  unsigned max_c=0;
  exclude.clear();
  
  while(exclude.size()<this->m_vertices.size()-2) {
    auto p = min_gamma(delta_1, delta_2);
    // std::cout << p.second << " ";
    exclude.insert(p.second);
    max_c=std::max(max_c,p.first);
  }
  exclude.clear();
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
    if(is_excluded(i)) continue;
    for(auto j=i+1;j<m_vertices.size();++j){
      if(is_excluded(j)) continue;
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
      std::set<unsigned> previous = commonNeighborhood(i, j, {1,1}).vertices;
      for(unsigned t=time.first+1;t<time.second;++t){
	unsigned diff=0;
	std::set<unsigned> current = commonNeighborhood(i, j, {t,t}).vertices;
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
      std::set<unsigned> previous = commonNeighborhood(i, j, {1,1}).vertices;
      for(unsigned t=time.first+1;t<time.second;++t){
	unsigned diff=0;
	std::set<unsigned> current = commonNeighborhood(i, j, {t,t}).vertices;
	for(auto u : current)
	  diff+=previous.erase(u)==0;
	max=std::max({diff,max,(unsigned)previous.size()});
	previous=std::move(current);
      }
    }
  }

  return max;
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
  file << min_weakly_c_closed(0,0) << "\t";
  file << min_c_closed(0,5) << "\t";
  file << min_weakly_c_closed(0,5) << "\t";
  file << min_c_closed(0,10) << "\t";
  file << min_weakly_c_closed(0,10) << "\t";
  file << min_c_closed(5,10) << "\t";
  file << min_weakly_c_closed(5,10) << "\t";
  file << "\n";

}


void Graph::display_gaps()
{
  bool gap=false, br=false;
  size_t start,end;
  std::cout << this->time.first << "-" << this->time.second << std::endl;
  for(size_t i : this->time) {
    //    std::cout << i << std::endl;
    br=true;
    auto it=this->m_edges.begin();

    if(std::any_of(this->m_edges.begin(),this->m_edges.end(),
		   [&](auto &e){return e.m_property.appears(i); })){
      if(gap && i-1-start>5) std::cout << "[" << start << "," << i-1 << "]" << std::endl;
      gap=false;
    }
    else {
      if(!gap) start=i;
      gap=true;
    }
    
  }
}
