#include "graph_particles.h"
#include "visualizer_cimg.h"
#include "flo_io.h"
#include "image_operator.h"
#include "colorcode.h"
#include <limits>

#include "image_reader_cimg.h" // Temporary
#include <iomanip>

using namespace std;
using namespace std::placeholders;

namespace pmbp{
  
//------------------------------------------------------------------------------

GraphParticles::GraphParticles(const Parameters& p) : parameters(p)
{
  image_operator = 0;
}

//------------------------------------------------------------------------------

GraphParticles::~GraphParticles()
{
  
}

//------------------------------------------------------------------------------

void GraphParticles::InitialiseImages(Image* one, Image* two)
{
  images[kOne] = one;
  images[kTwo] = two;
  w[kOne] = one->width;
  h[kOne] = one->height;
  w[kTwo] = two->width;
  h[kTwo] = two->height;
  gradients[kOne] = images[kOne]->GetGradient();
  gradients[kTwo] = images[kTwo]->GetGradient();
  filtered[kOne] = images[kOne]->MedianFilter(3);
  filtered[kTwo] = images[kTwo]->MedianFilter(3);
  
  // Bind the function to the GetDisplacement of this object
  auto f = &GraphParticles::GetDisplacement;
  DisplacementFunction displacement_function = std::bind(f, this, _1, _2, _3, _4, _5);
  
  image_operator = new ImageOperator(images, gradients, filtered, w, h, parameters, displacement_function);
}

//------------------------------------------------------------------------------

void GraphParticles::InitialiseFields(){
  if(parameters.bidirectional)
  {
    InitialiseFields(kTwo);
  }
  InitialiseFields(kOne);
}

//------------------------------------------------------------------------------
  
void GraphParticles::InitialiseFields(View view){
  // Allocate memory
  nodes[view].Resize(w[view], h[view]);
  processed[view].Resize(w[view], h[view]);
  propagated[view].Resize(w[view], h[view]);
  
  propagated[kOne].SetAll(false);
  propagated[kTwo].SetAll(false);

  // Reserve memory for nodes
  for(int j=0; j<h[view]; ++j){
    for(int i=0; i<w[view]; ++i){
      nodes[view].Set(i, j, 0, Node(parameters.n_particles));
      nodes[view].Get(i,j)->InitialiseFoundation();
    }
  }
}

//------------------------------------------------------------------------------
  
void GraphParticles::InitialiseNodes(){

  // If there is no import file
  if(parameters.import_file.empty()){

    // Initialise the nodes in the normal way
    if(parameters.bidirectional)
    {
      InitialiseNodes(kTwo);
    }
    InitialiseNodes(kOne);
  }else{
    // Otherwise we import them from the file
    ImportFields(parameters.import_file);
  }
}
  
//------------------------------------------------------------------------------
  
void GraphParticles::InitialiseNodes(View view)
{
  // Initialise the nodes
  for(int j=0; j<h[view]; ++j){
    for(int i=0; i<w[view]; ++i){
      // Initialise node
      InitialiseNode(view, i, j);
    }
  }
}
  
//------------------------------------------------------------------------------

void GraphParticles::Solve()
{
  // Initialise
  InitialiseFields();
  InitialiseNodes();

  // Energy tracking
  float unary_energy;
  float pairwise_energy;
  
  // Time logging
  Clock clock;
  
  // Create visualizers
  VisualizerCImg visu_motion("Motion", OutputMotionField(kOne));
  //VisualizerCImg visu_unary("Unary Energy", OutputUnaryEnergy(kOne, unary_energy));
  //VisualizerCImg visu_pairwise("Pairwise Energy", OutputPairwiseEnergy(kOne, pairwise_energy));
  //VisualizerCImg visu_reconstruction("Reconstruction", OutputReconstruction(kOne));
    
  // Iterate
  for(int i=0; i<parameters.n_iterations; ++i){
    Iterate(i);
    
    // Visualise
    visu_motion.Show(OutputMotionField(kOne));
    //visu_unary.Show(OutputUnaryEnergy(kOne, unary_energy));
    //visu_pairwise.Show(OutputPairwiseEnergy(kOne, pairwise_energy));
    //visu_reconstruction.Show(OutputReconstruction(kOne));

    cout << "Iteration " << i << std::endl;
    cout << "  Time elapsed: " << clock.Poll() << "s" << std::endl;
    //cout << "  Unary energy: " << unary_energy << endl;
    //cout << "  Pairwise energy: " << pairwise_energy << endl;
    //cout << "  Total energy: " << unary_energy + pairwise_energy << endl;
    DrawLine();
  }
}

//------------------------------------------------------------------------------

void GraphParticles::Iterate(int it)
{
  if(parameters.bidirectional)
  {
    IterateView(it, kTwo);
  }
  
  IterateView(it, kOne);

  ResetProcessed();
}

//------------------------------------------------------------------------------
  
void GraphParticles::IterateView(int it, View view)
{
  ImageReaderCImg ireader;
  std::stringstream ss;
  ss << parameters.output_dir << "/motion_it_" <<  std::setw( 3 ) << std::setfill( '0' ) << it << ".png";
  
  Image motion = OutputMotionField(kOne);
  ireader.save(&motion, ss.str());
  
  int i_first, i_last, j_first, j_last, i_incr, j_incr;
  GetDirections(it, view, i_first, i_last, j_first, j_last, i_incr, j_incr);
  
  std::stringstream title;
  title << "[View " << view << "] - Iteration " << it << " -";
  
  for(int j=j_first; j!=j_last; j+=j_incr){
    ProgressBar(title.str(), abs(j-j_first), h[view]-1, std::min(h[view]-1, 200), 28);
    for(int i=i_first; i!=i_last; i+=i_incr){
      // Basic message passing operations
      
      // Pulls the messages
      UpdateCurrentDisbelief(view, i, j);
      
      // Update particles
      Update(view, i, j);
      
      // Cache new foundations
      Cache(view, i, j);
    }
  }
}

//------------------------------------------------------------------------------

void GraphParticles::ResetProcessed()
{
  if(parameters.bidirectional)
  {
    processed[kTwo].SetAll(false);
  }
  processed[kOne].SetAll(false);
}
  
//------------------------------------------------------------------------------

void GraphParticles::Cache(View view, int x, int y)
{
  // Here we update the cached foundations
  
  Node* node = nodes[view].Get(x, y);

  for(int k=0; k<node->Size(); ++k){

    State const* particle = node->GetParticle(k);
    float d = node->GetDisbelief(k);
    
    if(x>0){
      float m = EvaluateMessage(view, x-1, y, x, y, *particle);
      float v = d - m;
      node->SetFoundationValue(kLeft, k, v);
    }

    if(y>0){
      float m = EvaluateMessage(view, x, y-1, x, y, *particle);
      float v = d - m;
      node->SetFoundationValue(kUp, k, v);
    }
    
    if(x<w[view]-1){
      float m = EvaluateMessage(view, x+1, y, x, y, *particle);
      float v = d - m;
      node->SetFoundationValue(kRight, k, v);
    }
    
    if(y<h[view]-1){
      float m = EvaluateMessage(view, x, y+1, x, y, *particle);
      float v = d - m;
      node->SetFoundationValue(kDown, k, v);
    }
  }
  
  if(x>0) node->NormalizeFoundation(kLeft);
  if(y>0) node->NormalizeFoundation(kUp);
  if(x<w[view]-1) node->NormalizeFoundation(kRight);
  if(y<h[view]-1) node->NormalizeFoundation(kDown);
  
  // Set processed
  processed[view].Set(x, y, true);
}
  
//------------------------------------------------------------------------------
  
void GraphParticles::ProposeCandidate(View view, int x, int y, const State& particle, bool force)
{
  Node* node = nodes[view].Get(x, y);
  
  // Look for the highest value
  int idx = node->GetMaxValueParticleIdx();
  float highest_value = node->GetMaxValue();
  
  float B = EvaluateDisbelief(view, x, y, particle);
  
  if(B<highest_value){
    propagated[view].Set(x, y, true);
    nodes[view].Get(x, y)->SetParticle(idx, particle, B);
  }

  return;
}

//------------------------------------------------------------------------------

float GraphParticles::EvaluateDisbelief(View view, int x, int y, const State& state, bool early_termination) const
{
  // We evaluate the value of the disbelief at the state given the cached foundations
  
  float message_sum = 0;
  
  if(x>0){
    message_sum += EvaluateMessage(view, x-1, y, x, y, state);
  }

  if(y>0){
    message_sum += EvaluateMessage(view, x, y-1, x, y, state);
  }
  
  if(x<w[view]-1){
    message_sum += EvaluateMessage(view, x+1, y, x, y, state);
  }
  
  if(y<h[view]-1){
    message_sum += EvaluateMessage(view, x, y+1, x, y, state);
  }
  
  float worst_value = parameters.infinity;
  
  if(early_termination){
    worst_value = GetMaxDisbelief(view, x, y) - message_sum;
  }
  float unary = UnaryEnergy(view, x, y, state, worst_value);

  return unary + message_sum;
}

//------------------------------------------------------------------------------
  
float GraphParticles::EvaluateMessage(View view, int from_x, int from_y, int to_x, int to_y, const State& state) const
{
  // Perform the miminization required to compute the message
  float value = infinity;
  Node const* source = nodes[view].Get(from_x, from_y);
  
  Direction direction = GetDirection(from_x, from_y, to_x, to_y);
  
  for(int i = 0; i<source->Size(); ++i){
    float pw = PairwiseEnergy(view, to_x, to_y, state, from_x, from_y, *source->GetParticle(i));
    float foundation = source->GetFoundationValue(direction, i);
    
    float current = pw + foundation;
    
    if(current < value){
      value = current;
    }
  }
  
  return value;
}
  
//------------------------------------------------------------------------------
  
State const* GraphParticles::GetMinDisbeliefState(View view, int x, int y) const
{
  Node const* node = nodes[view].Get(x, y);
  return node->GetMinValueParticle();
}
  
//------------------------------------------------------------------------------

float GraphParticles::GetMaxDisbelief(View view, int x, int y) const
{
  Node const* node = nodes[view].Get(x, y);
  return node->GetMaxValue();
}

//------------------------------------------------------------------------------
  
void GraphParticles::UpdateCurrentDisbelief(View view, int x, int y)
{
  Node* node = nodes[view].Get(x, y);
  
  for(int k=0; k<node->Size(); ++k){
    State const * particle = node->GetParticle(k);
    node->SetParticleValue(k, EvaluateDisbelief(view, x, y, *particle));
  }
}

//------------------------------------------------------------------------------

Image GraphParticles::OutputMotionField(View view) const
{
  Image output(w[view], h[view]);
  
  float maxx = -9999, maxy = -9999;
  float minx =  9999, miny =  9999;
  float maxrad = -1;
  
  for(int i=0; i<output.width; ++i){
    for(int j=0; j<output.height; ++j){
      
      float dx, dy;
      State const* state = GetMinDisbeliefState(view, i, j);
      GetDisplacement(i, j, *state, dx, dy);
      
      maxx=std::max(maxx, dx);
      minx=std::min(minx, dx);
      maxy=std::max(maxy, dy);
      miny=std::min(miny, dy);
      float rad = sqrt(dx * dx + dy * dy);
      maxrad = std::max(maxrad, rad);
    }
  }
  
  if (parameters.max_motion > 0) // i.e., specified
    maxrad = parameters.max_motion;
  
  if (maxrad == 0) // if flow == 0 everywhere
    maxrad = 1;
  
  for(int i=0; i<output.width; ++i){
    for(int j=0; j<output.height; ++j){
      
      float dx, dy;
      State const* state = GetMinDisbeliefState(view, i, j);
      GetDisplacement(i, j, *state, dx, dy);
      
      unsigned char pix[3];
      
      computeColor(dx/maxrad, dy/maxrad, pix);
      
      int colour = Image::EncodeColour(pix[2], pix[1], pix[0], 255);
      output.SetGridPixel(i, j, colour);
      
    }
  }
  
  
  return output;
}

//------------------------------------------------------------------------------

Image GraphParticles::OutputReconstruction(View view) const
{
  View target = view;
  View source = OtherView(target);
  
  Image reconstructed(images[view]->width, images[view]->height);
  
  std::vector<std::vector<float> > weights;
  weights.resize(reconstructed.width);
  for(int i=0; i<reconstructed.width; ++i){
    weights[i].resize(reconstructed.height);
    for(int j=0; j<reconstructed.height; ++j){
      weights[i][j] = 0;
    }
  }
  
  std::vector<std::vector<std::vector<float> > > colours;
  colours.resize(reconstructed.width);
  for(int i=0; i<reconstructed.width; ++i){
    colours[i].resize(reconstructed.height);
    for(int j=0; j<reconstructed.height; ++j){
      colours[i][j].resize(3);
      for(int k=0; k<3; ++k){
        colours[i][j][k] = 0;
      }
    }
  }
  
  for(int i=0; i<reconstructed.width; ++i){
    for(int j=0; j<reconstructed.height; ++j){
      
      float dx, dy;
      State const* state = GetMinDisbeliefState(view, i, j);
      GetDisplacement(i, j, *state, dx, dy);
      
      float start_x = i - parameters.patch_size;
      float end_x   = i + parameters.patch_size;
      float start_y = j - parameters.patch_size;
      float end_y   = j + parameters.patch_size;
      
      start_x = std::max(start_x, 0.f);
      end_x   = std::min(end_x, (float)reconstructed.width-1);
      start_y = std::max(start_y, 0.f);
      end_y   = std::min(end_y, (float)reconstructed.height-1);
      
      float xc_source = i + dx;
      float yc_source = j + dy;
      
      for (float y_target = start_y; y_target <= end_y; ++y_target){
        for (float x_target = start_x; x_target <= end_x; ++x_target){
          
          GetDisplacement(x_target, y_target, *state, dx, dy);
          
          float x_source = x_target + dx;
          float y_source = y_target + dy;
          
          if(images[target]->IsInside(x_target, y_target)){
            if(images[source]->IsInside(x_source, y_source)){
              
              float r_s, g_s, b_s;
              
              images[source]->GetTransformedSubPixel(x_source, y_source, 0, 1, xc_source, yc_source, r_s, g_s, b_s);
              
              colours[x_target][y_target][0] += r_s;
              colours[x_target][y_target][1] += g_s;
              colours[x_target][y_target][2] += b_s;
              weights[x_target][y_target]++;
              
            }
          }
        }
      }
    }
  }
  
  for(int i=0; i<reconstructed.width; ++i){
    for(int j=0; j<reconstructed.height; ++j){
      
      float r = std::max(std::min((float)(colours[i][j][0]/weights[i][j]), 255.f), 0.f);
      float g = std::max(std::min((float)(colours[i][j][1]/weights[i][j]), 255.f), 0.f);
      float b = std::max(std::min((float)(colours[i][j][2]/weights[i][j]), 255.f), 0.f);
      
      int colour = Image::EncodeColour((int)r , (int)g, (int)b, 255);
      reconstructed.SetGridPixel(i, j, colour);
      
    }
  }
  
  return reconstructed;
}


//------------------------------------------------------------------------------

Flo* GraphParticles::ExportFlo(View view) const
{
  Flo* flo = new Flo(w[view], h[view]);
  
  for(int i=0; i<w[view]; ++i){
    for(int j=0; j<h[view]; ++j){
      
      float dx, dy;
      State const* state = GetMinDisbeliefState(view, i, j);
      GetDisplacement(i, j, *state , dx, dy);
      
      flo->SetFlow(j, i, dx, dy);
    }
  }
  
  return flo;
}
  
//------------------------------------------------------------------------------
  
Image GraphParticles::OutputUnaryEnergy(View view, float& energy) const
{
  energy = 0.f;
  
  Field<float> field(w[view], h[view], 1);
  float max_energy = 0;
  
  for(int i=0; i<w[view]; ++i){
    for(int j=0; j<h[view]; ++j){
      State const* state = GetMinDisbeliefState(view, i, j);
      float e = UnaryEnergy(view, i, j, *state, infinity);
      energy += e;
      field[i][j][0] = e;
      
      if(max_energy < e){
        max_energy = e;
      }
    }
  }
  
  Image image(w[view], h[view]);
  
  for(int i=0; i<w[view]; ++i){
    for(int j=0; j<h[view]; ++j){
      float e = 255*field[i][j][0]/max_energy;
      image.SetGridPixel(i, j, Image::EncodeColour(e, e, e, 255));
    }
  }
  
  return image;
}
  
//------------------------------------------------------------------------------
  
Image GraphParticles::OutputPairwiseEnergy(View view, float& energy) const
{
  energy = 0.f;
  
  Field<float> field(w[view], h[view], 1);
  float max_energy = 0;
  
  for(int i=0; i<w[view]; ++i){
    for(int j=0; j<h[view]; ++j){
      State const* state = GetMinDisbeliefState(view, i, j);
      
      float e = 0;
      
      if(i>0){
        e += PairwiseEnergy(view, i, j, *state, i-1, j, *GetMinDisbeliefState(view, i-1, j));
      }
      
      if(j>0){
        e += PairwiseEnergy(view, i, j, *state, i, j-1, *GetMinDisbeliefState(view, i, j-1));
      }
      
      if(i<w[view]-1){
        e += PairwiseEnergy(view, i, j, *state, i+1, j, *GetMinDisbeliefState(view, i+1, j));
      }
      
      if(j<h[view]-1){
        e += PairwiseEnergy(view, i, j, *state, i, j+1, *GetMinDisbeliefState(view, i, j+1));
      }
      
      energy += e;
      field[i][j][0] = e;
      
      if(max_energy < e){
        max_energy = e;
      }
    }
  }
  
  Image image(w[view], h[view]);
  
  for(int i=0; i<w[view]; ++i){
    for(int j=0; j<h[view]; ++j){
      float e = 255*field[i][j][0]/max_energy;
      
      e = std::min(255.f, e);
      e = std::max(0.f, e);
      
      image.SetGridPixel(i, j, Image::EncodeColour(e, e, e, 255));
    }
  }
  
  return image;
}
  
//------------------------------------------------------------------------------
  
void GraphParticles::ImportFields(const std::string& filename){
  std::cout << "Importing: " << filename << std::endl;
  
  std::fstream fs(filename.c_str(), std::ios::in | std::ios::binary);
  
  // Read the tag
  char tag;
  fs.read(&tag, 1);
  
  int www = 0;
  int hhh = 0;
  
  // Read state dimensions
  fs.read((char*)&data_dim, sizeof(data_dim));
  fs.read((char*)&meta_dim, sizeof(meta_dim));
  // Read image dimensions
  fs.read((char*)&www, sizeof(www));
  fs.read((char*)&hhh, sizeof(hhh));
  // Read number of particles
  fs.read((char*)&parameters.n_particles, sizeof(parameters.n_particles));

  std::cout << "Importing match field with dim [" << data_dim << "," << meta_dim << "] and size [" << www << "," << hhh << "] with " << parameters.n_particles << " particles" << std::endl;
  
  for(int i=0; i<www; ++i){
    for(int j=0; j<hhh; ++j){
      for(int k=0; k<parameters.n_particles; ++k){
        State state(data_dim, meta_dim);
        fs.read((char*)&state.data[0], data_dim*sizeof(state.data[0]));
        fs.read((char*)&state.meta[0], meta_dim*sizeof(state.meta[0]));
        nodes[kOne].Set(i, j, 0, Node(parameters.n_particles));
        nodes[kOne].Get(i,j)->InitialiseFoundation();
        nodes[kOne].Get(i, j)->SetParticle(k, state, 0);
      }
    }
  }
  
  if(parameters.bidirectional)
  {
    fs.read((char*)&www, sizeof(www));
    fs.read((char*)&hhh, sizeof(hhh));
    
    // Right
    for(int i=0; i<www; ++i){
      for(int j=0; j<hhh; ++j){
        for(int k=0; k<parameters.n_particles; ++k){
          State state(data_dim, meta_dim);
          fs.read((char*)&state.data[0], data_dim*sizeof(state.data[0]));
          fs.read((char*)&state.meta[0], meta_dim*sizeof(state.meta[0]));
          nodes[kTwo].Set(i, j, 0, Node(parameters.n_particles));
          nodes[kTwo].Get(i,j)->InitialiseFoundation();
          nodes[kTwo].Get(i, j)->SetParticle(k, state, 0);
        }
      }
    }
  }
  
  fs.close();
  
  std::cout << "Initialisation OK!" << std::endl;
}

//------------------------------------------------------------------------------

void GraphParticles::ExportFields(const std::string& filename){
  std::fstream fs(filename.c_str(), std::ios::out | std::ios::binary);
  
  char tag = GetTag();
  
  // Export application specific tag
  fs.write(&tag, sizeof(tag));
  
  // Export state dimensions
  fs.write((char*)&data_dim, sizeof(data_dim));
  fs.write((char*)&meta_dim, sizeof(meta_dim));
  
  // Export w and h
  fs.write((char*)&w[kOne], sizeof(w[kOne]));
  fs.write((char*)&h[kOne], sizeof(h[kOne]));

  // Export number of particles
  fs.write((char*)&parameters.n_particles, sizeof(parameters.n_particles));
  
  std::cout << "Exporting match field with dim [" << data_dim << "," << meta_dim << "] and  size [" << w[kOne] << "," << h[kOne] << "] with " << parameters.n_particles << " particles" << std::endl;
  
  for(int i=0; i<w[kOne]; ++i){
    for(int j=0; j<h[kOne]; ++j){
      for(int k=0; k<parameters.n_particles; ++k){
        State const* state = nodes[kOne].Get(i, j)->GetParticle(k);
        fs.write((char*)&state->data[0], data_dim*sizeof(state->data[0]));
        fs.write((char*)&state->meta[0], meta_dim*sizeof(state->meta[0]));
      }
    }
  }

  if(parameters.bidirectional){
    fs.write((char*)&w[kOne], sizeof(w[kTwo]));
    fs.write((char*)&h[kOne], sizeof(h[kTwo]));
    
    std::cout << "Exporting match field with size " << w[kTwo] << "," << h[kTwo] << std::endl;
    
    for(int i=0; i<w[kTwo]; ++i){
      for(int j=0; j<h[kTwo]; ++j){
        for(int k=0; k<parameters.n_particles; ++k){
          State const* state = nodes[kTwo].Get(i, j)->GetParticle(k);
          fs.write((char*)&state->data[0], data_dim*sizeof(state->data[0]));
          fs.write((char*)&state->meta[0], meta_dim*sizeof(state->meta[0]));
        }
      }
    }
  }
  
  fs.close();
}
  
//------------------------------------------------------------------------------

void GraphParticles::GetDirections(int k, View view, int& i_first, int& i_last, int& j_first, int& j_last, int& i_incr, int& j_incr) const
{
  // Four cases
  if(k%4==0){
    // Top left -> bottom right
    i_first = 0; j_first = 0; i_last = w[view]-1; j_last = h[view]-1; i_incr = 1; j_incr = 1;
  }else if(k%4==2){
    // Bottom left -> top right
    i_first = 0; j_first = h[view]-1; i_last = w[view]-1; j_last = 0; i_incr = 1; j_incr = -1;
  }else if(k%4==1){
    // Bottom right -> top left
    i_first = w[view]-1; j_first = h[view]-1; i_last = 0; j_last = 0; i_incr = -1; j_incr = -1;
  }else{
    // Top right -> bottom left
    i_first = w[view]-1; j_first = 0; i_last = 0; j_last = h[view]-1; i_incr = -1; j_incr = 1;
  }
}

//------------------------------------------------------------------------------

void GraphParticles::Inspect(){
  
  Image flow = OutputMotionField(kOne);
  CImg<unsigned char> cflow = ImageToCImg(flow);
  
  CImg<unsigned char> one = ImageToCImg(*images[kOne]);
  CImgDisplay visualisation_one(one, "Inspect - One");
  CImgDisplay visualisation_flow(cflow, "Inspect - Flow");
  
  while (!visualisation_flow.is_closed()) {
    
    CImgDisplay::wait_all();
    
    if(visualisation_one.button()) {
      const int x = visualisation_flow.mouse_x();
      const int y = visualisation_flow.mouse_y();
      State const * state = GetMinDisbeliefState(kOne, x, y);
      std::cout << "At [" << x << "," << y << "]: " << nodes[kOne].Get(x, y)->Summary() << std::endl;
      std::cout << "Best displacement at [" << x << "," << y << "]: " << "[" << state->data[0] << "," << state->data[1] << "] with disbelief: " << nodes[kOne].Get(x, y)->GetMinValue() << endl;
    }
    
    
    if(visualisation_flow.button()) {
      const int x = visualisation_flow.mouse_x();
      const int y = visualisation_flow.mouse_y();
      State const * state = GetMinDisbeliefState(kOne, x, y);
      std::cout << "At [" << x << "," << y << "]: " << nodes[kOne].Get(x, y)->Summary() << std::endl;
      std::cout << "Best displacement at [" << x << "," << y << "]: " << "[" << state->data[0] << "," << state->data[1] << "] with disbelief: " << nodes[kOne].Get(x, y)->GetMinValue() << endl;
    }
  }  
}

  
//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------