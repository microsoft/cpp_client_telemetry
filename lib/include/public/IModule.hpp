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
#ifndef IMODULE_HPP
#define IMODULE_HPP

#include "Version.hpp"
#include "ctmacros.hpp"

///@cond INTERNAL_DOCS
namespace MAT_NS_BEGIN
{
    class ILogManager;

    /// <summary>
    /// IModule is a broad container interface that allows an application to override an internal component
    /// </summary>
    class MATSDK_LIBABI IModule
    {
    public:
        /// <summary>
        /// IModule destructor
        /// </summary>
        virtual ~IModule() noexcept = default;

        /// <summary>
        /// Initializes the module.
        /// Invoked as part of parent ILogManager is constructed.
        /// </summary>
        virtual void Initialize(ILogManager*) noexcept {};

        /// <summary>
        /// Tears down the module.
        /// Invoked as part of parent ILogManager's FlushAndTeardown() method.
        /// </summary>
        virtual void Teardown() noexcept {};
    };

    /// @endcond

} MAT_NS_END

#endif // IMODULE_HPP
