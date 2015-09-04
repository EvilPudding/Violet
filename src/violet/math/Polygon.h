#ifndef VIOLET_Polygon_H
#define VIOLET_Polygon_H

#include <vector>

#include "violet/math/Vec2.h"
#include "violet/math/AABB.h"

namespace Violet
{
	class VIOLET_API Polygon
	{
	public:

		explicit Polygon(std::vector<Vec2f> && vertices);
		explicit Polygon(Deserializer & deserializer);

		bool contains(const Vec2f & point) const;
		AABB getBoundingBox() const;

		void translate(const Vec2f delta);

		FloatInterval project(const Vec2f & axis) const;
		Vec2f getCenter() const;


	public:

		std::vector<Vec2f> m_vertices;
	};

	VIOLET_API Serializer & operator<<(Serializer & serializer, const Polygon & poly);
	VIOLET_API Deserializer & operator>>(Deserializer & deserializer, Polygon & poly);
}

#endif
