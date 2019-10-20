# SPH-Fluid-Simulation
SPH Fluid Simulation. See "Particle-Based Fluid Simulation for Interactive Applications" for details. Contains 4 versions of SPH implemented mostly entirely separately for readability.

* SPHDoimain: Single thread. No binning. Nice for learning. Not recommended.
* ThreadedSPHDomain: Multi threaded with standard namespace threads. Threads spawn at start of simulation. Only gets faster at higher particle counts.
* ThreadPoolSPHDomain: Same as above but maintains the threads through the simulation.
* GLSLSPHDomain: Compute shader SPH. In progress.

Repository designed with readability in mind so code is almost entirely duplicate for each of these 4 methods. Best to learn from SPHDomain. ThreadPoolSPHDomain should be preferred over ThreadedSPHDomain as it removes the overhead with starting threads by keep them alive and waiting. For offline computations (not real time) it wouldn't really matter which you use.

Currently working on IISPH implementation.

Paper: http://matthias-mueller-fischer.ch/publications/sca03.pdf

Requires: GLM, GLFW, GLEW

Build with CMake using the provided CMakeLists.txt.

This also includes my own graphics abstraction GLEngine as a submodule. See Constants.h for parameters and controls of the simulation.

Controls:

* Left Click: Rotate View
* Right Click: Interact with simulation
* Scroll: Zoom
* Middle Click: Pan
* Enter: Start/Pause Simulation.
* P: If frame outputting is turned on through Constants.h, p can start/stop frame writing.

![Alt text](https://andaharoo.files.wordpress.com/2018/12/frame_0347-1.png)