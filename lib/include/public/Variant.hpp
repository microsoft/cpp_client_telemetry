///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MAT_VARIANT_HPP
#define MAT_VARIANT_HPP

#include "Version.hpp"
#include "Enums.hpp"

#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <initializer_list>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <cassert>

#ifdef VARIANT_CONCURRENCY
#ifdef _MANAGED
// Implementation of a LOCKGUARD for C++/CX .NET 4.x
#include <msclr/lock.h>
public ref class VariantLockGuard { public: static Object ^ lock = gcnew Object(); };
#define VARIANT_LOCKGUARD(lock_object)  msclr::lock l(VariantLockGuard::lock);
#define VARIANT_LOCK(lock_object)
#else
#ifdef _WIN32
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>
#endif
// Implementation of a LOCKGUARD for C++11
#include <mutex>
#define VARIANT_LOCKGUARD(lock_object)  std::lock_guard<decltype(lock_object)> TOKENPASTE2(__guard_, __LINE__) (lock_object)
#define VARIANT_LOCK(lock_object)       std::recursive_mutex  lock_object
#endif
#endif

namespace MAT_NS_BEGIN
{
    class Variant;

#ifdef VARIANT_CONCURRENCY
    // Windows provides concurrent maps and vectors as part of PPL concurrency package.
    // We are not currently using it, but we may consider to use it for safe concurrent
    // access to config tree from different threads at the same time.
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
} MAT_NS_END

#endif
