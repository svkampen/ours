#ifndef NM_EVENTMAP_HPP
#define NM_EVENTMAP_HPP

#include <map>
#include <functional>
#include <boost/signals2.hpp>
#include <netmine.pb.h>

namespace nm
{
	#define MessageType(X) nm::message::MessageWrapper_Type_##X

	template<typename Types, typename ...Data>
	class EventMap
	{
		public:
		std::map<Types,
			boost::signals2::signal<void (Data...)>> _map;

		void connect(Types t, std::function<void (Data...)> fn)
		{
			if(_map.find(t) == _map.end())
				_map[t] = {};
			_map[t].connect(fn);
		}

		void fire(Types t, Data... data)
		{
			auto it = _map.find(t);
			if (it != _map.end())
			{
				it->second(data...);
			}
		}

	};
}

#endif
