#ifndef fpmbp_visualizer_h
#define fpmbp_visualizer_h

//------------------------------------------------------------------------------

#include "image.h"

//------------------------------------------------------------------------------

namespace pmbp{
 
//------------------------------------------------------------------------------
  
class Visualizer{
public:
  Visualizer(const std::string& title) : m_title(title) {}
  virtual ~Visualizer(){};
  
  virtual void Show(const Image& image) = 0;
  
protected:
  std::string m_title;
  
};
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif