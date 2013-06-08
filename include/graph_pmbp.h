#ifndef fpmbp_graph_pmbp_h
#define fpmbp_graph_pmbp_h

//------------------------------------------------------------------------------

#include "graph_particles.h"

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------

// Base PMBP implementation, containing resampling interface
  
class GraphPmbp : public GraphParticles
{
public:
  GraphPmbp(const Parameters& p);
  virtual ~GraphPmbp();
  
  // Initialisation
  virtual void InitialiseNode(View view, int x, int y);
  
  // Node-wise operations
  virtual void Update(View view, int x, int y);
  virtual void Propagate(View view, int x, int y);
  virtual void Randomise(View view, int x, int y);
  
  // Candidate state generation
  virtual State GetRandomState(View view, int x, int y) const = 0;
  virtual State GetRandomStateAround(View view, int x, int y, const State& current, float ratio) const = 0;
  virtual State GetStateFromNeighbour(View view, int x, int y, int nx, int ny) const = 0;
};

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif