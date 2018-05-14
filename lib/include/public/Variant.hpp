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
#include <cassert>

#if 0
// TODO: this code would allow to provide per-property locking. Currently this code is not enabled.
#ifdef _MANAGED
// Implementation of a LOCKGUARD for C++/CX .NET 4.x
#include <msclr/lock.h>
public ref class VariantLockGuard { public: static Object ^ lock = gcnew Object(); };
#define VARIANT_LOCKGUARD(lock_object)  msclr::lock l(VariantLockGuard::lock);
#define VARIANT_LOCK(lock_object)
#else
// Implementation of a LOCKGUARD for C++11
#include <mutex>
#define VARIANT_LOCKGUARD(lock_object)  std::lock_guard<decltype(lock_object)> TOKENPASTE2(__guard_, __LINE__) (lock_object)
#define VARIANT_LOCK(lock_object)       std::recursive_mutex  lock_object
#endif
#endif

#ifdef _WIN32
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>
#endif

namespace ARIASDK_NS_BEGIN
{
    class Variant;

#if 0
    //
    // TODO: Windows provides concurrent maps and vectors as part of PPL concurrency package.
    // We are not currently using it, but we may consider to use it for safe concurrent access
    // to config tree from different threads at the same time. This solution is not being used
    // because it is not portable.
    //
    // Base type for Object (map)
    typedef concurrency::concurrent_unordered_map<std::string, Variant>     VariantMap;
    // Base type for Array (vector)
    typedef concurrency::concurrent_vector<Variant>                         VariantArray;
#else
    // Base type for Object (map)
    typedef std::map<std::string, Variant>                                  VariantMap;
    // Base type for Array (vector)
    typedef std::vector<Variant>                                            VariantArray;
#endif

// Generic variant type implementation added to SDK namespace
#include "VariantType.hpp"

    // Shortcut for VariantArray constructor
    // const auto ARR = Variant::from_array;
} ARIASDK_NS_END