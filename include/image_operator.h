#ifndef fpmbp_image_operator_h
#define fpmbp_image_operator_h

//------------------------------------------------------------------------------

#include <functional>	
#include "utils.h"

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------
  
class Image;
class GraphParticles;

//------------------------------------------------------------------------------

typedef std::function<void(float, float, const State&, float&, float&)> DisplacementFunction;
  
//------------------------------------------------------------------------------

// Performs image operations

class ImageOperator
{
public:
  ImageOperator(Image** img, Image** grad, Image** filt, int* ww, int* hh, Parameters& params, DisplacementFunction displacement_function);
  virtual ~ImageOperator();
  
  // Patch comparison
  float PatchCost(View view, int x, int y, const State& state, float threshold) const;
  float PixelCost(View target, View source, float x_source, float y_source, float x_target, float y_target, float r_center, float g_center, float b_center) const;
  
  // State validity
  bool IsStateValid(View view, int x, int y, const State& state) const;
  
  // Images and dimensions (pointer to that of the graph)
  Image** images;
  Image** gradients;
  Image** filtered;
  int* w;
  int* h;
  
  // Displacement function object
  DisplacementFunction displacement_function;
  
  // Parameters
  Parameters& parameters;
};

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif