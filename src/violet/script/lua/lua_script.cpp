// ============================================================================

#include <assert.h>
#include <fstream>
#include <lua.hpp>

#include "violet/script/script_factory.h"
#include "violet/script/lua/lua_library.h"
#include "violet/script/lua/lua_script.h"

using namespace vlt;

namespace LuaScriptNamespace
{
	// ----------------------------------------------------------------------------

	struct BlockStreamReader
	{
	public:

		static const char * readChunk(lua_State * lua, void * data, size_t * size);

	public:

		BlockStreamReader(std::istream & stream, u32 bufferSize);
		~BlockStreamReader();

	public:

		const char * read(size_t & size);

	private:

		std::istream & m_stream;
		u32 const m_bufferSize;
		char * const m_buffer;
	};

	// ----------------------------------------------------------------------------

	std::unique_ptr<Script> createFromFile(const char * fileName);

	// ----------------------------------------------------------------------------
}

using namespace LuaScriptNamespace;

// ============================================================================

void Lua_Script::install()
{
	Script_Factory::assign("lua", createFromFile);
}

// ============================================================================

Lua_Script::Lua_Script(shared_val<Text_Resource> source) :
	m_source(std::move(source)),
	m_lua(),
	m_valid(false)
{
	load();
}

// ----------------------------------------------------------------------------

Lua_Script::Lua_Script(Lua_Script && other) :
	m_source(std::move(other.m_source)),
	m_lua(other.m_lua),
	m_valid(other.m_valid)
{
	other.m_lua = nullptr;
	other.m_valid = false;
}

// ----------------------------------------------------------------------------

Lua_Script::~Lua_Script()
{
	if (m_valid)
		unload();
}

// ----------------------------------------------------------------------------

std::string const & Lua_Script::get_filename() const
{
	return m_source->name;
}

// ----------------------------------------------------------------------------

bool Lua_Script::is_valid() const
{
	return m_valid;
}

// ----------------------------------------------------------------------------

void Lua_Script::reload()
{
	if (m_valid)
		unload();
	load();
}

// ============================================================================

void Lua_Script::load()
{
	m_lua = luaL_newstate();
	m_valid = false;

	luaL_openlibs(m_lua);
	LuaLibrary::open(m_lua);

	auto istream = m_source->read();
	if (istream->good())
	{
		BlockStreamReader reader(istream, 512);
		if (lua_load(m_lua, BlockStreamReader::readChunk, &reader, m_source->name.c_str(), "bt") == LUA_OK)
		{
			if (lua_pcall(m_lua, 0, 0, 0) == LUA_OK)
			{
				lua_getglobal(m_lua, "init");
				if (lua_pcall(m_lua, 0, 0, 0) == LUA_OK)
					m_valid = true;
				else
				{
					const char * error = lua_tostring(m_lua, -1);
					printf("%s\n", error);
				}
			}
			else
			{
				const char * error = lua_tostring(m_lua, -1);
				printf("%s compilation error: %s\n", m_source->name.c_str(), error);
				lua_pop(m_lua, 1);
			}
		}
	}

	if (!m_valid)
	{
		lua_close(m_lua);
		m_lua = nullptr;
	}
}

// ----------------------------------------------------------------------------

void Lua_Script::unload()
{
	lua_getglobal(m_lua, "clean");
	lua_pcall(m_lua, 0, 0, 0);
	lua_close(m_lua);
	m_valid = false;
}

// ============================================================================

LuaScriptNamespace::BlockStreamReader::BlockStreamReader(std::istream & stream, u32 bufferSize) :
	m_stream(stream),
	m_bufferSize(bufferSize),
	m_buffer(new char[bufferSize])
{
}

// ----------------------------------------------------------------------------

LuaScriptNamespace::BlockStreamReader::~BlockStreamReader()
{
	delete[] m_buffer;
}

// ----------------------------------------------------------------------------

const char * LuaScriptNamespace::BlockStreamReader::read(size_t & size)
{
	if (!m_stream)
		return nullptr;

	m_stream.read(m_buffer, m_bufferSize - 1);
	size = static_cast<u32>(m_stream.gcount());
	m_buffer[size] = 0;
	return m_buffer;
}

// ----------------------------------------------------------------------------

const char * LuaScriptNamespace::BlockStreamReader::readChunk(lua_State * /*lua*/, void * data, size_t * size)
{
	auto reader = static_cast<BlockStreamReader *>(data);
	return reader->read(*size);
}

// ============================================================================

std::unique_ptr<Script> LuaScriptNamespace::createFromFile(const char * const fileName)
{
	auto file = make_shared_val<File_Resource>(fileName);
	return std::make_unique<Lua_Script>(file);
}

// ============================================================================
