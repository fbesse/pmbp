#ifndef STATE_H
#define STATE_H

//------------------------------------------------------------------------------

#include <vector>
#include <sstream>
#include <cmath>

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------

// Represent a point in the search space. The coordinate are contained in the
// member "data". It can also carry extra information, in the member "meta".
// This can be useful for example in the stereo case, where the coordinates
// are computed from the extra information which is easier to propagate.
  
class State {
 public:
  State(){};
  State(const State& state){
    data = state.data;
    meta = state.meta;
  }
  State(int size){
  	data.resize(size);	
    for(int i=0; i<size; ++i) data[i] = 0;
  };
  State(int size_data, int size_meta){
    data.resize(size_data);
    meta.resize(size_meta);  
    for(int i=0; i<size_data; ++i) data[i] = 0;
    for(int i=0; i<size_meta; ++i) meta[i] = 0;
  };
  State Copy() const{
    State state;
    state.data = data;
    state.meta = meta;
    return state;
  }

  virtual ~State(){};

  State& operator=(const State& other){
  	data = other.data;
    meta = other.meta;
  	return *this;
  }

  std::string Summary() const{
  	std::stringstream ss;
  	for(int i=0; i<data.size(); ++i){
  		ss << data[i] << " ";
  	}
    if(meta.size()!=0){
      ss << " || meta: ";
      for(int i=0; i<meta.size(); ++i){
        ss << meta[i] << " ";
      }
    }
  	return ss.str();
  }

  size_t Size(){
    return data.size();
  }
  
  static State EmptyState(){
    State state(0,0);
    return state;
  }

  std::vector<float> data;
  std::vector<float> meta;
};

//------------------------------------------------------------------------------
  
typedef std::vector<State> StateVector;
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif