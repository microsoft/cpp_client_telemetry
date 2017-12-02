
#ifndef ARIA_EVENTPROPERTY_HPP
#define ARIA_EVENTPROPERTY_HPP

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

                /// Default constructor for an empty object
                /// </summary>
                time_ticks_t();

                /// <summary>
                /// Convert number of .NET ticks to time_ticks_t structure
                /// </summary>
                time_ticks_t(uint64_t raw);

                /// <summary>
                /// time_t time must contain timestamp in UTC time
                /// </summary>
                time_ticks_t(const std::time_t* time);

                /// <summary>
                /// time_ticks_t copy constructor
                /// </summary>
                time_ticks_t(const time_ticks_t& t);
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
                /// <summary>Specifies the first 8 hexadecimal digits of the GUID.</summary>
                uint32_t Data1;

                /// <summary>Specifies the first group of 4 hexadecimal digits.</summary>
                uint16_t Data2;

                /// <summary>Specifies the second group of 4 hexadecimal digits.</summary>
                uint16_t Data3;

                /// <summary>Array of 8 bytes. The first 2 bytes contain the third group of 4 hexadecimal digits.
                /// The remaining 6 bytes contain the final 12 hexadecimal digits.</summary>
                uint8_t  Data4[8];

                /// <summary>
                /// Create an empty Nil instance of GUID_t object (initialized to all 0)
                /// {00000000-0000-0000-0000-000000000000}
                /// </summary>
                GUID_t();

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

                GUID_t(int d1, int d2, int d3, const std::initializer_list<uint8_t> &v);
                
                /// <summary>
                /// GUID_t copy constructor
                /// </summary>
                GUID_t(const GUID_t& guid);

#ifdef _WIN32
                /// <summary>
                /// Create GUID_t object from Windows GUID object
                /// </summary>
                GUID_t(GUID guid);

                static GUID convertUintVectorToGUID(std::vector<uint8_t> const& bytes);
             
#endif

                void to_bytes(uint8_t(&guid_bytes)[16]) const;

                std::string to_string() const;

                // The output from this method is compatible with std::unordered_map.
                std::size_t HashForMap() const;

                // Are 2 GUID_t objects equivalent? (needed for maps)
                bool operator==(GUID_t const& other) const;
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

                /// <summary>Event field Data Category</summary>
                DataCategory dataCategory = DataCategory_PartC;

                /// <summary>
                /// Variant object value
                /// </summary>
                union
                {
                    char*  as_string;
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
                static const char *type_name(unsigned typeId);
            
                /// <summary>
                /// EventProperty copy constructor
                /// </summary>
                /// <param name="source">Right-hand side value of object</param>
                EventProperty(const EventProperty& source);

                /// <summary>
                /// EventProperty move constructor
                /// </summary>
                /// <param name="source">Right-hand side value of object</param>
                EventProperty(EventProperty&& source);

                /// <summary>
                /// EventProperty equalto operator
                /// </summary>
                bool operator==(const EventProperty& source) const;

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(const EventProperty& source);

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(const std::string& value);

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(const char *value);

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(int64_t value);

                // All other integer types get converted to int64_t
#ifdef _WIN32
                EventProperty& operator=(long    value);
#endif
                EventProperty& operator=(int8_t  value); 
                EventProperty& operator=(int16_t value); 
                EventProperty& operator=(int32_t value); 
                EventProperty& operator=(uint8_t  value); 
                EventProperty& operator=(uint16_t value); 
                EventProperty& operator=(uint32_t value); 
                EventProperty& operator=(uint64_t value); 

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(double value);

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(bool value);

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(time_ticks_t value);

                /// <summary>
                /// EventProperty assignment operator
                /// </summary>
                EventProperty& operator=(GUID_t value);

                /// <summary>
                /// Clears value object, deallocating memory if needed
                /// </summary>
                void clear();

                /// EventProperty destructor
                /// </summary>
                virtual ~EventProperty();

                /// <summary>
                /// EventProperty default constructor for empty string value
                /// </summary>
                EventProperty();

                /// <summary>
                /// EventProperty constructor for string value
                /// </summary>
                /// <param name="value">string value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(const char* value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                /// <summary>
                /// EventProperty constructor for string value
                /// </summary>
                /// <param name="value">string value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(const std::string& value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                /// <summary>
                /// EventProperty constructor for int64 value
                /// </summary>
                /// <param name="value">int64_t value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(int64_t       value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                /// <summary>
                /// EventProperty constructor for double value
                /// </summary>
                /// <param name="value">double value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(double        value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                /// <summary>
                /// EventProperty constructor for time in .NET ticks
                /// </summary>
                /// <param name="value">time_ticks_t value - time in .NET ticks</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(time_ticks_t  value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                /// <summary>
                /// EventProperty constructor for boolean value
                /// </summary>
                /// <param name="value">boolean value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(bool          value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                /// <summary>
                /// EventProperty constructor for GUID
                /// </summary>
                /// <param name="value">GUID_t value</param>
                /// <param name="piiKind">Pii kind</param>
                EventProperty(GUID_t        value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);

                // All other integer types get converted to int64_t
#ifdef _WIN32
                EventProperty(long     value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 
#endif
                EventProperty(int8_t   value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 
                EventProperty(int16_t  value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 
                EventProperty(int32_t  value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 
                EventProperty(uint8_t  value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 
                EventProperty(uint16_t value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC);
                EventProperty(uint32_t value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 
                EventProperty(uint64_t value, PiiKind piiKind = PiiKind_None, DataCategory category = DataCategory_PartC); 

                /// <summary>Returns true whether the type is string AND the value is empty (i.e. whether its length is 0).</summary>
                bool empty();

                /// <summary>Return a string representation of this value object</summary>
                virtual std::string to_string() const;

            };

        }
    }
}

#endif //ARIA_EVENTPROPERTY_HPP
