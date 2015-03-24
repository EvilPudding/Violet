#ifndef SETUP_H
#define SETUP_H

#include "violet/core/script/CppScript.h"
#include "violet/core/script/system/ScriptSystem.h"
#include "violet/core/system/SystemFactory.h"
#include "violet/core/transform/TransformSystem.h"
#include "violet/extras/serialization/JsonDeserializer.h"
#include "violet/plugins/graphics/system/RenderSystem.h"
#include "violet/plugins/physics/system/PhysicsSystem.h"
#include "violet/plugins/input/system/InputSystem.h"
#include "violet/plugins/update/system/UpdateSystem.h"

SystemFactory setup(int argc, char ** argv)
{
	Violet::JsonDeserializer::install();

	Violet::CppScript::install();

	Violet::SystemFactory factory;
	Violet::TransformSystem::install(factory);
	Violet::PhysicsSystem::install(factory);
	Violet::RenderSystem::install(factory);
	Violet::InputSystem::install(factory);
	Violet::ScriptSystem::install(factory);
	Violet::UpdateSystem::install(factory);

	return factory;
}

#endif