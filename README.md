# SPH-Fluid-Simulation
SPH Fluid Simulation. See "Particle-Based Fluid Simulation for Interactive Applications" for details. Contains 4 versions of SPH implemented separately for readability. SPHDomain is a "base" one.

* SPHDomain: Single thread. No binning. Nice for learning. Not recommended.
* ThreadedSPHDomain: Multi threaded with standard namespace threads. Threads spawn at start of simulation. Only gets faster at higher particle counts.
* ThreadPoolSPHDomain: Same as above but maintains the threads through the simulation.
* GLSLSPHDomain: Compute shader SPH. In progress. Currently uses "megakernels" to dish out different kernels in one compute shader. IE: Uses a task id and branching. Wanting to compare it to subroutines as well as separate kernels/shaders for each step. Maybe expirement with CUDA as well.

ThreadPoolSPHDomain should be preferred over ThreadedSPHDomain as it removes the overhead with starting threads by keeping them alive and waiting. For offline computations (not real time) it doesn't matter which threaded implementation you use, as thread overhead is negligible at that point. This repo also includes my own graphics abstraction as a submodule.

Currently working on IISPH implementation.

Paper: http://matthias-mueller-fischer.ch/publications/sca03.pdf

Requires: GLM, GLFW, GLEW

Build with CMake using the provided CMakeLists.txt.

Controls (see Constants.h for parameters & more options):

* Left Click: Rotate View
* Right Click: Interact with simulation
* Scroll: Zoom
* Middle Click: Pan
* Enter: Start/Pause Simulation.
* P: If frame outputting is turned on through Constants.h, p can start/stop frame writing.

![Alt text](https://andaharoo.files.wordpress.com/2018/12/frame_0347-1.png)