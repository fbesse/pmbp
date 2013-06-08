#ifndef IMAGE_H
#define IMAGE_H

//------------------------------------------------------------------------------

#include <cmath>
#include <iostream>

//------------------------------------------------------------------------------

namespace pmbp{
  
//------------------------------------------------------------------------------
  
class Image{
 public:
  Image();
  Image(int w, int h);
  ~Image();

  Image* GetGradient();

  // Pixel access
  int GetGridPixel(int x, int y) const;
  float GetGreyGridPixel(int x, int y) const;
  void GetTransformedSubPixel(float x, float y, float rotation, float scale, float center_x, float center_y, float& r, float& g, float& b) const;
  void SetGridPixel(int x, int y, int colour);
  void SetGridPixelGrey(int x, int y, unsigned char value);
  void GetTransformedVector(float dx, float dy, float rotation, float& rdx, float& rdy) const;
  void GetTransformedPixelCoordinate(float x, float y, float center_x, float center_y, float rotation, float scale, float& out_x, float& out_y) const;
  void GetInterpolatedPixel(float x, float y, float& r, float& g, float& b, bool disp=false) const;
  void GetHorizontallyInterpolatedPixel(float x, float y, float& r, float& g, float& b, bool disp=false) const;
  Image* MedianFilter(int s);

  float GetRealGradientX(int x, int y) const;
  float GetRealGradientY(int x, int y) const;
  float GetRealGradient(int x, int y) const;
  
  // Dimensions & boundary check
  int width;
  int height;
  bool IsInside(float x, float y) const;
  
  // Pixel data
  int* data;
  
  // Static method to access channel data from an int
  static int EncodeColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
  static void DecodeColour(int color, unsigned char& r, unsigned char& g, unsigned char& b);
  static unsigned char Red(int colour);
  static unsigned char Green(int colour);
  static unsigned char Blue(int colour);
  static unsigned char Alpha(int colour);
  static unsigned char Grey(int colour);
};
 
//------------------------------------------------------------------------------
  
inline
float Image::GetGreyGridPixel(int x, int y) const
{
  int pixel = GetGridPixel(x, y);
  
  float r = Red(pixel);
  float g = Green(pixel);
  float b = Blue(pixel);
  
  return (r+g+b)/3.f;
}
  
//------------------------------------------------------------------------------
  
inline
int Image::GetGridPixel(int x, int y) const
{
  return data[y*width+x];
}
  
//------------------------------------------------------------------------------
  
inline
void Image::GetTransformedSubPixel(float x, float y, float rotation, float scale, float center_x, float center_y, float& r, float& g, float& b) const
{
    float transformed_x = 0;
    float transformed_y = 0;

    GetTransformedPixelCoordinate(x, y, center_x, center_y, rotation, scale, transformed_x, transformed_y); 
    GetInterpolatedPixel(transformed_x, transformed_y, r, g, b);
}

//------------------------------------------------------------------------------
  
inline
void Image::SetGridPixel(int x, int y, int colour)
{
  data[y*width+x] = colour;
}

//------------------------------------------------------------------------------
  
inline
void Image::SetGridPixelGrey(int x, int y, unsigned char value)
{
  int colour = 0xFF000000 | ((unsigned int)value) << 16 | ((unsigned int)value) << 8 | ((unsigned int)value);
  SetGridPixel(x, y, colour);
}

//------------------------------------------------------------------------------
  
inline
void Image::GetTransformedVector(float dx, float dy, float rotation, float& rdx, float& rdy) const
{

  // Create rotation elements
  float cos_theta = cos(rotation);
  float sin_theta = sin(rotation);

  // Rotate vector
  rdx = dx*cos_theta-dy*sin_theta;
  rdy = dx*sin_theta+dy*cos_theta;
}

//------------------------------------------------------------------------------
  
inline
void Image::GetTransformedPixelCoordinate(float x, float y, float center_x, float center_y, float rotation, float scale, float& out_x, float& out_y) const
{
  // Calculate vector from the center to the point
  float vx = x - center_x;
  float vy = y - center_y;

  // First scale
  float vx_scaled = vx * scale;
  float vy_scaled = vy * scale;

  // Create rotation elements
  float cos_theta = cos(rotation);
  float sin_theta = sin(rotation);

  // Rotate vector
  float vx_transformed = vx_scaled*cos_theta-vy_scaled*sin_theta;
  float vy_transformed = vx_scaled*sin_theta+vy_scaled*cos_theta;

  out_x = center_x + vx_transformed;
  out_y = center_y + vy_transformed;
}
  
//------------------------------------------------------------------------------

inline
void Image::GetInterpolatedPixel(float x, float y, float& r, float& g, float& b, bool disp) const
{
  r = g = b = 0;

  int NW_x = std::min((int)x, width-1);
  int NW_y = std::min((int)y, height-1);
  int NE_x = std::min(NW_x + 1, width-1);
  int NE_y = NW_y;
  int SW_x = NW_x;
  int SW_y = std::min(NW_y + 1, height-1);
  int SE_x = std::min(NW_x + 1, width-1);
  int SE_y = std::min(NW_y + 1, height-1);

  float dx = x - NW_x;
  float dy = y - NW_y;

 if(NW_x>=0 && NW_y>=0 && 
    SE_x<width && SE_y<height){

    int NW_v = GetGridPixel(NW_x, NW_y);
    int NE_v = GetGridPixel(NE_x, NE_y);
    int SW_v = GetGridPixel(SW_x, SW_y);
    int SE_v = GetGridPixel(SE_x, SE_y);

    float NW_w = (1.f-dx)*(1.f-dy);
    float NE_w = dx*(1.f-dy);
    float SW_w = (1.f-dx)*dy;
    float SE_w = dx*dy;

    r = float(Red(NW_v))*NW_w +
        float(Red(NE_v))*NE_w +
        float(Red(SW_v))*SW_w +
        float(Red(SE_v))*SE_w;

    g = float(Green(NW_v))*NW_w +
        float(Green(NE_v))*NE_w +
        float(Green(SW_v))*SW_w +
        float(Green(SE_v))*SE_w;

    b = float(Blue(NW_v))*NW_w +
        float(Blue(NE_v))*NE_w +
        float(Blue(SW_v))*SW_w +
        float(Blue(SE_v))*SE_w;
  }
}
  
//------------------------------------------------------------------------------

inline
void Image::GetHorizontallyInterpolatedPixel(float x, float y, float& r, float& g, float& b, bool disp) const
{
  r = g = b = 0;

  float NW_x = (int)x;
  float NW_y = (int)y;
  float NE_x = NW_x + 1;
  float NE_y = NW_y;

  float dx = x - NW_x;

 if(NW_x>=0 && NW_y>=0){

    int NW_v = GetGridPixel(NW_x, NW_y);
    int NE_v = GetGridPixel(NE_x, NE_y);

    float NW_w = (1.f-dx);
    float NE_w = dx;

    r = float(Red(NW_v))*NW_w +
        float(Red(NE_v))*NE_w;

    g = float(Green(NW_v))*NW_w +
        float(Green(NE_v))*NE_w;

    b = float(Blue(NW_v))*NW_w +
        float(Blue(NE_v))*NE_w;
  }
}
  
//------------------------------------------------------------------------------

inline
bool Image::IsInside(float x, float y) const
{
  if(x>=0 && x<width && y>=0 && y<height)
    return true;
  return false;
}

//------------------------------------------------------------------------------
  
inline
int Image::EncodeColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  int colour = ((unsigned int)a) << 24  | ((unsigned int)r) << 16 | ((unsigned int)g) << 8 | ((unsigned int)b);
  return colour;
}

inline
void Image::DecodeColour(int color, unsigned char& r, unsigned char& g, unsigned char& b)
{
  r = Red(color);
  g = Green(color);
  b = Blue(color);
}

//------------------------------------------------------------------------------
  
inline
unsigned char Image::Red(int colour)
{
  return (0x00FF0000 & colour) >> 16;
}

//------------------------------------------------------------------------------
  
inline
unsigned char Image::Green(int colour)
{
  return (0x0000FF00 & colour) >> 8;
}  

//------------------------------------------------------------------------------
  
inline
unsigned char Image::Blue(int colour)
{
  return (0x000000FF & colour);
}

//------------------------------------------------------------------------------
  
inline
unsigned char Image::Alpha(int colour)
{
  return colour >> 24;
}

//------------------------------------------------------------------------------
  
inline
unsigned char Image::Grey(int colour)
{
  float r = ((float)Red(colour));
  float g = ((float)Green(colour));
  float b = ((float)Blue(colour));

  return (unsigned char)((r + g + b)/3.f);
}

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif
