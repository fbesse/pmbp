#include "image.h"

#include <vector>
#include <algorithm>
#include <iostream>

#include <CImg.h>
using namespace cimg_library;

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------
  
Image::Image(){
  data = 0;
}
  
//------------------------------------------------------------------------------

Image::Image(int w, int h){
  data = new int[w*h];
  width = w;
  height = h;
}
  
//------------------------------------------------------------------------------
  
Image::~Image(){
  delete [] data;
}

//------------------------------------------------------------------------------
  
float Image::GetRealGradientX(int x, int y) const{
	int grey_center = Grey(GetGridPixel(x, y));
	int grey_left = grey_center;
	int grey_right = grey_center;

  if(x!=0){
    grey_left = Grey(GetGridPixel(x-1, y));
  }

	if(x!=width-1){
		grey_right = Grey(GetGridPixel(x+1, y));
	}

	return abs(grey_left-grey_right);
}

//------------------------------------------------------------------------------
  
float Image::GetRealGradientY(int x, int y) const{
	int grey_center = Grey(GetGridPixel(x, y));
	int grey_top = grey_center;
	int grey_bottom = grey_center;

  if(y!=0){
    grey_top = Grey(GetGridPixel(x, y-1));
  }

	if(y!=height-1){
		grey_bottom = Grey(GetGridPixel(x, y+1));
	}

	return abs(grey_top-grey_bottom);
}

//------------------------------------------------------------------------------
  
float Image::GetRealGradient(int x, int y) const{
	float dx = GetRealGradientX(x, y);
	float dy = GetRealGradientY(x, y);
	return sqrt(dx*dx+dy*dy);
}

//------------------------------------------------------------------------------
  
Image* Image::GetGradient(){

	Image* gradient = new Image(width, height);

	for(int i=0; i<width; ++i){
		for(int j=0; j<height; ++j){

			int grey_center = Grey(GetGridPixel(i, j));

			int grey_left = grey_center;
			int grey_right = grey_center;
			int grey_top = grey_center;
			int grey_bottom = grey_center;

			if(i!=0){
				grey_left = Grey(GetGridPixel(i-1, j));
			}

			if(i!=width-1){
				grey_right = Grey(GetGridPixel(i+1, j));
			}

			if(j!=0){
				grey_top = Grey(GetGridPixel(i, j-1));
			}

			if(j!=height-1){
				grey_bottom = Grey(GetGridPixel(i, j+1));
			}

			float delta_x = (float)grey_right - (float)grey_left;
			float delta_y = (float)grey_bottom - (float)grey_top;
      
      // Calculate gradient magnitude
      
      float mag = sqrt(delta_x*delta_x+delta_y*delta_y);

      int int_mag = int(mag);
      
			int_mag = std::min(int_mag, 255);
			int_mag = std::max(int_mag, 0);

      int img_value = EncodeColour(int_mag, int_mag, int_mag, 255);
    
			gradient->SetGridPixel(i, j, img_value);

		}
	}

	return gradient;
}

//------------------------------------------------------------------------------
  
Image* Image::MedianFilter(int s){
  Image* filt = new Image(width, height);

	for(int i=0; i<width; ++i){
		for(int j=0; j<height; ++j){

			std::vector<int> r;
			std::vector<int> g;
			std::vector<int> b;

			for(int u=-s/2; u<=s/2; ++u){
				for(int v=-s/2; v<=s/2; ++v){

					if(!IsInside(i+u, j+v))
						continue;

					int color = GetGridPixel(i+u, j+v);

					r.push_back(Red(color));
					g.push_back(Green(color));
					b.push_back(Blue(color));

				}
			}

			std::sort(r.begin(), r.end());
			std::sort(g.begin(), g.end());
			std::sort(b.begin(), b.end());

			int color = EncodeColour(r[(r.size()-1)/2], g[(g.size()-1)/2], b[(b.size()-1)/2], 255);

			filt->SetGridPixel(i, j, color);
		}
	}

	return filt;

}

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------