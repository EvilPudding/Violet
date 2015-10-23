// ============================================================================

#include "violet/window/sdl/SDLWindowSystem.h"

#include "violet/Engine.h"
#include "violet/graphics/system/RenderSystem.h"
#include "violet/serialization/Deserializer.h"
#include "violet/system/SystemFactory.h"

#include <SDL.h>

#include <assert.h>
#include <iostream>

using namespace Violet;

// ============================================================================

namespace SDLWindowSystemNamespace
{
	char convertKey(SDL_Keycode key);
	int filterEvent(void * userdata, SDL_Event * event);
}

using namespace SDLWindowSystemNamespace;

// ============================================================================

void SDLWindowSystem::install(SystemFactory & factory)
{
	factory.assign(getStaticLabel(), &SDLWindowSystem::init);
}

// ----------------------------------------------------------------------------

std::unique_ptr<System> SDLWindowSystem::init(Deserializer & deserializer)
{
	auto settingsSegment = deserializer.enterSegment(getStaticLabel());

	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL_Init(VIDEO) failed: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	int const x = settingsSegment->getInt("x");
	int const y = settingsSegment->getInt("y");
	int const width = settingsSegment->getInt("width");
	int const height = settingsSegment->getInt("height");
	const char * title = settingsSegment->getString("title");

	//Use OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Window * window = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_OPENGL);
	if (window == nullptr)
	{
		std::cout << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (glContext == nullptr)
	{
		std::cout << "SDL_CreateContext failed: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	SDL_SetEventFilter(filterEvent, nullptr);

	return std::unique_ptr<System>(new SDLWindowSystem(window, glContext));
}

// ============================================================================

SDLWindowSystem::~SDLWindowSystem()
{
	SDL_GL_DeleteContext(m_glContext);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

// ----------------------------------------------------------------------------

void SDLWindowSystem::render()
{
	SDL_GL_SwapWindow(m_window);
}

// ----------------------------------------------------------------------------

void SDLWindowSystem::update(float /*dt*/, const Engine & /*engine*/)
{
}

// ----------------------------------------------------------------------------

bool SDLWindowSystem::getEvent(const EventType type, Event * const event)
{
	bool queueEmpty = false;
	bool foundEvent = false;

	while (!queueEmpty && !foundEvent)
	{
		SDL_Event sdlEvent;
		if (SDL_PollEvent(&sdlEvent) == 1)
		{
			switch (sdlEvent.type)
			{
			case SDL_KEYDOWN:
				if (type & ET_KeyDown)
				{
					event->type = WindowSystem::ET_KeyDown;
					event->key.code = convertKey(sdlEvent.key.keysym.sym);
					foundEvent = true;
				}
				break;

			case SDL_KEYUP:
				if (type & ET_KeyUp)
				{
					event->type = WindowSystem::ET_KeyUp;
					event->key.code = convertKey(sdlEvent.key.keysym.sym);
					foundEvent = true;
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (type & ET_MouseDown)
				{
					event->type = WindowSystem::ET_MouseDown;
					event->mouse.x = sdlEvent.button.x;
					event->mouse.y = sdlEvent.button.y;
					event->mouse.button = static_cast<MouseButton>(sdlEvent.button.button - 1);
					foundEvent = true;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if (type & ET_MouseUp)
				{
					event->type = WindowSystem::ET_MouseUp;
					event->mouse.x = sdlEvent.button.x;
					event->mouse.y = sdlEvent.button.y;
					event->mouse.button = static_cast<MouseButton>(sdlEvent.button.button - 1);
					foundEvent = true;
				}
				break;

			case SDL_MOUSEMOTION:
				if (type & ET_MouseMove)
				{
					event->type = WindowSystem::ET_MouseMove;
					event->motion.x = sdlEvent.motion.x;
					event->motion.y = sdlEvent.motion.y;
					event->motion.xrel = sdlEvent.motion.xrel;
					event->motion.yrel = sdlEvent.motion.yrel;
					foundEvent = true;
				}
				break;

			case SDL_QUIT:
				if (type & ET_Quit)
				{
					event->type = WindowSystem::ET_Quit;
					foundEvent = true;
				}
				break;
			}
		}
		else
			queueEmpty = true;
	}

	return foundEvent;
}

// ----------------------------------------------------------------------------

void SDLWindowSystem::addEvent(Event event)
{
	SDL_Event sdlEvent;
	switch (event.type)
	{
	case WindowSystem::ET_KeyDown:
		sdlEvent.type = SDL_KEYDOWN;
		sdlEvent.key.keysym.sym = static_cast<SDL_Keycode>(event.key.code);
		break;

	case WindowSystem::ET_KeyUp:
		sdlEvent.type = SDL_KEYUP;
		sdlEvent.key.keysym.sym = static_cast<SDL_Keycode>(event.key.code);
		break;

	case WindowSystem::ET_MouseUp:
		sdlEvent.type = SDL_MOUSEBUTTONUP;
		sdlEvent.button.x = event.mouse.x;
		sdlEvent.button.y = event.mouse.y;
		break;

	case WindowSystem::ET_MouseDown:
		sdlEvent.type = SDL_MOUSEBUTTONDOWN;
		sdlEvent.button.x = event.mouse.x;
		sdlEvent.button.y = event.mouse.y;
		break;

	case WindowSystem::ET_MouseMove:
		sdlEvent.type = SDL_MOUSEMOTION;
		sdlEvent.motion.x = event.motion.x;
		sdlEvent.motion.y = event.motion.y;
		sdlEvent.motion.xrel = event.motion.xrel;
		sdlEvent.motion.yrel = event.motion.yrel;
		break;
	}
	SDL_PushEvent(&sdlEvent);
}

// ----------------------------------------------------------------------------

int SDLWindowSystem::getWidth() const
{
	return m_width;
}

// ----------------------------------------------------------------------------

int SDLWindowSystem::getHeight() const
{
	return m_height;
}

// ============================================================================

SDLWindowSystem::SDLWindowSystem(SDL_Window * window, SDL_GLContext context) :
	WindowSystem(),
	m_window(window),
	m_glContext(context),
	m_width(0),
	m_height(0)
{
	SDL_GetWindowSize(m_window, &m_width, &m_height);
}

// ============================================================================

char SDLWindowSystemNamespace::convertKey(SDL_Keycode const key)
{
	if (key == 10)
		return 13;
	else if (key >= 0 && key <= 255)
		return key;
	else if (key >= SDLK_KP_1 && key <= SDLK_KP_0)
	{
		static char keys[1 + SDLK_KP_0 - SDLK_KP_1] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
		return keys[key - SDLK_KP_1];
	}

	return 0;
}

// ----------------------------------------------------------------------------

int SDLWindowSystemNamespace::filterEvent(void * /*userdata*/, SDL_Event * event)
{
	switch (event->type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_QUIT:
		return 1;

	default:
		return 0;
		break;
	}
}

// ============================================================================