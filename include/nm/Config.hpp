#ifndef _NM_CONFIG_
#define _NM_CONFIG_

#include <string>
#include <json.hpp>

namespace nm
{
	class Config
	{
		private:
			nlohmann::json config;
			std::string filename;
			
			using T = nlohmann::json;
		public:
			Config();
			~Config();

			T& operator[](std::string key)
			{
				return config[key];
			}
			
			bool save() const;
			bool save(const std::string& filename) const;
			bool load(const std::string& filename, bool merge = true);
			void merge(const nlohmann::json& other);
	};

	extern Config config;
}

#endif
