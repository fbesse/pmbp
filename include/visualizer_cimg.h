#ifndef fpmbp_visualizer_cimg_h
#define fpmbp_visualizer_cimg_h

//------------------------------------------------------------------------------

#include "visualizer.h"
#include "image_reader_cimg.h"
#include <CImg.h>
using namespace cimg_library;

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------
  
class VisualizerCImg : public Visualizer{
public:
  VisualizerCImg(const std::string& title) : Visualizer(title) {}
  VisualizerCImg(const std::string& title, const Image& image) : Visualizer(title) {Show(image); }
  virtual~ VisualizerCImg(){}
  
  virtual void Show(const Image& image){
    CImg<unsigned char> cimg = ImageToCImg(image);
    m_display.display(cimg);
  }
  
private:
  CImgDisplay m_display;
};
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif