#pragma once
#include "Engine/MathHelper.h"

static const GLuint FPS = 30;
static const GLuint SUBSTEPS = 5;
static const GLfloat TIMESCALE = 0.5f;
static const GLfloat TIMESTEP = TIMESCALE / (FPS * SUBSTEPS);

static const GLfloat VISCOSITY = 5.0f; // Scales the viscous force
static const GLfloat STIFFNESS = 10.0f; // Scales the interal Pressure
static const GLfloat REST_DENSITY = 998.29f; // Resting density of the fluid kg/m^3 (998 for water)
// Frictional value that specifies how much friction to have against the boundaries, keep low for fluid
static const GLfloat FRICTION = 0.05f;

// Particle diameter in meters
static const GLfloat h = 0.06f;
static const GLfloat r = h * 0.5f;
static const GLfloat PARTICLE_VOLUME = (4.0f / 3.0f) * PI * r * r * r;
static const GLfloat PARTICLE_MASS = PARTICLE_VOLUME * REST_DENSITY;

// Various useful constants for sph kernel
static const GLfloat h2 = h * h;
static const GLfloat h4 = h2 * h2;
static const GLfloat h6 = h4 * h2;
static const GLfloat h9 = h6 * h2 * h;
static const GLfloat poly6Coe = 315.0f / (64.0f * PI * h9);
static const GLfloat spikyGradCoe = -45.0f / (PI * h6);
static const GLfloat polyLapCoe = -spikyGradCoe;

// When this is defined the program will compile to write frames
//#define OUTPUTFRAMES
// Maximum amount of frames allowed to output
//static const GLuint MAXOUTPUTFRAMES = 1000;

// When this is defined the program will compile to write stats to the console every frame
//#define STATS