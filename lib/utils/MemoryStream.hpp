/*
 * MemoryStream.hpp
 */

#ifndef LIB_UTILS_MEMORYSTREAM_HPP_
#define LIB_UTILS_MEMORYSTREAM_HPP_

#include "Version.hpp"

#include <istream>
#include <streambuf>

namespace ARIASDK_NS_BEGIN
{
    class MemoryBuffer : public std::basic_streambuf<char>
    {
       public:
        MemoryBuffer(const uint8_t* data, size_t aLength) { setg((char*)data, (char*)data, (char*)data + aLength); }
    };

    class MemoryIStream : public std::istream
    {
        MemoryBuffer m_buffer;

       public:
        MemoryIStream(const uint8_t* aData, size_t aLength) : std::istream(&m_buffer), m_buffer(aData, aLength) { rdbuf(&m_buffer); };

        virtual ~MemoryIStream(){};
    };

}  // namespace ARIASDK_NS_BEGIN
ARIASDK_NS_END

#endif
