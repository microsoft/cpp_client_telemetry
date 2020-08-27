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
#ifndef IDECORATOR_HPP
#define IDECORATOR_HPP

#include <cstddef>

#include "Enums.hpp"
#include "CsProtocol_types.hpp"
#include "IModule.hpp"

namespace MAT_NS_BEGIN {

     /// <summary>
     /// Interface that allows to decorate outgoing Common Schema protocol Record.
     /// Part of advanced functionality SDK headers geared to provide the apps that
     /// rely on Common Schema protocol with ability to populate common properties.
     /// </summary>
    class IDecorator
    {

    public:
        /// <summary>
        /// Finalizes an instance of the <see cref="IDecorator"/> class.
        /// </summary>
        virtual ~IDecorator() = default;

        /// <summary>
        /// Decorates the specified record with common properties.
        /// </summary>
        /// <param name="record">Common Schema protocol record.</param>
        /// <returns></returns>
        virtual bool decorate(::CsProtocol::Record&) = 0;

    };

    //Forward declaring to resolve circular dependencies
    class ILogManager;

    /// <summary>
    /// Interface that allows to implement a custom decorator.
    /// </summary>
    class IDecoratorModule :
        public IDecorator,
        public IModule
    {

    protected:
        ILogManager* m_owner { nullptr };

    public:
        IDecoratorModule() = default;

        void Initialize(ILogManager *owner) noexcept override
        {
            m_owner = owner;
        }

        void Teardown() noexcept override
        {
            m_owner = nullptr;
        }
    };

} MAT_NS_END
#endif
