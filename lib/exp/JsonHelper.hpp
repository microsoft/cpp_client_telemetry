#pragma once

#include "json.hpp"
#include <string>
#include <stdlib.h>

using nlohmann::json;
namespace Microsoft {
	namespace Applications {
		namespace Experimentation {
			class JsonHelper
			{
			public:
				static std::string JsonHelper::Combine(const std::string& str1, const std::string& str2, char seperator);
				static std::vector<std::string> JsonHelper::Split(const std::string& path, char seperator);
				static bool GetJson(const json& var, const std::string& path, json& output);

				static std::vector<std::string> GetKeys(const json& var, const std::string path);

				static std::vector<std::string> GetValuesString(const json& var, const std::string& path);

				static std::vector<int> GetValuesInt(const json& var, const std::string& path);

				static std::vector<double> GetValuesDouble(const json& var, const std::string& path);

				static std::string GetValueString(const json& var, const std::string& path, const std::string& defaultValue);

				static int GetValueInt(const json& var, const std::string& path, const int& defaultValue);

				static bool GetValueBool(const json& var, const std::string& path, const bool& defaultValue);

				static double GetValueDouble(const json& var, const std::string& path, const double& defaultValue);


			private:

				JsonHelper() {}
			};
		}
	}
}
