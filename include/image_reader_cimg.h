#ifndef IMAGE_READER_CIMG_H
#define IMAGE_READER_CIMG_H

//------------------------------------------------------------------------------

#include "image_reader.h"
#include <CImg.h>
using namespace cimg_library;

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------

class Image;
  
//------------------------------------------------------------------------------
  
class ImageReaderCImg : public ImageReader {
 public:
  ImageReaderCImg();
  ~ImageReaderCImg();
  
  Image* load(const std::string& filename);
  void save(Image* image, const std::string& filename);
};
  
//------------------------------------------------------------------------------
  
CImg<unsigned char> ImageToCImg(const Image& image);
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif
