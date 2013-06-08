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
- Add post-processing for stereo