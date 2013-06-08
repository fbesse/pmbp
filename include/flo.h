#ifndef FLO_H
#define FLO_H

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------

class Flo {
 public:
  Flo(int w, int h){
    width = w;
    height = h;
    data = new float[width*height*2];
  }
  ~Flo(){};
  
  void SetFlow(int u, int v, float fu, float fv){
    data[2*(u*width+v)] = fu;
    data[2*(u*width+v)+1] = fv;
  }

  void GetFlow(int u, int v, float &fu, float &fv){
    fu = data[2*(u*width+v)];
    fv= data[2*(u*width+v)+1];
  }

  int width;
  int height;
  float* data;
};
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif