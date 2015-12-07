#include "violet/Engine.h"
#include "violet/component/ComponentManager.h"
#include "violet/input/Key.h"
#include "violet/input/component/KeyInputComponent.h"
#include "violet/input/system/InputSystem.h"
#include "violet/graphics/component/TextComponent.h"
#include "violet/script/ScriptComponent.h"
#include "violet/script/cpp/CppScript.h"
#include "violet/utility/FormattedString.h"

#include "dialog.h"

#include <cctype>
#include <functional>

using namespace Violet;
using namespace std::placeholders;

class Instance : public CppScript::Instance
{
public:

    Instance(CppScript & script) :
        CppScript::Instance(script)
    {
        KeyUpMethod::assign(script, std::bind(&Instance::onKeyUp, this, _1, _2));
        MouseDownMethod::assign(script, std::bind(&Instance::onMouseDown, this, _1, _2));
    }

    virtual ~Instance() override
    {
        MouseDownMethod::remove(m_script);
        KeyUpMethod::remove(m_script);
    }

private:

    InputResult onMouseDown(const Handle entityId, const InputSystem::MouseButtonEvent & event)
    {
        const Engine & engine = Engine::getInstance();
        if (!engine.getCurrentScene().hasComponent<KeyInputComponent>(entityId))
        {
            Engine::getInstance().addWriteTask(engine.getCurrentScene(),
                [=](ComponentManager & scene)
                {
                    scene.createComponent<KeyInputComponent>(entityId);
                });
        }

        return InputResult::Block;
    }

    void onKeyUp(const Handle entityId, const unsigned char key)
    {
        const Engine & engine = Engine::getInstance();
        if (std::isalpha(key) || key == '.')
        {
            const auto * tc = engine.getCurrentScene().getComponent<TextComponent>(entityId);
            if (tc != nullptr)
            {
                engine.addWriteTask(*tc, [key](TextComponent & tc)
                    {
                        tc.m_text += key;
                    });
            }
        }
        else if (key == Key::Backspace)
        {
            const auto * tc = engine.getCurrentScene().getComponent<TextComponent>(entityId);
            if (tc != nullptr && !tc->m_text.empty())
            {
                engine.addWriteTask(*tc, [](TextComponent & tc)
                    {
                        tc.m_text.pop_back();
                    });
            }
        }
        else if (key == Key::Return)
        {
            const auto * tc = engine.getCurrentScene().getComponent<TextComponent>(entityId);
            if (tc != nullptr && !tc->m_text.empty())
            {
                DialogClosedEvent::emit(engine, tc->m_text);
                engine.addWriteTask(engine.getCurrentScene(),
                    [=](ComponentManager & manager)
                    {
                        manager.remove<KeyInputComponent>(entityId);
                        Engine::getInstance().getCurrentScene().removeAll(entityId);
                    });
            }
        }
        else
            printf("%d\n", key);
    }
};

VIOLET_SCRIPT_EXPORT void init(CppScript & script, std::unique_ptr<CppScript::Instance> & instance)
{
    instance = std::make_unique<Instance>(script);
}

