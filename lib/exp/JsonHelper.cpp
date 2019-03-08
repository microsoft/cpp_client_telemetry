#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "JsonHelper.hpp"

using nlohmann::json;

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            std::string JsonHelper::Combine(const std::string& str1, const std::string& str2, char seperator)
            {
                if (str1.empty())
                {
                    return str2;
                }
                else if (str2.empty())
                {
                    return str1;
                }
                else if ((*str1.rbegin() == seperator && *str2.begin() != seperator) ||
                    (*str1.rbegin() != seperator && *str2.begin() == seperator))
                {
                    return str1 + str2;
                }
                else
                {
                    return str1 + seperator + str2;
                }
            }
            std::vector<std::string> JsonHelper::Split(const std::string& path, char seperator)
            {
                std::vector<std::string> output;
                size_t len = path.size();
                std::string::size_type pos = 0;
                size_t i;
                for (i = 0; i < len; i++)
                {
                    pos = path.find(seperator, i);
                    if (pos < len)
                    {
                        std::string s = path.substr(i, pos - i);
                        output.push_back(s);
                        i = pos;
                    }
                    else
                    {
                        break;
                    }
                }

                if (i < len)
                {
                    output.push_back(path.substr(i));
                }

                return output;
            }
            /*static*/
            bool JsonHelper::GetJson(const json& var, const std::string& path, json& output)
            {
                const std::vector<std::string>& vpaths = Split(path, '/');
                json tmp = var;

                for (size_t i = 0; i < vpaths.size(); ++i)
                {
                    if (!tmp.is_object())
                    {
                        return false;
                    }
                    tmp = tmp[vpaths[i]];
                }

                output = tmp;
                return true;
            }

            /*static*/
            std::vector<std::string> JsonHelper::GetKeys(const json& var, const std::string& path)
            {
                std::vector<std::string> keyvec;
                json tmp;
                if (GetJson(var, path, tmp))
                {
                    for (auto it = tmp.begin(); it != tmp.end(); ++it)
                    {
                        keyvec.push_back(it.key());
                    }
                }
                return keyvec;
            }

            /*static*/
            std::vector<std::string> JsonHelper::GetValuesString(
                const json& var,
                const std::string& path)
            {
                json tmp;
                std::vector<std::string> keyvec;
                if (GetJson(var, path, tmp))
                {

                    for (auto it = tmp.begin(); it != tmp.end(); ++it)
                    {
                        if (it.value().is_string())
                        {
                            keyvec.push_back(it.value().get<std::string>());
                        }
                    }
                }
                return keyvec;
            }

            /*static*/
            std::vector<int> JsonHelper::GetValuesInt(
                const json& var,
                const std::string& path)
            {
                json tmp;
                std::vector<int> keyvec;
                if (GetJson(var, path, tmp))
                {

                    for (auto it = tmp.begin(); it != tmp.end(); ++it)
                    {
                        if (it.value().is_number_integer())
                        {
                            keyvec.push_back(it.value().get<int>());
                        }
                    }
                }

                return keyvec;
            }

            /*static*/
            std::vector<double> JsonHelper::GetValuesDouble(
                const json& var,
                const std::string& path)
            {
                json tmp;
                std::vector<double> keyvec;
                if (GetJson(var, path, tmp))
                {
                    for (auto it = tmp.begin(); it != tmp.end(); ++it)
                    {
                        if (it.value().is_number_float())
                        {
                            keyvec.push_back(it.value().get<double>());
                        }
                    }
                }
                return keyvec;
            }

            bool JsonHelper::TryGetValueString(const json& var, const std::string& path, std::string& value)
            {
                json tmp;
                if (GetJson(var, path, tmp))
                {
                    if (tmp.is_string())
                    {
                        value = tmp.get<std::string>();
                        return true;
                    }
                }
                return false;
            }

            std::string JsonHelper::GetValueString(const json& var, const std::string& path, const std::string& defaultValue)
            {
                std::string value;
                if (TryGetValueString(var, path, value))
                {
                    return value;
                }
                return defaultValue;
            }

            bool JsonHelper::TryGetValueInt(const json& var, const std::string& path, int& value)
            {
                json tmp;
                if (GetJson(var, path, tmp))
                {
                    if (tmp.is_number_integer())
                    {
                        value = tmp.get<int>();
                        return true;
                    }
                }
                return false;
            }

            int JsonHelper::GetValueInt(const json& var, const std::string& path, const int& defaultValue)
            {
                int value;
                if (TryGetValueInt(var, path, value))
                {
                    return value;
                }
                return defaultValue;
            }

            bool JsonHelper::TryGetValueLong(const json& var, const std::string& path, long& value)
            {
                json tmp;
                if (GetJson(var, path, tmp))
                {
                    if (tmp.is_number_integer())
                    {
                        value = tmp.get<long>();
                        return true;
                    }
                }
                return false;
            }

            long JsonHelper::GetValueLong(const json& var, const std::string& path, const long& defaultValue)
            {
                long value;
                if (TryGetValueLong(var, path, value))
                {
                    return value;
                }
                return defaultValue;
            }

            bool JsonHelper::TryGetValueBool(const json& var, const std::string& path, bool& value)
            {
                json tmp;
                if (GetJson(var, path, tmp))
                {
                    if (tmp.is_boolean())
                    {
                        value = tmp.get<bool>();
                        return true;
                    }
                }
                return false;
            }

            bool JsonHelper::GetValueBool(const json& var, const std::string& path, const bool& defaultValue)
            {
                bool value;
                if (TryGetValueBool(var, path, value))
                {
                    return value;
                }
                return defaultValue;
            }

            bool JsonHelper::TryGetValueDouble(const json& var, const std::string& path, double& value)
            {
                json tmp;
                if (GetJson(var, path, tmp))
                {
                    if (tmp.is_number_float())
                    {
                        value = tmp.get<float>();
                        return true;
                    }
                }
                return false;
            }

            double JsonHelper::GetValueDouble(const json& var, const std::string& path, const double& defaultValue)
            {
                double value;
                if (TryGetValueDouble(var, path, value))
                {
                    return value;
                }
                return defaultValue;
            }
        }
    }
}
#endif
