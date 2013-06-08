import os;
import sys;
import platform;

if(platform.system()=="Windows"):
  bin_directory = "bin/Release"
else:
  bin_directory = "bin"

n_iterations = 5; 
patch_size = 2;
max_motion = 59;
n_particles = 1;
weight_pw = 1;
truncate_pw = 1;
tau1 = 10;
tau2 = 2;
alpha = 0.9;
asw = 10;
border = 0.8;
output_disparity_scale = 4;

out_dir = "outputs/stereo"
if(platform.system()=="Windows"):
  out_dir = out_dir.replace("/","\\")
os.system("mkdir -p "+out_dir) 

one_name = "data/view1.png";
two_name = "data/view5.png";

if(platform.system()=="Windows"):
  exe = "pmbp.exe"
else:
  exe = "pmbp"

command = bin_directory + "/" + exe + " -stereo " + \
          " -one " + one_name + \
          " -two " + two_name + \
          " -n_iterations " + str(n_iterations) + \
          " -patch_size " + str(patch_size) + \
          " -max_motion " + str(max_motion) + \
          " -weight_pw " + str(weight_pw) + \
          " -n_particles " + str(n_particles) + \
          " -weight_pw " + str(weight_pw) + \
          " -truncate_pw " + str(truncate_pw) + \
          " -tau1 " + str(tau1) + \
          " -tau2 " + str(tau2) + \
          " -alpha " + str(alpha) + \
          " -asw " + str(asw) + \
          " -border " + str(border) + \
          " -disp_scale " + str(output_disparity_scale) + \
          " -out_dir " + str(out_dir)

if(platform.system()=="Windows"):
  command = command.replace("/","\\")

os.system(command)
