#ifndef UTILS_H
#define UTILS_H

//------------------------------------------------------------------------------

#include <vector>
#include <map>
#include <cstdarg>
#include <random>
#include <ctime>
#include "image.h"
#include "state.h"

//------------------------------------------------------------------------------

#ifdef _WIN32

#define isinf(x) (!_finite(x))
#define isnan(x) (x!=x)

#else

#define isinf(x) std::isinf(x)
#define isnan(x) std::isnan(x)

#endif

//------------------------------------------------------------------------------

namespace pmbp{

//------------------------------------------------------------------------------
  
const float infinity = 1e10f;
const float invalid = -123456789.f;
  
//------------------------------------------------------------------------------
  
enum View{
  kOne = 0,
  kTwo = 1
};

//------------------------------------------------------------------------------
  
inline
View OtherView(View view){
  if(view == kOne)
    return kTwo;
  else
    return kOne;
}
  
//------------------------------------------------------------------------------
  
enum Direction{
  kLeft = 0,
  kUp = 1,
  kRight = 2,
  kDown = 3,
  kUnknown
};
  
//------------------------------------------------------------------------------

inline
Direction GetDirection(int from_x, int from_y, int to_x, int to_y)
{
  if(from_x == to_x){
    if(from_y < to_y){
      return kDown;
    }else{
      return kUp;
    }
  }else{
    if(from_x < to_x){
      return kRight;
    }else{
      return kLeft;
    }
  }
  
  return kUnknown;
}

//------------------------------------------------------------------------------
  
template <typename T>
T Clamp(T a, T b, T v){
  
  T m = std::min(a, b);
  T M = std::max(a, b);
  
  return std::min(M, std::max(m, v));
}
  
//------------------------------------------------------------------------------
  
// Usef to perform median filtering
class MedFilterState{
public:
  MedFilterState(){};
  MedFilterState(State s, float m, float w){
    state = s;
    motion = m;
    weight = w;
  }
  
  bool operator < ( const MedFilterState &a) const
  {
    return motion < a.motion;
  }
    
  State state;
  float motion;
  float weight;
};

//------------------------------------------------------------------------------
    
class Clock{
public:
  Clock(){Start();}
  virtual ~Clock(){}
  
  void Start(){
    m_start = clock();
  }
  
  float Poll(){
    clock_t now = clock();
    return (float)(now-m_start)/CLOCKS_PER_SEC;
  }
  
private:
  std::clock_t m_start;
};
    
//------------------------------------------------------------------------------
  
class Random{
public:
  static float DrawNormal(){
    return normal(engine);
  }
  
  static float DrawUniform(){
    return uniform(engine);
  }

  static float DrawUniform(float m, float M){
    return m+uniform(engine)*(M-m);
  }
  
  static std::mt19937_64 engine;
  static std::uniform_real_distribution<float> uniform;
  static std::normal_distribution<float> normal;
};
    
//------------------------------------------------------------------------------
    
// Global parameters
struct Parameters
{
  std::string one_name;
  std::string two_name;
  int n_iterations;
  int patch_size;
  float max_motion;
  int n_particles;
  float weight_pw;
  float truncate_pw;
  float tau1;
  float tau2;
  float alpha;
  float asw;
  float border;
  float output_disparity_scale;
  float discrete_step;
  bool bidirectional;
  std::string output_dir;
  std::string import_file;
  
  float infinity;
};
    
//------------------------------------------------------------------------------
        
template <class T>
class Field {
public:
  Field(){};
  Field(int w, int h, int k){
    Resize(w, h, k);
  };
  ~Field(){};
  
  void Resize(int w, int h, int k){
    data_.resize(w);
    for(int i=0; i<w; ++i){
      data_[i].resize(h);
      for(int j=0; j<h; ++j){
        data_[i][j].resize(k);
      }
    }
  }

  void Resize(int w, int h){
    Resize(w, h, 1);
  }
  
  std::vector<std::vector<T> > & operator[](int i){
    return data_[i];
  }
  
  size_t Width(){return data_.size();}
  size_t Height(){return data_[0].size();}
  size_t K(){ return data_[0][0].size();}
  
  void Set(int i, int j, int k, const T& v){
    data_[i][j][k] = v;
  }
  
  void SetAll(const T& v){
    for(int i=0; i<Width(); ++i){
      for(int j=0; j<Height(); ++j){
        for(int k=0; k<K(); ++k){
          data_[i][j][k] = v;
        }
      }
    }
  }

  /*const T& operator()(int i, int j) const{
    return data_[i][j][0];
  }*/

  T const * Get(int i, int j, int k=0) const{
    return &data_[i][j][k];
  }
  
  T* Get(int i, int j, int k=0){
    return &data_[i][j][k];
  }
    
private:
  std::vector<std::vector<std::vector<T> > > data_;
};

//------------------------------------------------------------------------------
    
class Mask {
public:
  Mask(){};
  Mask(int w, int h){
    Resize(w, h);
  };
  ~Mask(){};
  
  void Resize(int w, int h){
    data_.resize(w);
    for(int i=0; i<w; ++i){
      data_[i].resize(h);
      for(int j=0; j<h; ++j){
        data_[i][j] = false;
      }
    }
  }
  
  std::vector<bool> & operator[](int i){
    return data_[i];
  }
  
  size_t Width(){return data_.size();}
  size_t Height(){return data_[0].size();}
  
  void Set(int i, int j, bool v){
    data_[i][j] = v;
  }
  
  void SetAll(bool v){
    for(int i=0; i<Width(); ++i){
      for(int j=0; j<Height(); ++j){
        data_[i][j] = v;
      }
    }
  }
  
  bool Get(int i, int j){
    return data_[i][j];
  }
  
  int Count(bool value){
    int count = 0;
    
    for(int i=0; i<Width(); ++i){
      for(int j=0; j<Height(); ++j){
        if(data_[i][j] == value)
          ++count;
      }
    }
    
    return count;
  }
  
  float Percentage(bool value){
    return 100*((float)Count(value))/(Width()*Height());
  }
  
private:
  std::vector<std::vector<bool > > data_;
};
    
//------------------------------------------------------------------------------
    
template <class T>
class Vector2
{
public:
  Vector2() : x(0), y(0) {};
  Vector2(T xx, T yy) : x(xx), y(yy) {};
  ~Vector2(){};
  
  Vector2<T>& operator=(const Vector2<T>& rhs)
  {
    x = rhs.x;
    y = rhs.y;
    return *this;
  };
  
  bool Equals(const Vector2<T>& rhs) const
  {
    return (rhs.x == x && rhs.y == y);
  };
  
  Vector2<T>& operator+=(const Vector2<T>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  };
  
  Vector2<T>& operator-=(const Vector2<T>& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  };
  
  Vector2<T>& operator*=(T rhs)
  {
    x *= rhs;
    y *= rhs;
    return *this;
  }
  
  Vector2<T>& operator/=(T rhs)
  {
    x /= rhs;
    y /= rhs;
    return *this;
  }
  
  const Vector2<T> operator+(const Vector2<T>& other) const {
    Vector2<T> result = *this;
    result += other;
    return result;
  };
  
  T x;
  T y;
};
    
//------------------------------------------------------------------------------

typedef Vector2<float> Pointf;
    
//------------------------------------------------------------------------------

// Logging
inline
void Log(const std::string& text)
{
  std::cout << text << std::endl;
}
    
//------------------------------------------------------------------------------

inline
void ProgressBar(const std::string& title, int x, int n, int r, int w)
{
  // Only update r times.
  if ( x % (n/r) != 0 ) return;
  
  // Calculuate the ratio of complete-to-incomplete.
  float ratio = x/(float)n;
  int   c     = ratio * w;
  
  // Show the percentage complete.
  printf("%s %3d%% [", title.c_str(), (int)(ratio*100) );
  
  // Show the load bar.
  for (int x=0; x<c; x++)
    printf("=");
  
  for (int x=c; x<w; x++)
    printf(" ");
  
  // ANSI Control codes to go back to the
  // previous line and clear it.
  // OS dependent
#ifdef _WIN32
  printf("]\r");
#else
  printf("]\n\033[F\033[J");
#endif
}

//------------------------------------------------------------------------------
    
inline
void DrawLine(){
  std::cout << "------------------------------------------------------------" << std::endl;
}
    
//------------------------------------------------------------------------------
    
}
    
//------------------------------------------------------------------------------

#endif
