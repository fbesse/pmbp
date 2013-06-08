//
//  graph_discrete.h
//  fpmbp
//
//  Created by Frederic Besse on 07/09/2012.
//  Copyright (c) 2012 Frederic Besse. All rights reserved.
//

#ifndef fpmbp_graph_discrete_h
#define fpmbp_graph_discrete_h

//------------------------------------------------------------------------------

#include "utils.h"
#include "node.h"
#include "graph_particles.h"
#include <map>
#include <set>

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------

class Image;
class Flo;

//------------------------------------------------------------------------------

class GraphDiscrete : public GraphParticles{
  
public:
  GraphDiscrete(const Parameters& p);
  virtual ~GraphDiscrete();
  
  // Initialisation
  virtual void InitialiseNode(View view, int x, int y);
  
  // Main node-wise operations
  virtual void Update(View view, int x, int y);
  
  // Particle generation
  State GetFixedState(float dx, float dy);
  
  // Energy evaluation
  virtual float UnaryEnergy(View view, int x, int y, const State& state, float threshold) const;
  virtual float PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const;
  
  // Displacement
  void GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const;
  
  // Import/export
  virtual char GetTag(){ return 'A'; }
};

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif
