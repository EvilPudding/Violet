#ifndef VIOLET_ResourceCache_H
#define VIOLET_ResourceCache_H

#include "violet/template/TupleUtilities.h"

#include <unordered_map>
#include <memory>
#include <mutex>

namespace Violet
{
	template <typename Resource, typename ... Args>
	class ResourceCache
	{
	public:

		typedef std::unique_ptr<Resource>(*Loader)(Args ...);

	public:

		ResourceCache(const Loader & loader) :
			m_loader(loader),
			m_resources(),
			m_mutex()
		{
		}

		std::shared_ptr<Resource> fetch(const Args & ... args)
		{
			const std::lock_guard<std::mutex> guard(m_mutex);
			std::shared_ptr<Resource> resource;
			const auto key = std::make_tuple(args...);
			const auto it = m_resources.find(key);
			if (it == m_resources.end())
			{
				resource.reset(Violet::for_all(m_loader, key).release());
				m_resources.emplace_hint(it, std::make_pair(key, resource));
			}
			else
				resource = it->second;
			return resource;
		}

		void clear()
		{
			const std::lock_guard<std::mutex> guard(m_mutex);
			m_resources.clear();
		}

	private:

		Loader m_loader;
		std::unordered_map<std::tuple<Args...>, std::shared_ptr<Resource>> m_resources;
		std::mutex m_mutex;
	};
}

#endif