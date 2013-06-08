//
//  graph_particles.h
//  fpmbp
//
//  Created by Frederic Besse on 07/09/2012.
//  Copyright (c) 2012 Frederic Besse. All rights reserved.
//

#ifndef fpmbp_graph_particles_h
#define fpmbp_graph_particles_h

//------------------------------------------------------------------------------

#include "utils.h"
#include "node.h"
#include "image_operator.h"
#include <map>
#include <set>

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------
  
class Image;
class Flo;
  
//------------------------------------------------------------------------------
  
// Base implementation of particle BP, without particle resampling
// Contains all the mechanisms for the caching of message foundations
  
class GraphParticles{

public:  
  GraphParticles(const Parameters& p);
  virtual ~GraphParticles();
  
  // Initialisation
  void InitialiseImages(Image* one, Image* two);
  void InitialiseFields(View view);
  void InitialiseFields();
  void InitialiseNodes(View view);
  void InitialiseNodes();
  virtual void InitialiseNode(View view, int x, int y) = 0;
  
  // Main methods
  void Solve();
  void Iterate(int it);
  void IterateView(int it, View view);
  void ResetProcessed();
  
  // Main node-wise operations
  virtual void Update(View view, int x, int y) = 0;
  void Cache(View view, int x, int y);
  
  // Particle operations
  void ProposeCandidate(View view, int x, int y, const State& particle, bool force=false);
  
  // Energy evaluation
  virtual float UnaryEnergy(View view, int x, int y, const State& state, float threshold) const = 0;
  virtual float PairwiseEnergy(View view, int x1, int y1, const State& state1, int x2, int y2, const State& state2) const = 0;
    
  // Disbelief & state operation
  float EvaluateDisbelief(View view, int x, int y, const State& state, bool early_termination=false) const;
  float EvaluateMessage(View view, int from_x, int from_y, int to_x, int to_y, const State& state) const;
  State const * GetMinDisbeliefState(View view, int x, int y) const;
  float GetMaxDisbelief(View view, int x, int y) const;
  void UpdateCurrentDisbelief(View view, int x, int y);
  
  // Displacement
  virtual void GetDisplacement(float x, float y, const State& state, float& dx, float& dy) const = 0;
  
  // Outputs
  virtual Image OutputMotionField(View view) const;
  virtual Image OutputReconstruction(View view) const;
  virtual Flo* ExportFlo(View view) const;
  Image OutputUnaryEnergy(View view, float& energy) const;
  Image OutputPairwiseEnergy(View view, float& energy) const;
  
  // Import/export
  virtual void ImportFields(const std::string& filename);
  virtual void ExportFields(const std::string& filename);
  virtual char GetTag() = 0;
  
  // Utilities
  void GetDirections(int k, View view, int& i_first, int& i_last, int& j_first, int& j_last, int& i_incr, int& j_incr) const;
  
  // Debug
  void Inspect();
  
protected:
  // Number of dimensions of the state space (data and meta)
  int data_dim;
  int meta_dim;
  
  // Images and dimensions
  Image* images[2];
  Image* gradients[2];
  Image* filtered[2];
  int w[2];
  int h[2];
  
  // Image operator and displacement function object
  ImageOperator* image_operator;
  
  // Fields
  NodeField nodes[2];
  Mask processed[2];
  Mask propagated[2];
  
  // Parameters
  Parameters parameters;
};
  
//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
