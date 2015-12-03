// ============================================================================

#include "editor/EditorSystem.h"

#include "violet/Engine.h"
#include "violet/graphics/component/ColorComponent.h"
#include "violet/graphics/component/TextComponent.h"
#include "violet/graphics/component/TextureComponent.h"
#include "violet/input/component/KeyInputComponent.h"
#include "violet/input/component/MouseInputComponent.h"
#include "violet/log/Log.h"
#include "violet/scene/Scene.h"
#include "violet/script/ScriptComponent.h"
#include "violet/serialization/file/FileDeserializerFactory.h"
#include "violet/serialization/Deserializer.h"
#include "violet/system/SystemFactory.h"
#include "violet/transform/component/TransformComponent.h"
#include "violet/update/component/UpdateComponent.h"
#include "violet/utility/FormattedString.h"
#include "violet/window/WindowSystem.h"

using namespace Violet;

// ============================================================================

namespace EditorSystemNamespace
{
	template <typename ComponentType>
	class EditorComponentWrapper : public ComponentBase<EditorComponentWrapper<ComponentType>>
	{
	public:

		static void create(Entity & entity, Deserializer & deserializer)
		{
			entity.addComponent<EditorComponentWrapper<ComponentType>>(deserializer);
		}

		static Tag getStaticTag() { return Tag('e', 'd', 't', 'r'); }

	public:

		EditorComponentWrapper(Entity & owner, Deserializer & deserializer) :
			ComponentBase<EditorComponentWrapper<ComponentType>>(owner),
			m_data(owner, deserializer)
		{
		}

		EditorComponentWrapper(EditorComponentWrapper && other) :
			ComponentBase<EditorComponentWrapper<ComponentType>>(std::move(other)),
			m_data(std::move(other.m_data))
		{
		}

		ComponentType m_data;
	};

	template <typename ComponentType>
	Serializer & operator<<(Serializer & serializer, const EditorComponentWrapper<ComponentType> & component)
	{
		return serializer << component.m_data;
	}

	void prepareForEditor(Entity & entity, std::string const & editScriptFileName);
}

using namespace EditorSystemNamespace;

// ============================================================================

const char * EditorSystem::getStaticLabel()
{
	return "edtr";
}

// ----------------------------------------------------------------------------

void EditorSystem::install(SystemFactory & factory)
{
	factory.assign(getStaticLabel(), &EditorSystem::init);
}

// ----------------------------------------------------------------------------

void EditorSystem::init(Deserializer & deserializer)
{
	auto segment = deserializer.enterSegment(getStaticLabel());
	const std::string editScriptFileName = segment->getString("editScriptFileName");

	Engine::getInstance().addWriteTask(Engine::getInstance(),
		[=](Engine & engine)
		{
			engine.addSystem(std::unique_ptr<System>(new EditorSystem(std::move(editScriptFileName))));
		});
}

// ============================================================================

void EditorSystem::update(const float /*dt*/, const Engine & /*engine*/)
{
}

// ----------------------------------------------------------------------------

void EditorSystem::loadScene(const char * const filename, const Engine & engine)
{
	if (m_rootSceneHandle.isValid())
	{
		// cleanup
		assert(false);
	}

	const std::string sceneFileName(filename);
	const std::string editScriptFileName = m_editScriptFileName;
	engine.addWriteTask(engine.getCurrentScene().getRoot(), [=](Entity & root)
		{
			Entity::uninstallComponent<MouseInputComponent>();
			Entity::uninstallComponent<KeyInputComponent>();
			Entity::uninstallComponent<ScriptComponent>();
			Entity::uninstallComponent<UpdateComponent>();

			Entity::installComponent(MouseInputComponent::getStaticTag(), &EditorComponentWrapper<MouseInputComponent>::create);
			Entity::installComponent(KeyInputComponent::getStaticTag(), &EditorComponentWrapper<KeyInputComponent>::create);
			Entity::installComponent(ScriptComponent::getStaticTag(), &EditorComponentWrapper<ScriptComponent>::create);
			Entity::installComponent(UpdateComponent::getStaticTag(), &EditorComponentWrapper<UpdateComponent>::create);

			Entity * newSceneRoot = nullptr;
			auto deserializer = FileDeserializerFactory::getInstance().create(sceneFileName.c_str());
			if (deserializer == nullptr)
				Log::log(FormattedString<128>().sprintf("Could not open scene file '%s'", sceneFileName.c_str()));
			else if (!*deserializer)
				Log::log(FormattedString<128>().sprintf("Failed to parse scene file '%s'", sceneFileName.c_str()));
			else
				newSceneRoot = &root.addChild(*deserializer);

			Entity::uninstallComponent(MouseInputComponent::getStaticTag());
			Entity::uninstallComponent(KeyInputComponent::getStaticTag());
			Entity::uninstallComponent(ScriptComponent::getStaticTag());
			Entity::uninstallComponent(UpdateComponent::getStaticTag());

			Entity::installComponent<MouseInputComponent>();
			Entity::installComponent<KeyInputComponent>();
			Entity::installComponent<ScriptComponent>();
			Entity::installComponent<UpdateComponent>();

			if (newSceneRoot != nullptr)
			{
				const auto & engine = Engine::getInstance();
				const float halfWidth = engine.getSystem<WindowSystem>()->getWidth() / 2.f;
				const float halfHeight = engine.getSystem<WindowSystem>()->getHeight() / 2.f;
				newSceneRoot->addComponent<HandleComponent>();
				newSceneRoot->addComponent<TransformComponent>();
				newSceneRoot->addComponent<MouseInputComponent>(Polygon{ {
					{ -halfWidth, -halfHeight },
					{ -halfWidth, halfHeight },
					{ halfWidth, halfHeight },
					{ halfWidth, -halfHeight }
					} });
				engine.addReadTask(std::make_unique<DelegateTask>(
					[this, newSceneRoot]()
					{
						m_rootSceneHandle = newSceneRoot->getComponent<HandleComponent>()->getHandle();
					}));
				for (auto & child : newSceneRoot->getChildren())
					prepareForEditor(child, editScriptFileName);
			}
		});
}

// ----------------------------------------------------------------------------

bool EditorSystem::hasScene() const
{
	return m_rootSceneHandle.isValid();
}

// ----------------------------------------------------------------------------

lent_ptr<const Entity> EditorSystem::getSceneRoot() const
{
	return Engine::getInstance().getCurrentScene().getEntity(m_rootSceneHandle);
}

// ============================================================================

EditorSystem::EditorSystem(std::string editScriptFileName) :
	System(getStaticLabel()),
	m_rootSceneHandle(),
	m_editScriptFileName(std::move(editScriptFileName))
{
}

// ============================================================================

void EditorSystemNamespace::prepareForEditor(Entity & entity, std::string const & editScriptFileName)
{
	if (!editScriptFileName.empty())
	{
		if (entity.hasComponent<ColorComponent>())
		{
			Engine::getInstance().addWriteTask(*entity.getComponent<ColorComponent>(),
				[=](ColorComponent & component)
				{
					component.getOwner().addComponent<ScriptComponent>(editScriptFileName.c_str());
					component.getOwner().addComponent<MouseInputComponent>(component.m_mesh->getPolygon());
				}, Engine::Thread::Window);
		}
		else if (entity.hasComponent<TextComponent>())
		{
			Engine::getInstance().addWriteTask(*entity.getComponent<TextComponent>(),
				[=](TextComponent & component)
				{
					component.getOwner().addComponent<ScriptComponent>(editScriptFileName.c_str());
					component.getOwner().addComponent<MouseInputComponent>(component.m_mesh->getPolygon());
				}, Engine::Thread::Window);
		}
		else if (entity.hasComponent<TextureComponent>())
		{
			Engine::getInstance().addWriteTask(*entity.getComponent<TextureComponent>(),
				[=](TextureComponent & component)
				{
					component.getOwner().addComponent<ScriptComponent>(editScriptFileName.c_str());
					component.getOwner().addComponent<MouseInputComponent>(component.m_mesh->getPolygon());
				}, Engine::Thread::Window);
		}
	}
}

// ============================================================================
