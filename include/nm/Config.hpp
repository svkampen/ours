#ifndef _NM_CONFIG_
#define _NM_CONFIG_

#include <json.hpp>
#include <string>

namespace nm
{
    class Config
    {
      private:
        nlohmann::json config;
        std::string filename;

      public:
        Config();
        ~Config();

        nlohmann::json& operator[](const std::string& key);

        bool contains(std::string key) const;
        bool save() const;
        bool save(const std::string& filename) const;
        bool load(const std::string& filename, bool merge = true);
        void merge(const nlohmann::json& other);
    };

    extern Config config;
}  // namespace nm

#endif
