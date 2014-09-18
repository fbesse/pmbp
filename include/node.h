#ifndef fpmbp_node_h
#define fpmbp_node_h

//------------------------------------------------------------------------------

#include "state.h"
#include "message.h"
#include "utils.h"
#include <vector>

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------
  
typedef std::pair<State, float> WeightedState;
  
//------------------------------------------------------------------------------
  
class Node{
public:
  Node() {}
  // Constructor with number of particles
  Node(int k) {
    particles.resize(k);
    foundations.resize(4);
  }
  
  void SetParticle(int k, const State& state, float value){
    particles[k].first = state;
    particles[k].second = value;
  }

  void SetParticleValue(int k, float value){
    particles[k].second = value;
  }
  
  State const* GetParticle(int k) const{
    return &particles[k].first;
  }

  float GetDisbelief(int k) const{
    return particles[k].second;
  }

  void InitialiseFoundation(){
    for(int i=0; i<foundations.size(); ++i){
      foundations[i] = Message(this);
      foundations[i].SetUniform();
    }
  }
  
  void SetFoundationValue(Direction direction, int k, float value){
    foundations[direction].Set(k, value);
  }
  
  float GetFoundationValue(Direction direction, int k) const{
    return foundations[direction].GetValue(k);
  }
  
  Message& GetFoundation(Direction direction){
    return foundations[direction];
  }
  
  void NormalizeFoundation(Direction direction){
    foundations[direction].Normalize();
  }
 
  State const* GetMinValueParticle() const{
    int idx = 0;
    float min_value = particles[0].second;
    
    for(int i=1; i<particles.size(); ++i){
      if(particles[i].second<min_value){
        min_value = particles[i].second;
        idx = i;
      }
    }
    
    return &particles[idx].first;
  }
  
  int GetMaxValueParticleIdx() const{
    int idx = 0;
    float max_value = particles[0].second;
    
    for(int i=1; i<particles.size(); ++i){
      if(particles[i].second>max_value){
        max_value = particles[i].second;
        idx = i;
      }
    }
    
    return idx;
  }
  
  float GetMinValue() const{
    float min_value = particles[0].second;
    
    for(int i=1; i<particles.size(); ++i){
      if(particles[i].second<min_value){
        min_value = particles[i].second;
      }
    }
    
    return min_value;
  }
  
  float GetMaxValue() const{
    float max_value = particles[0].second;
    
    for(int i=1; i<particles.size(); ++i){
      if(particles[i].second>max_value){
        max_value = particles[i].second;
      }
    }
    
    return max_value;
  }
  
  size_t Size() const { return particles.size(); }
  
  std::string Summary() const
  {
    std::stringstream summary;
    for(int i=0; i<particles.size(); ++i){
      summary << "value [" << particles[i].second << "] at " << particles[i].first.Summary() << std::endl;
    }
    
    return summary.str();
  }
  
private:
  std::vector<WeightedState> particles;   // Particle positions and disbelief values
  std::vector<Message> foundations;
};
  
//------------------------------------------------------------------------------
  
typedef Field<Node> NodeField;
  
//------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------

#endif