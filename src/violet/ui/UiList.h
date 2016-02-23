#ifndef VIOLET_UiList_H
#define VIOLET_UiList_H

#include "violet/input/system/inputsystem.h"
#include "violet/transform/component/transformcomponent.h"

namespace Violet
{
	class Engine;
	class Entity;

	class VIOLET_API UiList
	{
	public:

		UiList(const char * elementFileName, uint32 elementHeight);

		void update(const Entity & entity, uint32 elementCount);

		void clean(Script & Script);

	private:

		std::string m_elementFileName;
		const uint32 m_elementHeight;
	};

	DEFINE_METHOD(AssignIndexMethod, void(const Entity &, uint32 index));
}

#endif