//------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include "graph_2d_flow.h"
#include "graph_stereo.h"
#include "graph_discrete.h"
#include "image_reader_cimg.h"
#include "utils.h"
#include "flo_io.h"

//------------------------------------------------------------------------------

using namespace pmbp;

//------------------------------------------------------------------------------

// Static elements
std::mt19937_64 Random::engine;
std::uniform_real_distribution<float> Random::uniform;
std::normal_distribution<float> Random::normal;

//------------------------------------------------------------------------------

Parameters default_stereo_parameters()
{
  Parameters parameters;
  parameters.one_name = "one.png";
  parameters.two_name = "two.png";
  parameters.patch_size = 10;
  parameters.max_motion = 59.f;
  parameters.weight_pw = 1;
  parameters.truncate_pw = 1.f;
  parameters.n_particles = 1;
  parameters.asw = 10;
  parameters.tau1 = 10;
  parameters.tau2 = 2;
  parameters.alpha = 0.9;
  parameters.border = 0.85f;
  parameters.output_disparity_scale = 4.f;
  parameters.bidirectional = false;
  parameters.infinity = 999999.f;
  parameters.output_dir = "";
  parameters.import_file = "";
  return parameters;
}

//------------------------------------------------------------------------------

Parameters default_discrete_parameters()
{
  Parameters parameters;
  parameters.one_name = "one.png";
  parameters.two_name = "two.png";
  parameters.n_iterations = 10;
  parameters.patch_size = 1;
  parameters.max_motion = 5.f;
  parameters.weight_pw = 0.1;//1.f;
  parameters.truncate_pw = 50;
  parameters.n_particles = 1;
  parameters.asw = 15;
  parameters.tau1 = 20;
  parameters.tau2 = 20;
  parameters.alpha = 0;
  parameters.border = 0.85f;
  parameters.discrete_step = 1;
  parameters.bidirectional = false;
  float maxmatchcosts = (1.f - parameters.alpha) * parameters.tau1 + parameters.alpha * parameters.tau2;
  float bordercosts = maxmatchcosts * parameters.border;
  parameters.infinity = parameters.patch_size*parameters.patch_size*bordercosts;
  parameters.output_dir = "";
  parameters.import_file = "";
  return parameters;
}

//------------------------------------------------------------------------------

Parameters default_2dflow_parameters()
{
  Parameters parameters;
  parameters.one_name = "one.png";
  parameters.two_name = "two.png";
  parameters.n_iterations = 10;
  parameters.patch_size = 2;
  parameters.max_motion = 0.f;
  parameters.weight_pw = 0.001;//1.f;
  parameters.truncate_pw = 50;
  parameters.n_particles = 1;
  parameters.asw = 15;
  parameters.tau1 = 20;
  parameters.tau2 = 20;
  parameters.alpha = 0;
  parameters.border = 0.85f;
  parameters.bidirectional = false;
  parameters.infinity = 9999999.f;
  parameters.output_dir = "";
  parameters.import_file = "";
  return parameters;
}

//------------------------------------------------------------------------------

typedef enum{
  kDiscrete,
  kStereo,
  k2DFlow
} Application;

//------------------------------------------------------------------------------

void display_usage(){
  
  std::cout << "Usage: fpmbp [mode] -one image1 -two image2 [options]" << std::endl;
  std::cout << std::endl;
  std::cout << "Modes: " << std::endl;
  std::cout << "  -discrete \t\t Run a dense, discrete BP (slow)" << std::endl;
  std::cout << "  -stereo \t\t Run the stereo application" << std::endl;
  std::cout << "  -2dflow \t\t Run the 2d flow application" << std::endl;
  std::cout << "Options: " << std::endl;
  std::cout << "  -n_iterations nit \t Number of iterations to run" << std::endl;
  std::cout << "  -patch_size p \t Half the patch size (full size is 2*p+1)" << std::endl;
  std::cout << "  -max_motion m \t Maximum displacement allowed (m=0 to set no limit)" << std::endl;
  std::cout << "  -n_particles n \t Number of particles" << std::endl;
  std::cout << "  -weight_pw n \t\t Weight of the pairwise term (n=0 to set no pairwise term)" << std::endl;
  std::cout << "  -truncate_pw t \t Truncation value for the pairwise energy" << std::endl;
  std::cout << "  -tau1 tau1 \t\t Truncation value for color component of the unary energy" << std::endl;
  std::cout << "  -tau2 tau2 \t\t Truncation value for gradient component of the unary energy" << std::endl;
  std::cout << "  -alpha a \t\t Weight of the gradient component of the unary energy" << std::endl;
  std::cout << "  -asw asw \t\t Adaptive support weight sigma value" << std::endl;
  std::cout << "  -border b \t\t Border penalty value" << std::endl;
  std::cout << "  -bidir [0|1] \t Enable computation of the forward AND backwards flow" << std::endl;
  std::cout << "  -out_dir out \t Directory where results are exported" << std::endl;
  std::cout << "  -import file \t Import previous results from file" << std::endl;
  std::cout << "  -disp_scale b \t Disparity scale for disparity field display (stereo mode only)" << std::endl;
  std::cout << "  -discrete_step d \t Discretisation value (discrete mode only)" << std::endl;
  std::cout << std::endl;
  
}

//------------------------------------------------------------------------------

void display_parameters(Application app, const Parameters& parameters){
  
  DrawLine();
  std::cout << "PMBP, Mode: [";
  if(app==kStereo) { std::cout << "stereo]"; }
  if(app==k2DFlow) { std::cout << "2dflow]"; }
  if(app==kDiscrete) { std::cout << "discrete]"; }
  std::cout << std::endl;
  DrawLine();
  std::cout << "Inputs: " << std::endl;
  DrawLine();
  std::cout << "  one: " << parameters.one_name << std::endl;
  std::cout << "  two: " << parameters.two_name << std::endl;
  DrawLine();
  std::cout << "Parameters: " << std::endl;
  DrawLine();
  std::cout << "  n_iterations: " << parameters.n_iterations << std::endl;
  std::cout << "  patch_size: \t" << parameters.patch_size << std::endl;
  std::cout << "  max_motion: \t" << parameters.max_motion << std::endl;
  std::cout << "  n_particles: \t" << parameters.n_particles << std::endl;
  std::cout << "  weight_pw: \t" << parameters.weight_pw << std::endl;
  std::cout << "  truncate_pw: \t" << parameters.truncate_pw << std::endl;
  std::cout << "  tau1: \t" << parameters.tau1 << std::endl;
  std::cout << "  tau2: \t" << parameters.tau2 << std::endl;
  std::cout << "  alpha: \t" << parameters.alpha << std::endl;
  std::cout << "  asw: \t\t" << parameters.asw << std::endl;
  std::cout << "  border: \t" << parameters.border << std::endl;
  std::cout << "  bidir: \t" << parameters.bidirectional << std::endl;
  std::cout << "  out_dir: \t" << parameters.output_dir << std::endl;
  std::cout << "  import_file: \t" << parameters.import_file << std::endl;
  
  if(app==kStereo)
    std::cout << "  disp_scale: \t" << parameters.output_disparity_scale<< std::endl;
  
  if(app==kDiscrete)
    std::cout << "  discrete_step:" << parameters.discrete_step<< std::endl;
  DrawLine();
  
}

//------------------------------------------------------------------------------

bool process_args(int argc, char* argv[], Application &application, Parameters& parameters)
{
  parameters = default_stereo_parameters();
  application = kStereo;

  int pos = 1;
  
  if(argc == 1) { display_usage(); return false; }
  
  // Parse the arguments
  while (1)
  {
    if (pos + 1 >= argc) break;
    
    if (std::string(argv[pos]) == "-help" || std::string(argv[pos]) == "--help" ) { display_usage(); return false; }
    else if (std::string(argv[pos]) == "-discrete")               { parameters = default_discrete_parameters(); application = kDiscrete; pos++; }
    else if (std::string(argv[pos]) == "-stereo")                 { parameters = default_stereo_parameters();application = kStereo; pos++; }
    else if (std::string(argv[pos]) == "-2dflow")                 { parameters = default_2dflow_parameters();application = k2DFlow; pos++; }
    else if (std::string(argv[pos]) == "-one")                    { parameters.one_name = argv[++pos]; pos++; }
    else if (std::string(argv[pos]) == "-two")                    { parameters.two_name = argv[++pos]; pos++; }
    else if (std::string(argv[pos]) == "-n_iterations")           { parameters.n_iterations = atoi(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-patch_size")             { parameters.patch_size = atoi(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-max_motion")             { parameters.max_motion = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-n_particles")            { parameters.n_particles = atoi(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-weight_pw")              { parameters.weight_pw = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-truncate_pw")            { parameters.truncate_pw = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-tau1")                   { parameters.tau1 = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-tau2")                   { parameters.tau2 = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-alpha")                  { parameters.alpha = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-asw")                    { parameters.asw = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-border")                 { parameters.border = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-bidir")                  { parameters.bidirectional = atoi(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-disp_scale")             { parameters.output_disparity_scale = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-discrete_step")          { parameters.discrete_step = atof(argv[++pos]); pos++; }
    else if (std::string(argv[pos]) == "-out_dir")                { parameters.output_dir = argv[++pos]; pos++; }
    else if (std::string(argv[pos]) == "-import_file")                { parameters.import_file = argv[++pos]; pos++; }
  }
  
  return true;
}

//------------------------------------------------------------------------------

void save_results(GraphParticles* graph, const std::string& output_dir)
{
  if(output_dir.empty()){
    return;
  }
  
  // Here add export of the different outputs
  ImageReaderCImg ireader;

  Image motion = graph->OutputMotionField(kOne);
  ireader.save(&motion, output_dir+"/motion_one.png");
  
  Image reconstruction = graph->OutputReconstruction(kOne);
  ireader.save(&reconstruction, output_dir+"/reconstruction_one.png");
  
  Flo* flo = graph->ExportFlo(kOne);
  FloIO flo_io;
  flo_io.Save(flo, output_dir+"/flow.flo");
  graph->ExportFields(output_dir+"/state.fields");
}

//------------------------------------------------------------------------------

void load(const Parameters& parameters, const std::string& filename)
{
  std::fstream fs(filename.c_str(), std::ios::in | std::ios::binary);
  
  // A -> discrete
  // B -> 2d flow
  // C -> stereo
  
  // Read the tag
  char tag;
  fs.read(&tag, 1);
  
  GraphParticles* graph;
  
  if(tag=='A'){
    graph = new GraphDiscrete(parameters);
  }else if(tag=='B'){
    graph = new Graph2DFlow(parameters);
  }else if(tag=='C'){
    graph = new GraphStereo(parameters);
  }else{
    fs.close();
    std::cerr << "Error: unknown application" << std::endl;
    return;
  }

  fs.close();
}

//------------------------------------------------------------------------------

void run(const Parameters& parameters, Application application){
  
  GraphParticles* graph = 0;
  
  if(application == kDiscrete){
    graph = new GraphDiscrete(parameters);
  } else if (application == kStereo){
    graph = new GraphStereo(parameters);
  } else if (application == k2DFlow){
    graph = new Graph2DFlow(parameters);
  }
  
  ImageReaderCImg ireader;
  Image* one = ireader.load(parameters.one_name);
  Image* two = ireader.load(parameters.two_name);
  
  graph->InitialiseImages(one, two);
  graph->Solve();
  
  // Save results to a folder
  save_results(graph, parameters.output_dir);
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  // Parse the arguments
  Application application;
  Parameters parameters;
  bool success = process_args(argc, argv, application, parameters);
  
  if(success){
    display_parameters(application, parameters);
    run(parameters, application);
  }
  
  return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------