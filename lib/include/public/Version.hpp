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
#ifndef MAT_VERSION_HPP
#define MAT_VERSION_HPP
// WARNING: DO NOT MODIFY THIS FILE!
// This file has been automatically generated, manual changes will be lost.
#define BUILD_VERSION_STR "3.4.269.1"
#define BUILD_VERSION 3,4,269,1

#ifndef RESOURCE_COMPILER_INVOKED
#include <stdint.h>

#ifdef HAVE_MAT_SHORT_NS
#define MAT_NS_BEGIN  MAT
#define MAT_NS_END
#define PAL_NS_BEGIN  PAL
#define PAL_NS_END
#else
#define MAT_NS_BEGIN  Microsoft { namespace Applications { namespace Events
#define MAT_NS_END    }}
#define MAT           ::Microsoft::Applications::Events
#define PAL_NS_BEGIN  Microsoft { namespace Applications { namespace Events { namespace PlatformAbstraction
#define PAL_NS_END    }}}
#define PAL           ::Microsoft::Applications::Events::PlatformAbstraction
#endif

#define MAT_v1        ::Microsoft::Applications::Telemetry

namespace MAT_NS_BEGIN {

uint64_t const Version =
    ((uint64_t)3 << 48) |
    ((uint64_t)4 << 32) |
    ((uint64_t)269 << 16) |
    ((uint64_t)1);

} MAT_NS_END

namespace PAL_NS_BEGIN { } PAL_NS_END

#endif // RESOURCE_COMPILER_INVOKED
#endif

