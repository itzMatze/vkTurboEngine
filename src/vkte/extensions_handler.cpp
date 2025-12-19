#include "vkte/extensions_handler.hpp"

#include <cstring>

namespace vkte
{
const std::vector<const char*>& ExtensionsHandler::get_missing_extensions() const
{
	return missing_extensions;
}

const std::vector<const char*>& ExtensionsHandler::get_extensions() const
{
	return extensions;
}

uint32_t ExtensionsHandler::get_size() const
{
	return extensions.size();
}

void ExtensionsHandler::add_extensions(const std::vector<const char*>& requested_extensions)
{
	extensions.insert(extensions.end(), requested_extensions.begin(), requested_extensions.end());
}

bool ExtensionsHandler::check_extension_availability(const std::vector<const char*>& available_extensions)
{
	for (uint32_t i = 0; i < extensions.size(); ++i)
	{
		if (!find_extension(extensions[i], available_extensions)) return false;
	}
	return true;
}

bool ExtensionsHandler::find_extension(const char* name) const
{
	return find_extension(name, extensions);
}

bool ExtensionsHandler::find_extension(const char* name, const std::vector<const char*>& extensions) const
{
	for (const auto& ext : extensions)
	{
		if (strcmp(ext, name) == 0)
		{
			return true;
		}
	}
	return false;
}
} // namespace vkte
