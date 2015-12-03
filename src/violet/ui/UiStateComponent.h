#ifndef VIOLET_UiStateComponent_H
#define VIOLET_UiStateComponent_H

#include "violet/component/Component.h"
#include "violet/utility/unique_val.h"
#include "violet/handle/Handle.h"

#include <string>
#include <vector>

namespace Violet
{
	class Deserializer;
	class Serializer;

	class VIOLET_API UiStateComponent : public ComponentBase<UiStateComponent>
	{
	public:

		VIOLET_API friend Serializer & operator<<(Serializer & serializer, const UiStateComponent & component);
		VIOLET_API friend Deserializer & operator>>(Deserializer & deserializer, UiStateComponent & component);

	public:

		static Tag getStaticTag();

	public:

		UiStateComponent(Entity & owner, Deserializer & deserializer);

		uint32 getCurrentIndex() const;
		void setCurrentIndex(uint32 index);

	private:

		uint32 m_currentIndex;
		Handle m_currentHandle;
		std::vector<unique_val<Entity>> m_states;
	};

    VIOLET_API Serializer & operator<<(Serializer & serializer, const UiStateComponent & component);
	VIOLET_API Deserializer & operator>>(Deserializer & deserializer, UiStateComponent & component);
}

#endif
