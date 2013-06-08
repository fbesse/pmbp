#ifndef fpmbp_graph_stereo_h
#define fpmbp_graph_stereo_h

//------------------------------------------------------------------------------

#include "graph_pmbp.h"

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------

// Performs stereo matching

class GraphStereo : public GraphPmbp
{
public:
  GraphStereo(const Parameters& p);
  virtual ~GraphStereo();
    
  // Energy evaluation
  virtual float UnaryEnergy(View view, int x, int y, const State& state, float threshold) const;
  virtual float PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const;
  
  // Candidate state generation
  State GetStateFromParametrization(float x, float y, float nx, float ny, float nz, float d) const;
  virtual State GetRandomState(View view, int x, int y) const;
  virtual State GetRandomStateAround(View view, int x, int y, const State& current, float ratio) const;
  virtual State GetStateFromNeighbour(View view, int x, int y, int nx, int ny) const;
  
  // Displacement
  void GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const;
  
  // Import/export
  virtual char GetTag(){ return 'C'; }
};

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif