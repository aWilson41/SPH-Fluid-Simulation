# SPH-Fluid-Simulation
SPH Fluid Simulation. See "Particle-Based Fluid Simulation for Interactive Applications" for details. Contains 4 versions of SPH implemented mostly entirely separately for readability.

SPHDoimain: Single thread. No binning. Nice for learning. Not recommended.
ThreadedSPHDomain: Multi threaded with standard namespace threads. Threads spawn at start of simulation. Only gets faster at higher particle counts.
ThreadPoolSPHDomain: Same as above but maintains the threads through the simulation.
GLSLSPHDomain: Compute shader SPH.



Though you can control the number of threads easily. The single threaded class is just kept for readability. A simple binning is used to speed up the algorithm a bit. Multithreading really only takes affect (becomes faster) at higher particle counts.

Currently working on IISPH implementation.

Paper: http://matthias-mueller-fischer.ch/publications/sca03.pdf

Requires: GLM, GLFW, GLEW

Build with CMake using the provided CMakeLists.txt.

This includes my own graphics abstraction in the "Engine" folder (I might give it it's own repo soon) includes LodePNG for reading/writing png images. See Constants.h for parameters.

Left click to rotate view, right click to interact with simulation, scroll zoom out, enter to start/stop simulation. If frame outputting is turned on through code, 'p' can start/stop frame writing.

![Alt text](https://andaharoo.files.wordpress.com/2018/12/frame_0347-1.png)