#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include "violet/core/math/Polygon.h"
#include "violet/plugins/graphics/Color.h"

#include <memory>

namespace Violet
{
	class RenderComponent
	{
	public:

		Polygon m_mesh;
		Color m_color;
	};
}

#endif