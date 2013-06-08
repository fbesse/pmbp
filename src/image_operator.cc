#include "image_operator.h"
#include "image.h"
#include "graph_particles.h"

//------------------------------------------------------------------------------

namespace pmbp {
  
//------------------------------------------------------------------------------
  
ImageOperator::ImageOperator(Image** img, Image** grad, Image** filt, int* ww, int* hh, Parameters& params, DisplacementFunction f) :
  images(img), gradients(grad), filtered(filt), w(ww), h(hh), parameters(params), displacement_function(f){

}
  
//------------------------------------------------------------------------------
  
ImageOperator::~ImageOperator(){
  
}

//------------------------------------------------------------------------------
  
float ImageOperator::PatchCost(View view, int x, int y, const State& state, float threshold) const
{
  float error(0);
  
  View target = view;
  View source = OtherView(target);
  
  // Center pixel
  float xc_target = x;
  float yc_target = y;
  
  // Patch boundaries
  float start_x = std::max(xc_target - parameters.patch_size, 0.f);
  float start_y = std::max(yc_target - parameters.patch_size, 0.f);
  float end_x = std::min((float)(xc_target + parameters.patch_size), (float)(w[target]-1));
  float end_y = std::min((float)(yc_target + parameters.patch_size), (float)(h[target]-1));
  
  // Center color for AWS
	int center_colour = filtered[view]->GetGridPixel(x, y);
	float r_center = Image::Red(center_colour);
	float g_center = Image::Green(center_colour);
	float b_center = Image::Blue(center_colour);
  
  for(int y_target = start_y; y_target <= end_y; ++y_target){
    for(int x_target = start_x; x_target <= end_x; ++x_target){
      
      float d_x, d_y;
      displacement_function(x_target, y_target, state, d_x, d_y);
      
      // Get source coordinate
      float x_source = x_target + d_x;
      float y_source = y_target + d_y;
      
      error += PixelCost(target, source, x_source, y_source, x_target, y_target, r_center, g_center, b_center);
    }
    
    // Early termination, must pass unary minus message sum as the message sum
    // will be added to the unary
    if(error > threshold){
      return parameters.infinity;
    }
    
  }
  
  return error;
}

//------------------------------------------------------------------------------
  
float ImageOperator::PixelCost(View target, View source, float x_source, float y_source, float x_target, float y_target, float r_center, float g_center, float b_center) const
{
  float error(0.f);
  
  if(images[source]->IsInside(x_source, y_source)){
    
    // Get target colour
    int target_colour = images[target]->GetGridPixel(x_target, y_target);
    unsigned char r_t, g_t, b_t;
    Image::DecodeColour(target_colour, r_t, g_t, b_t);
    
    // Get target gradient colour
    int target_gradient_colour = gradients[target]->GetGridPixel(x_target, y_target);
    float dr_t = Image::Red(target_gradient_colour);
    
    // Get source colour
    float r_s, g_s, b_s;
    images[source]->GetInterpolatedPixel(x_source, y_source, r_s, g_s, b_s);
    
    // Get source gradient
    float dr_s, dg_s, db_s;
    gradients[source]->GetInterpolatedPixel(x_source, y_source, dr_s, dg_s, db_s);
    
    // Difference
    float diff_colour = (fabs(float(r_t)-float(r_s))+fabs(float(g_t)-float(g_s))+fabs(float(b_t)-float(b_s)))/3.f;
    float diff_gradient = fabs(float(dr_t)-float(dr_s));
    
    // Adaptive support weight
    float filt_r, filt_g, filt_b;
    filtered[target]->GetInterpolatedPixel(x_target, y_target, filt_r, filt_g, filt_b);
    
    float diff_asw = (fabs(r_center-filt_r)+fabs(g_center-filt_g)+fabs(b_center-filt_b));
    float w = exp(-(diff_asw)/parameters.asw);
    
    diff_colour = std::min(diff_colour, parameters.tau1);
    diff_gradient = std::min(diff_gradient, parameters.tau2);
    
    error = w*((1.f-parameters.alpha)*diff_colour + parameters.alpha*diff_gradient);
  }
  else{
   
   float maxmatchcosts = (1.f - parameters.alpha) * parameters.tau1 + parameters.alpha * parameters.tau2;
   float bordercosts = maxmatchcosts * parameters.border;
   
   // Adaptive support weight
   float filt_r, filt_g, filt_b;
   filtered[target]->GetInterpolatedPixel(x_target, y_target, filt_r, filt_g, filt_b);
   
   float diff_asw = (fabs(r_center-filt_r)+fabs(g_center-filt_g)+fabs(b_center-filt_b));
   float w = exp(-(diff_asw)/parameters.asw);
   
   error = w*(bordercosts);
  }
  
  return error;
}
  
//------------------------------------------------------------------------------
  
bool ImageOperator::IsStateValid(View view, int x, int y, const State& state) const
{
  float dx, dy;
  displacement_function(x, y, state, dx, dy);
  
  // Check that the displacement is not more than the max motion
  if(parameters.max_motion!=0.f && dx*dx+dy*dy>parameters.max_motion*parameters.max_motion)
    return false;
  
  // Check that we are still in the image
  if(!images[view]->IsInside(x+dx, y+dy))
    return false;
  
  return true;
}
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------
