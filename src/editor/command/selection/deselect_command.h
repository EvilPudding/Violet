#ifndef EDITOR_DESELECT_COMMAND_H
#define EDITOR_DESELECT_COMMAND_H

#include <memory>
#include <string>
#include <vector>

#include "editor/command/command.h"
#include "violet/core/defines.h"
#include "violet/core/handle.h"

namespace edt
{
	struct EDITOR_API Deselect_Command : public Command
	{
		std::vector<vlt::Handle> proxy_ids;

		static const char * get_usage();
		static std::unique_ptr<Command> parse(const std::string & text);

		explicit Deselect_Command(vlt::Handle proxy_id);
		explicit Deselect_Command(const std::vector<vlt::Handle> & proxy_ids);
		explicit Deselect_Command(std::vector<vlt::Handle> && proxy_ids);
		virtual ~Deselect_Command() override = default;

		virtual void execute() override;
		virtual bool can_undo() const override;
		virtual void undo() override;
	};
}

#endif
