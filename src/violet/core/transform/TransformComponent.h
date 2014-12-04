#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "violet/core/component/Component.h"

#include "violet/core/math/Vec2.h"

namespace Violet
{
	class Deserializer;

	class TransformComponent : Component
	{
	public:

		TransformComponent(const Entity & entity);
		TransformComponent(const Entity & entity, Deserializer & deserializer);
		TransformComponent(TransformComponent && other);
		TransformComponent(const TransformComponent &) = delete;

	public:

		Vec2f m_position;
		float m_rotation;
	};
}

#endif