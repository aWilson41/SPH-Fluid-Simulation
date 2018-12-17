# SPH-Fluid-Simulation
SPH Fluid Simulation. See "Particle-Based Fluid Simulation for Interactive Applications" for details.

Paper: http://matthias-mueller-fischer.ch/publications/sca03.pdf

Requires: GLM, GLFW, GLEW

Build with CMake using the provided CMakeLists.txt.

This includes my own graphics abstraction in the "Engine" folder (I might give it it's own repo soon) includes LodePNG for reading/writing png images. See Constants.h for parameters.

Rotate camera with mouse, scroll zoom out, enter to start/stop simulation. If frame outputting is turned on through code, 'p' can start/stop frame writing.

![Alt text](https://andaharoo.files.wordpress.com/2017/12/2017-12-07-17_56_44-animate.png?w=2000&h=)