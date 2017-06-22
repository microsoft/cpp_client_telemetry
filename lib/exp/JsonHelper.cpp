
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
			std::vector<std::string> JsonHelper::GetKeys(const json& var, const std::string path)
			{
				std::vector<std::string> keyvec;
				json tmp;
				if (GetJson(var, path, tmp) && tmp.is_object())
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
				if (GetJson(var, path, tmp) && tmp.is_object())
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
				if (GetJson(var, path, tmp) && tmp.is_object())
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
				if (GetJson(var, path, tmp) && tmp.is_object())
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

			std::string JsonHelper::GetValueString(const json& var, const std::string& path, const std::string& defaultValue)
			{
				json tmp;
				if (GetJson(var, path, tmp) && tmp.is_object())
				{
					if (tmp.is_string())
					{
						return tmp.get<std::string>();
					}
				}
				return defaultValue;
			}

			int JsonHelper::GetValueInt(const json& var, const std::string& path, const int& defaultValue)
			{
				json tmp;
				if (GetJson(var, path, tmp) && tmp.is_object())
				{
					if (tmp.is_number_integer())
					{
						return tmp.get<int>();
					}
				}
				return defaultValue;
			}

			bool JsonHelper::GetValueBool(const json& var, const std::string& path, const bool& defaultValue)
			{
				json tmp;
				if (GetJson(var, path, tmp) && tmp.is_object())
				{
					if (tmp.is_boolean())
					{
						return tmp.get<bool>();
					}
				}
				return defaultValue;
			}

			double JsonHelper::GetValueDouble(const json& var, const std::string& path, const double& defaultValue)
			{
				json tmp;
				if (GetJson(var, path, tmp) && tmp.is_object())
				{
					if (tmp.is_number_float())
					{
						return tmp.get<float>();
					}
				}
				return defaultValue;
			}
		}
	}
}
