#ifndef JSONHELPER_HPP
#define JSONHELPER_HPP
#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "json.hpp"

#include <string>
#include <cstdlib>

using nlohmann::json;

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            class JsonHelper
            {
            public:
                static std::string Combine(const std::string& str1, const std::string& str2, char seperator);

                static std::vector<std::string> Split(const std::string& path, char seperator);

                static bool GetJson(const json& var, const std::string& path, json& output);

                static std::vector<std::string> GetKeys(const json& var, const std::string& path);

                static std::vector<std::string> GetValuesString(const json& var, const std::string& path);

                static std::vector<int> GetValuesInt(const json& var, const std::string& path);

                static std::vector<double> GetValuesDouble(const json& var, const std::string& path);

                static std::string GetValueString(const json& var, const std::string& path, const std::string& defaultValue);

                static int GetValueInt(const json& var, const std::string& path, const int& defaultValue);

                static long GetValueLong(const json& var, const std::string& path, const long& defaultValue);

                static bool GetValueBool(const json& var, const std::string& path, const bool& defaultValue);

                static double GetValueDouble(const json& var, const std::string& path, const double& defaultValue);

                static bool TryGetValueString(const json& var, const std::string& path, std::string& value);

                static bool TryGetValueInt(const json& var, const std::string& path, int& value);

                static bool TryGetValueLong(const json& var, const std::string& path, long& value);

                static bool TryGetValueBool(const json& var, const std::string& path, bool& value);

                static bool TryGetValueDouble(const json& var, const std::string& path, double& value);

            private:

                JsonHelper() {}
            };
        }
    }
}
#endif
#endif
