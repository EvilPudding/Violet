#ifndef VIOLET_ScriptComponent_H
#define VIOLET_ScriptComponent_H

#include "violet/component/Component.h"

#include "violet/script/Script.h"

#include <memory>

namespace Violet
{
	class Deserializer;
	class Serializer;

	class VIOLET_API ScriptComponent : public ComponentBase<ScriptComponent>
	{
	public:

		static Tag getStaticTag();

	public:

		ScriptComponent(Entity & owner, Deserializer & deserializer);
		ScriptComponent(Entity & owner, const char * fileName);
		ScriptComponent(ScriptComponent && other);
		virtual ~ScriptComponent() override;

	public:

		std::unique_ptr<Script> m_script;
	};

	VIOLET_API Serializer & operator<<(Serializer & serializer, const ScriptComponent & component);

	DEFINE_METHOD(BindToComponentMethod, void(Entity & owner));
	DEFINE_METHOD(UnbindFromComponentMethod, void(Entity & owner));
}

#endif