#include "image_reader_cimg.h"

#include <CImg.h>
#include "image.h"

using namespace cimg_library;

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------
  
ImageReaderCImg::ImageReaderCImg(){
  
}
  
//------------------------------------------------------------------------------

ImageReaderCImg::~ImageReaderCImg(){
  
}
  
//------------------------------------------------------------------------------
 
Image* ImageReaderCImg::load(const std::string& filename){
  CImg<unsigned char> cimage(filename.c_str()); 
  
  Image* image = new Image(cimage.width(), cimage.height());
  
  for(int i=0; i<image->width; ++i){
    for(int j=0; j<image->height; ++j){
      
      int colour = 0;
      
      if(cimage.spectrum()==1){
        unsigned char grey = cimage(i, j, 0, 0);
        
        colour = Image::EncodeColour(grey, grey, grey, 255);
        
      }else{
        unsigned char red = cimage(i, j, 0, 0);
        unsigned char green = cimage(i, j, 0, 1);
        unsigned char blue = cimage(i, j, 0, 2);
        
        colour = Image::EncodeColour(red, green, blue, 255);
      }
      
      image->SetGridPixel(i, j, colour);
      
    }
  }
  
  return image;
}
  
//------------------------------------------------------------------------------
  
void ImageReaderCImg::save(Image* image, const std::string& filename){
  CImg<unsigned char> cimage(image->width, image->height, 1, 3);
  
  for(int i=0; i<image->width; ++i){
    for(int j=0; j<image->height; ++j){
      unsigned char red = (unsigned char)Image::Red(image->GetGridPixel(i, j));
      unsigned char green = (unsigned char)Image::Green(image->GetGridPixel(i, j));
      unsigned char blue = (unsigned char)Image::Blue(image->GetGridPixel(i, j));
      
      cimage(i, j, 0, 0) = red;
      cimage(i, j, 0, 1) = green;
      cimage(i, j, 0, 2) = blue;
    }
  }
  
  cimage.save_png(filename.c_str());
}
  
//------------------------------------------------------------------------------
  
CImg<unsigned char> ImageToCImg(const Image& image){
  CImg<unsigned char> cimage(image.width, image.height, 1, 3);
  
  for(int i=0; i<image.width; ++i){
    for(int j=0; j<image.height; ++j){
      unsigned char red = (unsigned char)Image::Red(image.GetGridPixel(i, j));
      unsigned char green = (unsigned char)Image::Green(image.GetGridPixel(i, j));
      unsigned char blue = (unsigned char)Image::Blue(image.GetGridPixel(i, j));
      
      cimage(i, j, 0, 0) = red;
      cimage(i, j, 0, 1) = green;
      cimage(i, j, 0, 2) = blue;
    }
  }
  
  return cimage;
}
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------