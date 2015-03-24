#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include "violet/core/system/System.h"
#include "violet/plugins/input/component/InputComponent.h"

namespace Violet
{
	class VIOLET_INPUT_API InputSystem : public ComponentSystem<InputComponent>
	{
	public:

		static void install(SystemFactory & factory);
		static std::unique_ptr<System> init(Deserializer & deserializer);

	public:

		virtual ~InputSystem() override = default;
		virtual void update(float dt, Engine & engine);

	private:

		static void onMouse(int button, int state, int x, int y);
		static void onKeyboardDown(unsigned char key, int x, int y);
		static void onKeyboardUp(unsigned char key, int x, int y);

	private:

		InputSystem() = default;
	};
}

#endif