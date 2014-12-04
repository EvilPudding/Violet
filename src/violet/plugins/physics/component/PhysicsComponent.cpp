#include "violet/plugins/physics/component/PhysicsComponent.h"

#include "violet/core/serialization/Deserializer.h"

using namespace Violet;

namespace PhysicsComponentNamespace
{
	const char * const ms_massLabel = "mass";

	float calculateMomentOfInertia(const Polygon & polygon, float mass);
}

using namespace PhysicsComponentNamespace;

PhysicsComponent::PhysicsComponent(const Entity & entity, Deserializer & deserializer) :
	Component(entity),
	m_polygon(deserializer),
	m_mass(deserializer.getFloat(ms_massLabel)),
	m_velocity(),
	m_force(),
	m_momentOfInertia(calculateMomentOfInertia(m_polygon, m_mass)),
	m_angularVelocity(),
	m_torque()
{
}

PhysicsComponent::PhysicsComponent(PhysicsComponent && other) :
	Component(std::move(other)),
	m_polygon(std::move(other.m_polygon)),
	m_mass(other.m_mass),
	m_velocity(std::move(other.m_velocity)),
	m_force(std::move(other.m_force)),
	m_momentOfInertia(other.m_momentOfInertia),
	m_angularVelocity(other.m_angularVelocity),
	m_torque(other.m_torque)
{
}

float PhysicsComponentNamespace::calculateMomentOfInertia(const Polygon & polygon, const float mass)
{
	float area = 0;
	float numerator = 0;
	float denominator = 0;
	const uint32 size = polygon.m_vertices.size();
	for (uint32 i = 0; i < size; ++i)
	{
		const Vec2f & vertex = polygon.m_vertices[i];
		const Vec2f & nextVertex = polygon.m_vertices[i < size - 1 ? i + 1 : 0];
		Vec2f perp = vertex.perpendicular();
		const float height = abs(nextVertex.dot(perp));
		const float squareArea = height * vertex.magnitude();
		area += 0.5f * squareArea;
		numerator += squareArea*(nextVertex.magSquared() + nextVertex.dot(vertex) + vertex.magSquared());
		denominator += squareArea;
	}
	return (mass / 6) * (numerator / denominator);
}