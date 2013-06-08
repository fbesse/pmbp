#ifndef fpmbp_graph_2d_flow_h
#define fpmbp_graph_2d_flow_h

//------------------------------------------------------------------------------

#include "graph_pmbp.h"

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------

// Performs 2d patch matching
  
class Graph2DFlow : public GraphPmbp
{
public:
  Graph2DFlow(const Parameters& p);
  virtual ~Graph2DFlow();
  
  // Energy evaluation
  virtual float UnaryEnergy(View view, int x, int y, const State& state, float threshold) const;
  virtual float PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const;
  
  // Candidate state generation
  virtual State GetRandomState(View view, int x, int y) const;
  virtual State GetRandomStateAround(View view, int x, int y, const State& current, float ratio) const;
  virtual State GetStateFromNeighbour(View view, int x, int y, int nx, int ny) const;
  
  // Displacement
  void GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const;
  
  // Import/export
  virtual char GetTag(){ return 'B'; }
};

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif