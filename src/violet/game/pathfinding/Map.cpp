#include "violet/game/pathfinding/Map.h"

#include "violet/core/serialization/Deserializer.h"

uint32 Map::Road::getSource() const
{
	return m_src;
}

Map::Map() :
	m_graph()
{
}

Map::Map(Deserializer & deserializer) :
	m_graph()
{
	auto const & mapSegment = deserializer.enterSegment("map");
	const uint32 roadWidth = mapSegment->getUint("roadWidth");
	{
		auto nodesSegment = mapSegment->enterSegment("nodes");
		const uint32 n = nodesSegment->getUint("n");
		auto listSegment = nodesSegment->enterSegment("nodes");
		for (uint32 i = 0; i < n; ++i)
			m_graph.addNode({ Vec2f(*listSegment->enterSegment(nullptr)) });
	}

	{
		auto edgesSegment = mapSegment->enterSegment("edges");
		const uint32 n = edgesSegment->getUint("n");
		auto listSegment = edgesSegment->enterSegment("edges");
		for (uint32 i = 0; i < n; ++i)
		{
			auto edgeSegment = listSegment->enterSegment("edge");
			const uint32 src = edgeSegment->getUint("src");
			const uint32 dst = edgeSegment->getUint("dst");
			m_graph.addEdge({ src, dst });
		}
	}
}

Violet::CrsGraph<Map::Intersection, Map::Road> & Map::getGraph()
{
	return m_graph;
}

const Violet::CrsGraph<Map::Intersection, Map::Road> & Map::getGraph() const
{
	return m_graph;
}