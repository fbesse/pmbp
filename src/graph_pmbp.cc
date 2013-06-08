//------------------------------------------------------------------------------

#include "graph_pmbp.h"
#include <iostream>
#include <algorithm>

using namespace std;

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------

GraphPmbp::GraphPmbp(const Parameters& p) : GraphParticles(p)
{

}

//------------------------------------------------------------------------------

GraphPmbp::~GraphPmbp()
{
  
}
  
//------------------------------------------------------------------------------

void GraphPmbp::InitialiseNode(View view, int x, int y)
{
  // For each particle, initialise randomly
  for(int k=0; k<parameters.n_particles; ++k){
    State state = GetRandomState(view, x, y);
    nodes[view].Get(x, y)->SetParticle(k, state, 0);
  }
}

//------------------------------------------------------------------------------
  
void GraphPmbp::Update(View view, int x, int y)
{
  // Generate candidate particles and refine the particle set of the node
  
  // First propagate from all the neighbours
  Propagate(view, x, y);
  
  // Then perform random search
  Randomise(view, x, y);
}
  
//------------------------------------------------------------------------------
  
void GraphPmbp::Propagate(View view, int x, int y)
{
  
  if(x>0 && processed[view].Get(x-1, y) ){
    State state = GetStateFromNeighbour(view, x, y, x-1, y);
    ProposeCandidate(view, x, y, state);
  }
  
  if(y>0 && processed[view].Get(x, y-1) ){
    State state = GetStateFromNeighbour(view, x, y, x, y-1);
    ProposeCandidate(view, x, y, state);
  }
  
  if(x<w[view]-1 && processed[view].Get(x+1, y) ){
    State state = GetStateFromNeighbour(view, x, y, x+1, y);
    ProposeCandidate(view, x, y, state);
  }
    
  if(y<h[view]-1 && processed[view].Get(x, y+1) ){
    State state = GetStateFromNeighbour(view, x, y, x, y+1);
    ProposeCandidate(view, x, y, state);
  }
}

//------------------------------------------------------------------------------
  
void GraphPmbp::Randomise(View view, int x, int y)
{
  Node* node = nodes[view].Get(x, y, 0);
  size_t size = node->Size();
  for(int k=0; k<size; ++k){
    float ratio = 0.1f;
    // Here we keep the current fixed from last iteration
    // Another option is to get the current best inside the following while
    State const* current = node->GetParticle(k);
    while(ratio > 0.001f){
      State state = GetRandomStateAround(view, x, y, *current, ratio);
      ProposeCandidate(view, x, y, state);
      
      ratio /= 2.f;
    }
  }
}

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------