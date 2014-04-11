# PMBP

This is an implementation of the PMBP algorithm (for more details, see our publication, **PMBP: PatchMatch Belief Propagation for Correspondence Field Estimation**, in BMVC 2012).

It currently contains two examples (stereo & 2d flow), and is meant to be modified and extended.

To create your own application, you need to create your own class that inherits from the base class **GraphPmbp** and implement the abstract methods.
PMBP combines PatchMatch and Belief Propagation using particles resampling, and there are three main steps to achieve that:

* Initialisation: To initialise the solution you need to be able to randomly generate particles. You will need to implement the **GraphPmbp::GetRandomState**.
* Propagation: The most important step of PMBP is the propagation using the particle set of a node's neighbours. You will need to implement **GraphPmbp::GetStateFromNeighbour**.
* Randomisation: Finally, you need to be able to sample around an existing particle. You will need to implement **GraphPmbp::GetRandomStateAround**.

See the two classes **GraphStereo** and **Graph2DFlow** for an example of how to implement these methods.

Note that there is also a more generic class, **GraphParticles** that is our base implementation of Particle BP without particle resampling. **GraphPmbp** derives from it, but feel free to create your own variation of **GraphParticles** that do not follow the resampling steps of **GraphPmbp**.

Other important methods are **GraphPmbp::UnaryEnergy** and **GraphPmbp::PairwiseEnergy** where you can define how your energies are computed.

You can use CMake to compile PMBP. It uses the **CImg** library, which is included in the tools directory, as well as **libpng** and **zlib** which you should install on your machine (and indicate the paths to CMake if it fails to find the packages automatically).

If you find any bug or have any comment, please let me know (f.besse@cs.ucl.ac.uk).

TODO:
- Add post-processing to stereo
- Add CRF weights to stereo

---------

Copyright (c) 2014, Frederic Besse
All rights reserved.

BSD LICENSE

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
