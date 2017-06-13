#pragma once
#include "ctmacros.hpp"
#include "Enums.hpp"
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <cstdlib>
#include <cstdint>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#ifndef strdup
#define strdup _strdup
#endif
/* Required for GUID type helper function on Windows */
#include <ObjBase.h>
#endif

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

            const uint64_t ticksPerSecond = 10000000UL;
            // Thursday, January 01, 1970 12:00:00 AM
            const uint64_t ticksUnixEpoch = 0x089f7ff5f7b58000;

            /// <summary>Time in .NET ticks</summary>
            /// <remarks>
            /// A single tick represents one hundred nanoseconds or one ten-millionth of a second.
            /// There are 10,000 ticks in a millisecond, or 10 million ticks in a second.
            /// The value of this property represents the number of 100 - nanosecond intervals that have
            /// elapsed since 12:00 : 00 midnight, January 1, 0001 (0:00 : 00 UTC on January 1, 0001, in
            /// the Gregorian calendar), which represents DateTime.MinValue.
            /// It does not include the number  of ticks that are attributable to leap seconds.
            /// </remarks>
            struct ARIASDK_LIBABI time_ticks_t {
                /// <summary>Raw 64-bit signed integer number representing number of .NET ticks</summary>
                uint64_t ticks;

                /// <summary>
                /// Default constructor for an empty object
                /// </summary>
                time_ticks_t() : ticks(0) {};

                /// <summary>
                /// Convert number of .NET ticks to time_ticks_t structure
                /// </summary>
                time_ticks_t(uint64_t raw) : ticks(raw) {};

                /// <summary>
                /// time_t time must contain timestamp in UTC time
                /// </summary>
                time_ticks_t(const std::time_t* time)
                {
                    ticks = ticksUnixEpoch + ticksPerSecond * ((uint64_t)(*time));
                }

                /// <summary>
                /// time_ticks_t copy constructor
                /// </summary>
                time_ticks_t(const time_ticks_t& t) {
                    this->ticks = t.ticks;
                }
            };

            /// <summary>GUID type - ARIA portable cross-platform implementation of GUID.
            /// GUIDs identify objects such as interfaces, manager entry-point vectors (EPVs), and class objects.
            /// A GUID is a 128-bit value consisting of one group of 8 hexadecimal digits, followed
            /// by three groups of 4 hexadecimal digits each, followed by one group of 12 hexadecimal digits.
            /// 
            /// Definition of this structure is a cross-platform equivalent of Windows RPC GUID definition.
            /// Ref: https://msdn.microsoft.com/en-us/library/windows/desktop/aa373931%28v=vs.85%29.aspx
            /// 
            /// Customers need to provide their own converter from RPC GUID type to ARIA portable GUID type.
            /// </remarks>
            struct ARIASDK_LIBABI GUID_t {
            private:
                /// <summary>Specifies the first 8 hexadecimal digits of the GUID.</summary>
                uint32_t Data1;

                /// <summary>Specifies the first group of 4 hexadecimal digits.</summary>
                uint16_t Data2;

                /// <summary>Specifies the second group of 4 hexadecimal digits.</summary>
                uint16_t Data3;

                /// <summary>Array of 8 bytes. The first 2 bytes contain the third group of 4 hexadecimal digits.
                /// The remaining 6 bytes contain the final 12 hexadecimal digits.</summary>
                uint8_t  Data4[8];

            public:
                /// <summary>
                /// Create an empty Nil instance of GUID_t object (initialized to all 0)
                /// {00000000-0000-0000-0000-000000000000}
                /// </summary>
                GUID_t() : Data1(0), Data2(0), Data3(0)
                {
                    // This silly code is required because vs2013 does not support
                    // initializing arrays within a constructor list (C++11 feature).
                    for(size_t i=0; i<8; i++)
                    {
                        Data4[i] = 0;
                    }
                };

                /// <summary>
                /// Create GUID_t object from hyphenated string (curly braces optional)
                /// </summary>
                GUID_t(const char* guid_string);

                /// <summary>
                /// Create GUID_t object from byte array
                /// </summary>
                /// <param name="bigEndian">
                /// false (default) - use the same order as .NET Guid constructor
                /// true            - use 'more natural' human-readable order
                /// </param>
                GUID_t(const uint8_t guid_bytes[16], bool bigEndian = false);

                GUID_t(int d1, int d2, int d3, const std::initializer_list<uint8_t> &v):
                    Data1((uint32_t)d1), Data2((uint16_t)d2), Data3((uint16_t)d3)
                {
                    size_t i = 0;
                    for (auto val : v) {
                        Data4[i] = val;
                        i++;
                    }
                }

                /// <summary>
                /// GUID_t copy constructor
                /// </summary>
                GUID_t(const GUID_t& guid) {
                    this->Data1 = guid.Data1;
                    this->Data2 = guid.Data2;
                    this->Data3 = guid.Data3;
                    memcpy(&(this->Data4[0]), &(guid.Data4[0]), sizeof(guid.Data4));
                }

#ifdef _WIN32
                /// <summary>
                /// Create GUID_t object from Windows GUID object
                /// </summary>
                GUID_t(GUID guid) {
                    this->Data1 = guid.Data1;
                    this->Data2 = guid.Data2;
                    this->Data3 = guid.Data3;
                    memcpy(&(this->Data4[0]), &(guid.Data4[0]), sizeof(guid.Data4));
                }

				static GUID convertUintVectorToGUID(std::vector<uint8_t> const& bytes)
				{
					GUID_t temp_t = GUID_t(bytes.data());
					GUID temp;
					temp.Data1 = temp_t.Data1;
					temp.Data2 = temp_t.Data2;
					temp.Data3 = temp_t.Data3;
					for (size_t i = 0; i < 8; i++)
					{
						temp.Data4[i] = temp_t.Data4[i];
					}
					return temp;
				}
#endif

                void to_bytes(uint8_t(&guid_bytes)[16]) const;

#pragma warning(push)
#pragma warning(disable:6031)
                std::string to_string() const
                {
                    const unsigned buffSize = 36 + 1;  // 36 + null-termination
                    char buff[buffSize];
                    snprintf(buff, buffSize,
                        "%08X-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                        this->Data1, this->Data2, this->Data3,
                        this->Data4[0], this->Data4[1], this->Data4[2], this->Data4[3],
                        this->Data4[4], this->Data4[5], this->Data4[6], this->Data4[7]);
                    std::string result(buff);
                    return result;
                }
#pragma warning(pop)

                // The output from this method is compatible with std::unordered_map.
                std::size_t HashForMap() const
                {
                    // Compute individual hash values for Data1, Data2, Data3, and parts of Data4
                    // http://stackoverflow.com/a/1646913/126995
                    size_t res = 17;
                    res = res * 31 + Data1;
                    res = res * 31 + Data2;
                    res = res * 31 + Data3;
                    res = res * 31 + (Data4[0] << 24 | Data4[1] << 16 | Data4[6] << 8 | Data4[7]);
                    return res;
                }

                // Are 2 GUID_t objects equivalent? (needed for maps)
                bool operator==(GUID_t const& other) const
                {
                    return Data1 == other.Data1 &&
                        Data2 == other.Data2 &&
                        Data3 == other.Data3 &&
                        (0 == memcmp(Data4, other.Data4, sizeof(Data4)));
                }
            };

            /// <summary>
            /// Declare this struct as the hasher when using GUID_t as a key in an unordered_map
            /// </summary>
            struct GuidMapHasher
            {
                inline std::size_t operator()(GUID_t const& key) const
                {
                    return key.HashForMap();
                }
            };

            /// <summary>
            /// C++11 variant object that holds:
            /// - type
            /// - value
            /// - <see cref="PiiKind">PiiKind</see>
            /// </summary>
            struct ARIASDK_LIBABI EventProperty
            {
                // <remarks>
                // With the concept of EventProperty value object we allow users implementing their
                // own type conversion system, which may subclass and provides an implementation of
                // to_string method
                // </remarks>
            public:

                /// <summary>
                /// Types supported by Aria collector:
                ///  - String
                ///  - Int64
                ///  - Double
                ///  - DateTime (in .NET Ticks)
                ///  - Boolean
                ///  - GUID (16-byte representation of GUID)
                /// </summary>
                enum
                {
                    /// <summary>String</summary>
                    TYPE_STRING,
                    /// <summary>64-bit signed integer</summary>
                    TYPE_INT64,
                    /// <summary>double</summary>
                    TYPE_DOUBLE,
                    /// <summary>Date/time represented in .NET ticks</summary>
                    TYPE_TIME,
                    /// <summary>boolean</summary>
                    TYPE_BOOLEAN,
                    /// <summary>GUID</summary>
                    TYPE_GUID,
                } type;

                /// <summary>Event field Pii kind</summary>
                PiiKind piiKind;

                /// <summary>
                /// Variant object value
                /// </summary>
                union
                {
                    const char*  as_string;
                    int64_t      as_int64;
                    double       as_double;
                    bool         as_bool;
#if defined(_MSC_VER) && (_MSC_VER < 1900)
                }; // end of anonymous union for vs2013
#endif
                /* *** C++11 compatibility quirk of vs2013 ***
                 * Unfortunately vs2013 is not fully C++11 compliant: it does not support complex
                 * variant objects within anonymous union. So the structure has to occcupy more
                 * RAM by placing these two members outside of the union.
                 * The rest of code logic remains the same. */
                    GUID_t       as_guid;
                    time_ticks_t as_time_ticks;
#if !(defined(_MSC_VER) && (_MSC_VER < 1900))
                }; // end of anonymous union for other C++11 compilers (sigh)
#endif

                /// <summary>Debug routine that returns string representation of type name</summary>
                static const char *type_name(unsigned typeId) {
                    switch (typeId) {
                    case TYPE_STRING:
                        return "string";
                    case TYPE_INT64:
                        return "int64";
                    case TYPE_DOUBLE:
                        return "double";
                    case TYPE_TIME:
                        return "time";
                    case TYPE_BOOLEAN:
                        return "bool";
                    case TYPE_GUID:
                        return "guid";
                    default:
                        return "unknown";
                    }
                }

                /// <summary>
                /// EventProperty copy constructor
                /// </summary>
                /// <param name="source">Right-hand side value of object</param>
                EventProperty(const EventProperty& source) :
                    type(source.type)
                {
                    memcpy((void*)this, (void*)&source, sizeof(EventProperty));
                    if (type == TYPE_STRING)
                    {
                        as_string = strdup(source.as_string);
                    }
                }

                /// <summary>
                /// EventProperty move constructor
                /// </summary>
                /// <param name="source">Right-hand side value of object</param>
                EventProperty(EventProperty&& source) /* noexcept */ :
                    type(source.type)
                {
                    memcpy((void*)this, (void*)&source, sizeof(EventProperty));
                    if (type == TYPE_STRING)
                    {
                        as_string = strdup(source.as_string);
                    }
                }


                /// <summary>
                /// EventProperty equalto operator
                /// </summary>
                bool operator==(const EventProperty& source) const
                {
                    if (type == source.type)
                    {
                        switch (type)
                        {
                        case TYPE_STRING:
                        {
                            std::string temp1 = as_string;
                            std::string temp2 = source.as_string;
                            if (temp1.compare(temp2) == 0)
                            {
                                return true;
                            }
                            break;
                        }
                        case TYPE_INT64:
                            if (as_int64 == source.as_int64)
                            {
                                return true;
                            }
                            break;
                        case TYPE_DOUBLE:
                            if (as_double == source.as_double)
                            {
                                return true;
                            }
                            break;
                        case TYPE_TIME:
                            if (as_time_ticks.ticks == source.as_time_ticks.ticks)
                            {
                                return true;
                            }
                            break;
                        case TYPE_BOOLEAN:
                            if (as_bool == source.as_bool)
                            {
                                return true;
                            }
                            break;
                        case TYPE_GUID:
                        {
                            std::string temp1 = as_guid.to_string();
                            std::string temp2 = source.as_guid.to_string();
                            if (temp1.compare(temp2) == 0)
                            {
                                return true;
                            }
                            break;
                        }
                            
                        }
                    }                    
                    return false;
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(const EventProperty& source)
                {
                    clear();
                    memcpy((void*)this, (void*)&source, sizeof(EventProperty));
                    if (type == TYPE_STRING)
                    {
                        as_string = strdup(source.as_string); // Error #28: LEAK 4 bytes 
                    }
                    return (*this);
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(const std::string& value)
                {
                    clear();
                    as_string = strdup(value.c_str());
                    type = TYPE_STRING;
                    return (*this);
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(const char *value)
                {
                    clear();
                    as_string = strdup(value);
                    type = TYPE_STRING;
                    return (*this);
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(int64_t value) {
                    clear();
                    this->type = TYPE_INT64;
                    this->as_int64 = value;
                    return (*this);
                }

                // All other integer types get converted to int64_t
                EventProperty& operator=(long    value)  { return ((*this) = (int64_t)value); }
                EventProperty& operator=(int8_t  value)  { return ((*this) = (int64_t)value); }
                EventProperty& operator=(int16_t value)  { return ((*this) = (int64_t)value); }
                EventProperty& operator=(int32_t value)  { return ((*this) = (int64_t)value); }
                EventProperty& operator=(uint8_t  value) { return ((*this) = (int64_t)value); }
                EventProperty& operator=(uint16_t value) { return ((*this) = (int64_t)value); }
                EventProperty& operator=(uint32_t value) { return ((*this) = (int64_t)value); }
                EventProperty& operator=(uint64_t value) { return ((*this) = (int64_t)value); }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(double value) {
                    clear();
                    this->type = TYPE_DOUBLE;
                    this->as_double = value;
                    return (*this);
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(bool value) {
                    clear();
                    this->type = TYPE_BOOLEAN;
                    this->as_bool = value;
                    return (*this);
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(time_ticks_t value) {
                    clear();
                    this->type = TYPE_TIME;
                    this->as_time_ticks = value;
                    return (*this);
                }

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(GUID_t value) {
                    clear();
                    this->type = TYPE_GUID;
                    this->as_guid = value;
                    return (*this);
                }

                /// <summary>
                /// Clears value object, deallocating memory if needed
                /// </summary>
                void clear() {
                    if (type == TYPE_STRING) {
                        if (as_string != NULL) {
                            free((void *)as_string);
                            as_string = NULL;
                        }
                    }
                }

                /// EventProperty destructor
                /// </summary>
                virtual ~EventProperty()
                {
                    clear();
                }

                /// <summary>
                /// EventProperty default constructor for empty string value
                /// </summary>
                EventProperty() :
                    type(TYPE_STRING),
                    piiKind(PiiKind_None)
                {
                    as_time_ticks.ticks = 0;
                    as_guid = {};
                    as_string = strdup("");
                };

                /// <summary>
                /// EventProperty constructor for string value
                /// </summary>
                /// <param name="value">string value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(const char* value, PiiKind piiKind = PiiKind_None) :
                    type(TYPE_STRING),
                    piiKind(piiKind) {
                    as_string = strdup((value != NULL) ? value : "");
                };

                /// <summary>
                /// EventProperty constructor for string value
                /// </summary>
                /// <param name="value">string value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(const std::string& value, PiiKind piiKind = PiiKind_None) :
                    type(TYPE_STRING),
                    piiKind(piiKind) {
                    as_string = strdup(value.c_str());
                };

                /// <summary>
                /// EventProperty constructor for int64 value
                /// </summary>
                /// <param name="value">int64_t value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(int64_t       value, PiiKind piiKind = PiiKind_None) : type(TYPE_INT64), piiKind(piiKind), as_int64(value) {};

                /// <summary>
                /// EventProperty constructor for double value
                /// </summary>
                /// <param name="value">double value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(double        value, PiiKind piiKind = PiiKind_None) : type(TYPE_DOUBLE), piiKind(piiKind), as_double(value) {};

                /// <summary>
                /// EventProperty constructor for time in .NET ticks
                /// </summary>
                /// <param name="value">time_ticks_t value - time in .NET ticks</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(time_ticks_t  value, PiiKind piiKind = PiiKind_None) : type(TYPE_TIME), piiKind(piiKind), as_time_ticks(value) {};

                /// <summary>
                /// EventProperty constructor for boolean value
                /// </summary>
                /// <param name="value">boolean value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(bool          value, PiiKind piiKind = PiiKind_None) : type(TYPE_BOOLEAN), piiKind(piiKind), as_bool(value) {};

                /// <summary>
                /// EventProperty constructor for GUID
                /// </summary>
                /// <param name="value">GUID_t value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(GUID_t        value, PiiKind piiKind = PiiKind_None) : type(TYPE_GUID), piiKind(piiKind), as_guid(value) {};

                // All other integer types get converted to int64_t
                EventProperty(long     value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(int8_t   value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(int16_t  value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(int32_t  value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(uint8_t  value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(uint16_t value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(uint32_t value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};
                EventProperty(uint64_t value, PiiKind piiKind = PiiKind_None) : EventProperty((int64_t)value, piiKind) {};

                /// <summary>Returns true whether the type is string AND the value is empty (i.e. whether its length is 0).</summary>
                bool empty()
                {
                    return ((type == TYPE_STRING) && (strlen(as_string) == 0));
                }

                /// <summary>Return a string representation of this value object</summary>
                virtual std::string to_string() const {
                    std::string result;
                    switch (type) {
                    case TYPE_STRING:
                        result = as_string;
                        break;
                    case TYPE_INT64:
                        result = std::to_string(as_int64);
                        break;
                    case TYPE_DOUBLE:
                        result = std::to_string(as_double);
                        break;
                    case TYPE_TIME:
                        // Note that we do not format time as time, we return it as raw number of .NET ticks
                        result = std::to_string(as_time_ticks.ticks);
                        break;
                    case TYPE_BOOLEAN:
                        result = ((as_bool) ? "true" : "false");
                        break;
                    case TYPE_GUID:
                        result = as_guid.to_string();
                        break;
                    default:
                        result = "";
                        break;
                    }
                    return result;
                }

            };

        }
    }
}

#ifdef _WIN32
#pragma warning(pop)
#endif
