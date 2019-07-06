#include <nm/Config.hpp>
#include <fstream>

namespace nm
{
	Config config;
	
	Config::Config()
	{
	};
	
	Config::~Config()
	{
		if (!this->filename.empty())
		{
			/* this->save(); for now, don't. */
		}
	}

	bool Config::contains(std::string key) const
	{
		return this->config.find(key) != this->config.end();
	}

	bool Config::save() const
	{
		return this->save(this->filename);
	}
	
	bool Config::save(const std::string& filename) const
	{
		std::ofstream file;
		file.open(filename, file.binary);
		
		if (file.is_open())
			file << std::setw(4) << this->config << std::endl;
		else
			return false;
		
		file.close();
		return true;
	}
	
	bool Config::load(const std::string& filename, bool merge)
	{
		this->filename = filename;
		
		std::ifstream file;
		file.open(filename, file.binary);
		
		if (file.is_open())
		{
			if (merge)
				this->merge(nlohmann::json(file));
			else
				file >> this->config;
		} else
		{
			return false;
		}
		
		file.close();
		return true;
	}
	
	void Config::merge(const nlohmann::json& other)
	{
		nlohmann::json result = this->config.flatten();
		nlohmann::json tmp = other.flatten();

		for (auto it = tmp.begin(); it != tmp.end(); ++it)
		{
            result[it.key()] = it.value();
		}

		this->config = result.unflatten();
	}
}
