#pragma once

#include "Version.hpp"
#include <Enums.hpp>

#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <initializer_list>
#include <cstdint>
#include <sstream>
#include <iomanip>

namespace ARIASDK_NS_BEGIN
{
    class Variant;

    // Base type for Object (map)
    typedef std::map<std::string, Variant> VariantMap;

    // Base type for Array (vector)
    typedef std::vector<Variant> VariantArray;

// Generic variant type implementation added to SDK namespace
#include "VariantType.hpp"

    // Shortcut for VariantArray constructor
    // const auto ARR = Variant::from_array;
} ARIASDK_NS_END