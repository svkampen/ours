#ifndef NM_EVENTMAP_HPP
#define NM_EVENTMAP_HPP

#include <boost/signals2.hpp>
#include <functional>
#include <netmine.pb.h>
#include <unordered_map>

namespace nm
{
#define MessageType(X) nm::message::MessageWrapper_Type_##X

    template <typename Key, typename... Params>
    class EventMap
    {
      private:
        std::unordered_map<Key, boost::signals2::signal<void(Params...)>> map;

      public:
        void connect(Key t, std::function<void(Params...)> fn)
        {
            map[t].connect(fn);
        }

        void fire(Key t, Params... data)
        {
            auto it = map.find(t);
            if (it != map.end())
            {
                it->second(data...);
            }
        }
    };
}  // namespace nm

#endif
