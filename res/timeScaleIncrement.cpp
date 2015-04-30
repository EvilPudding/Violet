#include "engine/Engine.h"
#include "engine/window/MouseButton.h"
#include "game/world/WorldSystem.h"
#include "engine/input/InputResult.h"

using namespace Violet;

extern "C" __declspec(dllexport) InputResult onMouseDown(Entity & e, Engine & engine, const MouseButton button)
{
    if (button == MB_Left)
    {
        auto & ws = engine.fetch<WorldSystem>(); 
        ws.setTimeScale(ws.getTimeScale() * 2);
    }

    return InputResult::Block;
}
