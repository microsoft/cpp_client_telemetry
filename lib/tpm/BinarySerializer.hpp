#pragma once

#include <string>
#include <map>

#include <aria/json.hpp>
using nlohmann::json;

namespace ARIASDK_NS_BEGIN {

	//typedef std::map< std::string, json> ObjectType; ///< Representation of JSON objects, i.e. unordered name-to-value map

            typedef enum VariantType {
                // Simple types
                VAR_TYPE_STRING  = 0, // 'someKey': 'someValue'
                VAR_TYPE_INTEGER = 1, // 'someKey': NUMBER
                VAR_TYPE_BOOLEAN = 2, // 'someKey': 'true'|'false'
                // Map from string to integer
                VAR_TYPE_ENUM    = 3, // 'someKey': '<value>' where value='lemons|oranges|apples' => [0,1,2]
                // Aggregate types
                VAR_TYPE_VEC_STR  = 4, // std::vector<std::string>
                VAR_TYPE_VEC_INT  = 5, // std::vector<int>
                VAR_TYPE_VEC_BOOL = 6  // std::vector<bool>
            } VariableType;

            typedef struct VariantField {
                VariantType type;
                size_t offset;
                std::map<std::string, std::string> extras;  // optional type-specific transform rules
            } FieldMapRule;

           //static bool jsonToStruct(json jsonObj, const std::map<std::string, json > &extractRules, void *out);
		} ARIASDK_NS_END