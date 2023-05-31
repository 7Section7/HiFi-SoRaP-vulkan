//////////////////////////////////////////////////////////////////////////////
//
//   The main header file for all examples from Angel 6th Edition
//   Renamed to Common.h
//
//////////////////////////////////////////////////////////////////////////////

#ifndef COMMON_H
#define COMMON_H

//----------------------------------------------------------------------------
//
// --- Include system headers ---
//

#include <cmath>
#include <iostream>
#include <stdlib.h>

//  Define M_PI in the case it's not defined in the math header file
#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif

//----------------------------------------------------------------------------
//
// --- Include OpenGL header files and helpers ---
//
//   The location of these files vary by operating system.  We've included
//     copies of open-soruce project headers in the "GL" directory local
//     this this "include" directory.
//

#define GL_GLEXT_PROTOTYPES 1

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#ifdef _WIN32
  #include <windows.h>
#endif
#include <GL/gl.h>
#ifdef _WIN32
  #include <Lib/glew/glext.h>
#else
  #include <GL/glext.h>
#endif
#endif

// Define a macro to modify warnings
#define STRINGIFY(a) #a
#define MODIFY_WARNINGS( parameter ) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wexpansion-to-defined")) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wunused-parameter")) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wunused-variable")) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wignored-attributes")) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wdeprecated-copy")) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wdeprecated-declarations")) \
	_Pragma( STRINGIFY(GCC diagnostic parameter "-Wint-in-bool-context"))

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

//----------------------------------------------------------------------------
//
//  --- Include our class libraries and constants ---
//


namespace Common {


//  Defined constant for when numbers are too small to be used in the
//    denominator of a division operation.  This is only used if the
//    DEBUG macro is defined.
const GLfloat  DivideByZeroTolerance = GLfloat(1.0e-07);

//  Degrees-to-radians constant
const long double DegreesToRadians = M_PI / 180.0L;


}  // namespace Common

#include "vec.h"
#include "mat.h"

#define Print(x)  do { std::cerr << #x " = " << (x) << std::endl; } while(0)


//  Globally use our namespace in our example programs.
using namespace Common;

typedef struct {
	  vector3 pmin;
	  vector3::value_type a, h, p;
} Box3D;

typedef struct {
	  vector2 pmin;
	  vector2::value_type a, h;
} Box2D;


#endif //
