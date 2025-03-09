/*
** Agartha-Software, 2025
** C++evy
** File description:
**  cross platform opengl helpers
*/

#pragma once

#if (_WIN32)
#include <GL/gl3w.h>
#include <GL/gl.h>

#ifdef min
#undef min
#endif // min
#ifdef max
#undef max
#endif // max

#ifdef far
#undef far
#endif // far
#ifdef near
#undef near
#endif // near

#endif
#if (__linux__)
#include <GL/glew.h>
#include <GL/gl.h>
#endif