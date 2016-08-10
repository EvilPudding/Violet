#ifndef VIOLET_U32MATH_H
#define VIOLET_U32MATH_H
#define _MATH_DEFINES
#define _MATH_INTERFACE
#elif defined(_MATH_EXT)
#define _MATH_DEFINES
#endif // VIOLET_U32MATH_H

#ifdef _MATH_DEFINES

#include "violet/core/macros.h"
#include "violet/core/types.h"
#include "violet/math/common.h"

#define SCALAR u32
#define SUFFIX u
#define ABS abs

#include "violet/math/templates.h"

#undef _MATH_DEFINES
#endif // _MATH_DEFINES

#ifdef _MATH_INTERFACE
#undef _MATH_INTERFACE
#endif // _MATH_INTERFACE

#if !defined(_MATH_IMPLEMENTATION) && !defined(_MATH_EXT)
#define _MATH_UNDEFINES

#include "violet/math/templates.h"

#undef _MATH_UNDEFINES

#undef SUFFIX
#undef SCALAR
#undef ABS

#endif // _MATH_IMPLEMENTATION

