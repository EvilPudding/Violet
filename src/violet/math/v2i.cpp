// ============================================================================

#include "violet/math/v2i.h"

using namespace vlt;

// ============================================================================

v2i::v2i() :
	x(0),
	y(0)
{
}

// ----------------------------------------------------------------------------

v2i::v2i(const v2i & rhs) :
	x(rhs.x),
	y(rhs.y)
{
}

// ----------------------------------------------------------------------------


v2i::v2i(const s32 _x, const s32 _y) :
	x(_x),
	y(_y)
{
}

// ============================================================================
