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
#ifndef COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
#define COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP

#include "Version.hpp"
#include "ctmacros.hpp"

#include <vector>

namespace MAT_NS_BEGIN { namespace Modules { namespace Filtering
{

    MATSDK_LIBABI void UpdateAllowedLevels(const std::vector<uint8_t>& allowedLevels) noexcept;

}}} MAT_NS_END

#endif // !COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
