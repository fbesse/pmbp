//------------------------------------------------------------------------------

#include "graph_discrete.h"

//------------------------------------------------------------------------------

using namespace std;
using namespace std::placeholders;

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------

GraphDiscrete::GraphDiscrete(const Parameters& p) : GraphParticles(p)
{
  data_dim = 2; // 2d displacement
  meta_dim = 0;
  float max_motion = (parameters.max_motion==0.f?std::max(w[kOne], h[kTwo]):parameters.max_motion);
  int side = (2*max_motion+1)/parameters.discrete_step;
  parameters.n_particles = side*side;
  
  std::cout << "Discrete BP, setting particle number: " << parameters.n_particles << std::endl;
}

//------------------------------------------------------------------------------

GraphDiscrete::~GraphDiscrete()
{
  
}

//------------------------------------------------------------------------------

void GraphDiscrete::InitialiseNode(View view, int x, int y)
{
  float max_motion = (parameters.max_motion==0.f?std::max(w[kOne], h[kTwo]):parameters.max_motion);
  int count = 0;
  // For all possible discrete state
  for(int v=-max_motion; v<=max_motion; v+=parameters.discrete_step){
    for(int u=-max_motion; u<=max_motion; u+=parameters.discrete_step){
      State state = GetFixedState(u, v);
      float unary = UnaryEnergy(view, x, y, state, parameters.infinity);
      nodes[view].Get(x, y)->SetParticle(count, state, unary);
      ++count;
    }
  }
}

//------------------------------------------------------------------------------
  
void GraphDiscrete::Update(View view, int x, int y)
{
  // Discrete case, there is no particle to update as our particles are fixed
}

//------------------------------------------------------------------------------

State GraphDiscrete::GetFixedState(float dx, float dy)
{
  State state(data_dim, meta_dim);
  state.data[0] = dx;
  state.data[1] = dy;
  return state;
}

//------------------------------------------------------------------------------

float GraphDiscrete::UnaryEnergy(View view, int x, int y, const State& state, float threshold) const
{
  if(!image_operator->IsStateValid(view, x, y, state))
    return parameters.infinity;
  
  return image_operator->PatchCost(view, x, y, state, threshold);
}

//------------------------------------------------------------------------------

float GraphDiscrete::PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const
{
  // Quadratic truncated pairwise term
  float dx = state1.data[0]-state2.data[0];
  float dy = state1.data[1]-state2.data[1];
  return parameters.weight_pw*std::min((dx*dx+dy*dy),parameters.truncate_pw);
}
  
//------------------------------------------------------------------------------

void GraphDiscrete::GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const
{
  dx = state.data[0];
  dy = state.data[1];
}

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------