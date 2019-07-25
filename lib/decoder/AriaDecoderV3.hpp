#ifndef AriaDecoderV3_HPP
#define AriaDecoderV3_HPP

#include <cinttypes>
#include <vector>

namespace AriaDecoderV3 {

    void decode(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool compressed = true);

    void ExpandVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out);
    void InflateVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out);

}  // namespace AriaDecoderV3

#endif
