#ifndef FLO_IO_H
#define FLO_IO_H

//------------------------------------------------------------------------------

#include "flo.h"
#include <string>
#include <fstream>
#include <iostream>

//------------------------------------------------------------------------------

namespace pmbp{
  
// Import/export of .flo files
  
class FloIO {
 public:
  // Save flow to file
  static void Save(Flo* flo, std::string filename){
    
    std::string TAG_STRING("PIEH");

    std::fstream fs(filename.c_str(), std::ios::out | std::ios::binary);

    fs.write((char*)&TAG_STRING[0], 4);
    fs.write((char*)&(flo->width), sizeof(flo->width));
    fs.write((char*)&(flo->height), sizeof(flo->height));

    fs.write((char*)&(flo->data[0]), flo->width*flo->height*2*4);

    fs.close();
  }

  // Read flow from file
  static Flo* Load(std::string filename){
    std::string TAG_STRING("PIEH");

    std::fstream fs(filename.c_str(), std::ios::in | std::ios::binary);

    char tag[4];
    fs.read(tag, 4);

    int width;
    int height;
    fs.read((char*)&width, 4);
    fs.read((char*)&height, 4);

    std::cout << "Loading flow with size [" << width << "," << height << "]" << std::endl;

    Flo* flo = new Flo(width,height);
    fs.read((char*)&(flo->data[0]), flo->width*flo->height*2*4);

    fs.close();

    return flo;
  }

};
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif