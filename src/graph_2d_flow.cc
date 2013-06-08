//------------------------------------------------------------------------------

#include "graph_2d_flow.h"

//------------------------------------------------------------------------------

using namespace std;
using namespace std::placeholders;

//------------------------------------------------------------------------------

namespace pmbp{

//------------------------------------------------------------------------------

Graph2DFlow::Graph2DFlow(const Parameters& p) : GraphPmbp(p)
{
  data_dim = 2; // 2d displacement
  meta_dim = 0;
}

//------------------------------------------------------------------------------

Graph2DFlow::~Graph2DFlow()
{
  
}
  
//------------------------------------------------------------------------------

float Graph2DFlow::UnaryEnergy(View view, int x, int y, const State& state, float threshold) const
{
  if(!image_operator->IsStateValid(view, x, y, state))
    return parameters.infinity;
  
  return image_operator->PatchCost(view, x, y, state, threshold);
}

//------------------------------------------------------------------------------

float Graph2DFlow::PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const
{
  // Quadratic truncated pairwise term
  float dx = state1.data[0]-state2.data[0];
  float dy = state1.data[1]-state2.data[1];
  return parameters.weight_pw*std::min((dx*dx+dy*dy),parameters.truncate_pw);
}

//------------------------------------------------------------------------------
  
State Graph2DFlow::GetRandomState(View view, int x, int y) const
{
  State state(data_dim, meta_dim);
  
  // Use the maximum dimension unless max_motion is set to 0
  float max_motion = (parameters.max_motion==0.f?std::max(w[view], h[view]):parameters.max_motion);
  
  // Generate random 2D offset until the motion is within the constraints
  do{
    state.data[0] = max_motion*Random::DrawUniform(-1.f, 1.f);
    state.data[1] = max_motion*Random::DrawUniform(-1.f, 1.f);
  } while(!image_operator->IsStateValid(view, x, y, state));
  
  return state;
}
  
//------------------------------------------------------------------------------
  
State Graph2DFlow::GetRandomStateAround(View view, int x, int y, const State& current, float ratio) const
{
  State state = current;
  
  // Use the maximum dimension unless max_motion is set to 0
  float max_motion = (parameters.max_motion==0.f?std::max(w[view], h[view]):parameters.max_motion);
  
  // Generate random 2D offset until the motion is within the constraints
  state.data[0] = current.data[0]+ratio*max_motion*Random::DrawUniform(-1.f, 1.f);
  state.data[1] = current.data[1]+ratio*max_motion*Random::DrawUniform(-1.f, 1.f);
  
  if(!image_operator->IsStateValid(view, x, y, state)){
    state = current;
  }
  
  return state;
}

//------------------------------------------------------------------------------
  
State Graph2DFlow::GetStateFromNeighbour(View view, int x, int y, int nx, int ny) const
{
  // Retrieve best state of the neighbour
  return GetMinDisbeliefState(view, nx, ny)->Copy();
}
  
//------------------------------------------------------------------------------
  
void Graph2DFlow::GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const
{
  dx = state.data[0];
  dy = state.data[1];
}

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------