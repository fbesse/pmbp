//------------------------------------------------------------------------------

#include "graph_stereo.h"

//------------------------------------------------------------------------------

using namespace std;
using namespace std::placeholders;

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------

GraphStereo::GraphStereo(const Parameters& p) : GraphPmbp(p)
{
  data_dim = 3; // Disparity can be calculated from 3 coefficients
  meta_dim = 4; // The coefficients can be calculated from normal + depth
}

//------------------------------------------------------------------------------

GraphStereo::~GraphStereo()
{
  
}

//------------------------------------------------------------------------------

float GraphStereo::UnaryEnergy(View view, int x, int y, const State& state, float threshold) const
{
  if(!image_operator->IsStateValid(view, x, y, state))
    return parameters.infinity;
  
  return image_operator->PatchCost(view, x, y, state, threshold);
}

//------------------------------------------------------------------------------

float GraphStereo::PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const
{
  float d1 = state1.meta[3];
  float d2 = state2.meta[3];
  
  float dx = x2 - x1;
  float dy = y2 - y1;
  float dd = d2 - d1;
  
  float n1x = state1.meta[0];
  float n1y = state1.meta[1];
  float n1z = state1.meta[2];
  float n2x = state2.meta[0];
  float n2y = state2.meta[1];
  float n2z = state2.meta[2];
  
  float e1 = fabs( n1x*dx + n1y*dy + n1z*dd );
  float e2 = fabs( n2x*dx + n2y*dy + n2z*dd );
  
  return parameters.weight_pw*std::min(e1+e2,parameters.truncate_pw);
}
  
//------------------------------------------------------------------------------
  
State GraphStereo::GetStateFromParametrization(float x, float y, float nx, float ny, float nz, float d) const{
  
  State state(data_dim, meta_dim);
  
  state.data[0] = - 1.f * nx / nz;
  state.data[1] = - 1.f * ny / nz;
  state.data[2] = (nx * x + ny * y + nz * d) / nz;
  
  state.meta[0] = nx;
  state.meta[1] = ny;
  state.meta[2] = nz;
  state.meta[3] = d;
  
  return state;
}
  
//------------------------------------------------------------------------------
  
State GraphStereo::GetRandomState(View view, int x, int y) const
{
  // Use the maximum dimension unless max_motion is set to 0
  float max_motion = (parameters.max_motion==0.f?std::max(w[view], h[view]):parameters.max_motion);
  
  // Random depth
  float d = max_motion*Random::DrawUniform(-1.f, 1.f);
  
  // Random normal vector
  float nx = Random::DrawUniform(-1.f, 1.f);
  float ny = Random::DrawUniform(-1.f, 1.f);
  float nz = Random::DrawUniform(-1.f, 1.f);
  
  // Normalize normal vector according to image dimensions
  nx /= w[view];
  ny /= h[view];
  nz /= max_motion;
  
  // Build unit vector
  float length = sqrt(nx*nx + ny*ny + nz*nz);
  nx /= length;
  ny /= length;
  nz /= length;
  
  return GetStateFromParametrization(x, y, nx, ny, nz, d);
}
  
//------------------------------------------------------------------------------
  
State GraphStereo::GetRandomStateAround(View view, int x, int y, const State& current, float ratio) const
{
  // Use the maximum dimension unless max_motion is set to 0
  float max_motion = (parameters.max_motion==0.f?std::max(w[view], h[view]):parameters.max_motion);
  
  float max_delta_z = max_motion*ratio;
  float max_delta_angle = ratio;
  
  // Random amount of change of point depth
  float delta_d = Random::DrawUniform(-1.f, 1.f) * max_delta_z;
  
  // Random amount of change of normal vector
  float delta_nx = Random::DrawUniform(-1.f, 1.f) * max_delta_angle;
  float delta_ny = Random::DrawUniform(-1.f, 1.f) * max_delta_angle;
  float delta_nz = Random::DrawUniform(-1.f, 1.f) * max_delta_angle;
  
  float nx = current.meta[0] + delta_nx;
  float ny = current.meta[1] + delta_ny;
  float nz = current.meta[2] + delta_nz;
  float d = current.meta[3] + delta_d;
  
  // Build unit vector
  float length = sqrt(nx*nx + ny*ny + nz*nz);
  nx /= length;
  ny /= length;
  nz /= length;
  
  return GetStateFromParametrization(x, y, nx, ny, nz, d);
}

//------------------------------------------------------------------------------
  
State GraphStereo::GetStateFromNeighbour(View view, int x, int y, int nx, int ny) const
{
  // Retrieve best state of the neighbour
  return GetMinDisbeliefState(view, nx, ny)->Copy();
}
  
//------------------------------------------------------------------------------
  
void GraphStereo::GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const
{
  dx = state.data[0] * x + state.data[1] * y + state.data[2];
  dy = 0;
}

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------