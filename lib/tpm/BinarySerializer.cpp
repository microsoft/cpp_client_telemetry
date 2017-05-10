#define LOG_MODULE DBG_SERIALIZER

#include "BinarySerializer.hpp"

using namespace std;

namespace ARIASDK_NS_BEGIN {
            /// <summary>
            /// Map JSON object to struct of any type using extraction (mapping) rules.
            /// </summary>
            /// <param name="jsonObj">JSON object to convert</param>
            /// <param name="extractRules">Field mapping rules</param>
            /// <param name="out">struct output</param>
            /// <returns>true if parsing succesful</returns>
            bool jsonToStruct(json jsonObj, const map<string, json> &extractRules, void *out)
			{
                bool success = true;
                for (auto &kv : jsonObj) {
                    auto &key = kv.first;
                    auto &val = kv.second;
                    auto it = extractRules.find(key);

                    // Check if field exists in extract rules
                    if (it != extractRules.end()) {
                        auto type = it->second->type;
                        auto offs = it->second->offset;
                        auto &extras = it->second->extras;

                        // Values extracted from JSON follow different extraction logic depending on their type
                        switch (type) {

                        case VAR_TYPE_STRING: {
                            json::StringType jS;
                            if (val.get(jS)) {
                                string *sVal = (string *)((char*)out + offs);
                                sVal->assign(jS.c_str());
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str() );
                            }
                            break;
                        }

                        case VAR_TYPE_INTEGER: {
                            json::IntType    jI;
                            if (val.get(jI)) {
                                int *iVal = (int*)((char*)out + offs);
                                (*iVal) = jI;
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str());
                            }
                            break;
                        }

                        case VAR_TYPE_BOOLEAN: {
                            json::BoolType   jB;
                            if (val.get(jB)) {
                                bool *bVal = (bool*)((char*)out + offs);
                                (*bVal) = jB;
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str());
                            }
                            break;
                        }

                        case VAR_TYPE_ENUM: {
                            json::StringType jE;
                            if (val.get(jE)) {
                                string key = jE;
                                auto it = extras.find(key);
                                if (it != extras.end()) {
                                    int ev = atoi(it->second.c_str());
                                    int *iVal = (int*)((char*)out + offs);
                                    (*iVal) = ev;
                                }
                                else {
                                    success = false;
                                    LOG_WARN("Attribute '%s' does not have an enum mapping for value '%s'", key.c_str(), val.serialize().c_str());
                                }
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str());
                            }
                            break;
                        }

                        case VAR_TYPE_VEC_STR: {
                            json::ArrayType  jArr;
                            if (val.get(jArr)) {
                                vector< string> *iVec = (vector< string> *)((char*)out + offs);
                                iVec->clear();
                                for (size_t j = 0; j < jArr.size(); j++) {
                                    json::StringType val;
                                    jArr[j].get(val);
                                    iVec->push_back(val);
                                }
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str());
                            }
                            break;
                        }

                        case VAR_TYPE_VEC_INT: {
                            json::ArrayType  jArr;
                            if (val.get(jArr)) {
                                vector<int> *iVec = (vector<int> *)((char*)out + offs);
                                iVec->clear();
                                for (size_t j = 0; j < jArr.size(); j++) {
                                    json::IntType val;
                                    jArr[j].get(val);
                                    iVec->push_back((int)val);
                                }
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str());
                            }
                            break;
                        }

                        case VAR_TYPE_VEC_BOOL: {
                            json::ArrayType  jArr;
                            if (val.get(jArr)) {
                                vector<bool> *iVec = (vector<bool> *)((char*)out + offs);
                                iVec->clear();
                                for (size_t j = 0; j < jArr.size(); j++) {
                                    json::BoolType val;
                                    jArr[j].get(val);
                                    iVec->push_back(val);
                                }
                            }
                            else {
                                success = false;
                                LOG_WARN("Attribute '%s' has invalid value '%s'", key.c_str(), val.serialize().c_str());
                            }
                            break;
                        }

                        default:
                            success = false;
                            LOG_WARN("Attribute %s has Unknown field type", key.c_str());
                        } /* end of switch */

                    } // else field is not found in extract rules
                    else {
                        // TODO: invalid record: skip the field, but print a warning
                        success = false;
                        LOG_WARN("Field %s is not found in schema definition!\n", key.c_str());
                    }
                }
                return success;
            }

} ARIASDK_NS_END