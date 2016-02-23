#ifndef GAME_Map_H
#define GAME_Map_H

#include "violet/math/Vec2.h"
#include "violet/structures/crsgraph.h"
#include "game/config.h"

class Intersection
{
public:

	Vec2f m_position;
};

class VIOLET_GAME_API Road
{
public:

	uint32 getSource() const;

public:

	uint32 m_src;
	uint32 m_destination;
};

class Map : public Violet::CrsGraph<Intersection, Road> {};

#endif