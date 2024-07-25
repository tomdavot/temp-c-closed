#include "Graph.hpp"
 
#include <algorithm>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/subgraph.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/container/flat_set.hpp>
#include <ostream>

bool Graph::parse_dot_edge(std::string fileName)
{
  std::ifstream file(fileName.c_str());
  if(!file) {
    std::cout << "Error: could not open " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }
  
  std::map<std::pair<Vertex,Vertex>,boost::container::flat_set<unsigned>> lambda;
  Vertex u,v;
  unsigned t;

  std::string line;
  unsigned l=1;
  while (std::getline(file, line)) {
    std::istringstream iss(line);

    if (!(iss >> u >> v)) {
      std::cerr << "Error: uncorrect format: " << line << std::endl;
      exit(EXIT_SUCCESS);
    }
    if(v<u) std::swap(u,v);

    bool empty=true;
    while (iss >> t) {
      empty=false;
      time.increase(t);
      lambda[{std::min(u,v),std::max(u,v)}].insert(t);
    }
    if(empty) {
      std::cerr << "Error: no time specification at line " << l << std::endl;
      return false;
    }
    l++;
  }

  for(auto &e : lambda)
    boost::add_edge(e.first.first,e.first.second,std::move(e.second),*this);
  time.display();
  std::cout << std::endl;

  
  
  return true;
}

void Graph::write_in_file(std::string fileName)
{
  std::ofstream file(fileName);
  if (!file) {
    std::cerr << "Error: unable to create " << fileName << std::endl;
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



Graph::Graph() : graph_p(), time()
{
  //  parse_dot_edge(fileName);
}

bool Graph::is_excluded(Vertex v) const
{
  return exclude.find(v)!=exclude.end(); 
}

bool Graph::areAdjacent(Vertex u, Vertex v, TimeWindow t)
{
  auto e = boost::edge(u,v,*this);
  if(e.second){
    if((*this)[e.first].appears(t))
      return true;
  }
  return false;
}

bool Graph::areAdjacent(Vertex u, Vertex v)
{
  return areAdjacent(u,v,time);
}

Graph::CommonNeighborhood Graph::commonNeighborhood(Vertex u, Vertex v, TimeWindow t)
{

  std::set<Vertex> neighbours_tmp;

  for (auto x : make_iterator_range(boost::adjacent_vertices(u,*this))) {
    auto e=boost::edge(u, x, *this);
    if(is_excluded(x)) continue;
    
    if((*this)[e.first].appears(t))
      neighbours_tmp.insert(x);
  }

  Graph::CommonNeighborhood neighbours;
  
  unsigned count=0;
  for (auto e : make_iterator_range(out_edges(v, (*this)))){
    if((*this)[e].appears(t) && neighbours_tmp.find(boost::target(e,*this))!=neighbours_tmp.end()) {
      neighbours.vertices.insert(boost::target(e, *this));
      
      TimeWindow w=(*this)[e].time(t);

      neighbours.time.increase(w);

      w=(*this)[boost::edge(u, boost::target(e, *this), *this).first].time(t);
      neighbours.time.increase(w);
      
    }
  }
  return neighbours;
}


unsigned Graph::max_locally_c_closed(Vertex u, Vertex v, unsigned delta_1, unsigned delta_2, unsigned opt) 
{ 
  auto n = commonNeighborhood(u, v, time);

  auto tmp= apply_every_time_window(delta_1, delta_2, n.time, [&](TimeWindow w1, TimeWindow w2)
  {
    if(areAdjacent(u,v,w2)) return (unsigned)0;
    return (unsigned)std::count_if(n.vertices.begin(),n.vertices.end(),[&](unsigned n)
    {
      return this->areAdjacent(u,n,w1) && this->areAdjacent(v,n,w1);
    });
  },
    [&](unsigned s) {return s > opt;}
    
    );
  
  return tmp.first;
}


unsigned Graph::min_gamma_unstability_decomposition(unsigned delta_1, unsigned delta_2)
{

  std::function<unsigned(Vertex,Vertex)>f = [&](Vertex u, Vertex v){
    return std::max(max_locally_c_closed(u, v, delta_1, delta_2),
		    local_pair_stability(u,v));
  };
  unsigned score;
  degeneracy(f,score);
  
  return score;
}


unsigned Graph::value_c_closed(unsigned delta_1, unsigned delta_2)
{
  return apply_every_pair<std::greater<>>(
		   [&](unsigned u, unsigned v){
		     return max_locally_c_closed(u, v, delta_1, delta_2);
		   }).first;
}


unsigned Graph::n_stability()
{
  unsigned max=0;
  for(auto v : make_iterator_range(this->vertex_set())){
    for(unsigned t : time){
      unsigned diff=0;
      for (auto e : make_iterator_range(out_edges(v, (*this)))){
	diff+=((*this)[e].appears(t)!=(*this)[e].appears(t+1));
      }
      max=std::max(max,diff);
    }
  }
  return max;
}

unsigned Graph::tmp_n_pair_stability()
{
  unsigned max=0;
  for(size_t i = 0; i<m_vertices.size()-1;++i){
    for(auto j=i+1;j<m_vertices.size();++j){
      if(j==2) exit(EXIT_SUCCESS);
      
      std::set<unsigned> previous = commonNeighborhood(i, j, {this->time.lower_bound(),this->time.lower_bound()}).vertices;
      for(unsigned t=time.lower_bound()+1;t<time.upper_bound();++t){
	if(t==15) exit(EXIT_SUCCESS);

	std::cout << "***************************" << std::endl;
	std::cout << "t= "<<t << ": " << std::endl;

	
	std::cout << "previous: ";
	unsigned x=0;
	for(auto u : previous)
	  std::cout << u << "(" << ++x << ") ";
	std::cout << std::endl;
	
	
	unsigned diff=0;
	std::set<unsigned> current = commonNeighborhood(i, j, {t,t}).vertices;
	
	std::cout << "current: ";
	x=0;
	for(auto u : current)
	  std::cout << u << "(" << ++x << ") ";
	std::cout << std::endl;

	for(auto u : current)
	  diff+=previous.erase(u)==0;
	
	
	diff+=previous.size();
	max=std::max(diff,max);
	previous=std::move(current);

	std:: cout << "Max: " << max << std::endl;
	std::cout << "***************************" << std::endl;
	
      }
    }
  }

  return max;
}

unsigned Graph::n_pair_stability()
{
  auto tmp = apply_every_pair<std::greater<>>([&](Vertex u, Vertex v){
			    std::set<Vertex> previous = commonNeighborhood(u, v,{this->time.lower_bound(),this->time.lower_bound()}).vertices;
      
    auto tmp=apply_every_time_window(0, 0, this->time,
				   [&](TimeWindow w1, TimeWindow w2){
				     unsigned diff=0;
				     std::set<unsigned> current = commonNeighborhood(u, v, w1).vertices;
				     for(auto x : current)
				       diff+=previous.erase(x)==0;
				     diff+=previous.size();
				     previous=std::move(current);
				     return diff;
				   });
    return tmp.first;
			  });

  
  
  return tmp.first;
}

unsigned Graph::n_pair_stability_v2()
{

  return apply_every_pair<std::greater<>>([&](Vertex u, Vertex v){
      std::set<unsigned> previous = commonNeighborhood(u, v, {this->time.lower_bound(),this->time.lower_bound()}).vertices;
      
    return apply_every_time_window(0, 0, this->time,
			    [&](TimeWindow w1, TimeWindow w2){
			      unsigned diff=0;
			      std::set<Vertex> current = commonNeighborhood(u, v, w1).vertices;
	for(auto x : current)
	  diff+=previous.erase(x)==0;
	unsigned tmp= std::max({diff,(unsigned)previous.size()});
	previous=std::move(current);
	return tmp;
			    }).first;
  }).first;
  
}


unsigned Graph::local_pair_stability(Vertex u, Vertex v)
{
  std::set<Vertex> previous = commonNeighborhood(u, v, {this->time.lower_bound(),this->time.lower_bound()}).vertices;
  return apply_every_time_window(0, 0, this->time,
			  [&](TimeWindow w1, TimeWindow w2){
			    unsigned diff=0;
			    std::set<Vertex> current = commonNeighborhood(u, v, w1).vertices;
			    for(auto u : current)
			      diff+=previous.erase(u)==0;
			    unsigned tmp=std::max(diff,(unsigned)previous.size());
			    previous=std::move(current);
			    return tmp;
			  }).first;
}

void Graph::write_stats(std::string fileName, std::string graphName)
{

  std::cout << "writing stats" << std::endl;
  std::ofstream file(fileName, std::ios::app);
  if (!file.is_open()) {
    std::cerr << "Error : not able to open " << fileName << std::endl;
    return;
  }
  
  unsigned deg1=0,deg2=0;
  file << graphName << "\t";//
  file << this->vertex_set().size();
  file <<"\t";

  
  file << this->m_edges.size() << "\t";
  file << "[" <<this->time.lower_bound() << "," << this->time.upper_bound() << "]" << "\t";
  unsigned maxDegree=0;
  unsigned minDegree=this->vertex_set().size();
  for(auto v : make_iterator_range(this->vertex_set())){
    deg1+=(unsigned)boost::degree(v,*this)==1;
    deg2+=(unsigned)boost::degree(v,*this)==2;
    maxDegree = std::max(maxDegree,(unsigned)boost::degree(v,*this));
    minDegree = std::min(minDegree,(unsigned)boost::degree(v,*this));
  }
  file << maxDegree << "|" << minDegree << "\t";
  file << deg1 <<"|" << deg2 << "\t";

  
  file << n_stability() << "\t";
  //  display_gaps();
  
  // std::cout << "old version: " << tmp_n_pair_stability() << std::endl;
  // exit(EXIT_SUCCESS);
  
  file << n_pair_stability() << "\t";
  file << n_pair_stability_v2() << "\t";


  
  file << value_c_closed(time.length(), time.length()) << "\t";
  
  std::vector<std::pair<unsigned,unsigned>> t = {
    {0,0},{0,5},{0,10},{5,10}
  };

  // std::vector<std::pair<unsigned,unsigned>> t = {
  //   {0,0}
  // };

  

  for(auto tmp : t){
    std::cout << "{" << tmp.first << "," << tmp.second << "}\n";
    file << value_c_closed(tmp.first,tmp.second) << "\t";
    file << min_gamma_unstability_decomposition(tmp.first,tmp.second) << "\t";
  }

  file << "\n";
  file.close();
  
}


void Graph::display_gaps()
{
  bool gap=false, br=false;
  size_t start,end;
  std::cout << this->time.lower_bound() << "-" << this->time.lower_bound() << std::endl;
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


// *******************************
//        Generic functions
// ******************************

 template<typename C>
 std::pair<unsigned,TimeWindow> Graph::apply_every_time_window(unsigned delta_1, unsigned delta_2, TimeWindow w, std::function<unsigned(TimeWindow w1,TimeWindow w2)>score, std::function<bool(unsigned)> opti)
  {
    C comp;
  
    if(delta_1>time.length()){
      delta_1=time.length();
      delta_2=0;
    }
  
  TimeWindow w1{w.lower_bound(),w.lower_bound()+delta_1};
  TimeWindow w2{w.lower_bound()-delta_2,w.lower_bound()+delta_1+delta_2};

  std::pair<unsigned,TimeWindow> ret;
  bool ret_init=false;
  do {

    unsigned tmp=score(w1,w2);
    if(!ret_init || comp(tmp,ret.first)){
      ret_init=true;
      ret= {tmp,w1};
      if(opti(ret.first)) return ret;
    }
    
    w1++;
    w2++;
    
  }while (w1.upper_bound()<=w.upper_bound());
 
  return ret;

}

template<typename Comp2,typename Comp1>
std::pair<unsigned,Vertex> Graph::apply_every_pair(std::function<unsigned(Vertex,Vertex)>score, std::function<bool(unsigned)> opti)
  {
    Comp1 c1;
    Comp2 c2;
    
    std::map<Vertex,unsigned> map;

    bool init=false;
    Vertex x = *vertex_set().begin(), y= *(std::next(vertex_set().begin()));

    bool ret_set=false;
    std::pair<unsigned,Vertex> ret;

  
  for(auto it_u = vertex_set().begin(); it_u!=vertex_set().end();++it_u){
    if(is_excluded(*it_u)) continue;
    for(auto it_v= std::next(it_u);it_v!=vertex_set().end();++it_v){
      if(is_excluded(*it_v)) continue;
      unsigned tmp=score(*it_u, *it_v);
      
      if(map.find(*it_u)!=map.end()){
	map[*it_u]= c1(tmp,map[*it_u]) ? tmp : map[*it_u];
      } else map[*it_u]=tmp;

      if(map.find(*it_v)!=map.end()){
	map[*it_v]= c1(tmp,map[*it_v]) ? tmp : map[*it_v];
      } else map[*it_v]=tmp;

    }
    if(!ret_set){
      ret_set=true;
      ret={map[*it_u],*it_u};
    }

    if(c2(map[*it_u],ret.first)) {
      ret={map[*it_u],*it_u};
      // optimisation: if a worst value as already been encountered we can return ret
      if(opti(ret.first)) return ret;
    }
  }
  
  return ret;
  }


std::vector<Vertex>
Graph::degeneracy(std::function<unsigned(Vertex, Vertex)> score_func, unsigned &score)
{
  
  std::vector<Vertex> order;
  score=0;

  exclude.clear();
  
  while(exclude.size()<this->m_vertices.size() -2) {
    auto p = apply_every_pair(score_func,[&](unsigned x) {return x<=score;});
     
    score=std::max(score,p.first);
    exclude.insert(p.second);
    order.push_back(p.second);
    
    std::cout << "insert: " << p.second
	      << " | score: " << p.first
	      << " | max: " << score
	      <<" -> " << exclude.size() << "/" << m_vertices.size()-2
	      << std::endl;


  }
  std::cout << std::endl;
  exclude.clear();
  return order;
}

  

 

