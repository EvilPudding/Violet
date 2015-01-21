#include "violet/core/script/CppScript.h"

#include "violet/core/Defines.h"
#include "violet/core/script/ScriptFactory.h"

#include <iostream>
#include <set>

using namespace Violet;

namespace CppScriptNamespace
{
	std::unique_ptr<Script> create(const char * filename);

	typedef void (* Proc)(const Entity &, AlterContext & context);

	void defaultProc(const Entity &);

	const char * const ms_extension = "dll";

	std::set<std::string> ms_missingProcedures;
}

using namespace CppScriptNamespace;

void CppScript::install()
{
	ScriptFactory::getInstance().assign(ms_extension, &create);
}

CppScript::CppScript(const char * filename) :
	m_lib(LoadLibrary(filename))
{
	if (m_lib == nullptr)
		std::cout << "Error loading script: " << filename << std::endl;
}

CppScript::~CppScript()
{
	FreeLibrary(m_lib);
}

void CppScript::run(const char * procedure, const Entity & entity, AlterContext & context) const
{
	auto proc = (Proc)GetProcAddress(m_lib, procedure);
	if (proc != nullptr)
		proc(entity, context);
	else
	{
		static const uint32 bufferSize = 64;
		char filename[bufferSize];
		GetModuleFileName(m_lib, filename, bufferSize);
		std::string fileAndProcedure(filename);
		fileAndProcedure.append(procedure);
		auto it = ms_missingProcedures.find(fileAndProcedure);
		if (it == ms_missingProcedures.end())
		{
			std::cout << "Error loading function " << procedure << " in script " << filename << std::endl;
			ms_missingProcedures.insert(fileAndProcedure);
		}
	}
}

std::unique_ptr<Script> CppScriptNamespace::create(const char * filename)
{
	return std::unique_ptr<Script>(new CppScript(filename));
}

void CppScriptNamespace::defaultProc(const Entity &)
{
}