// Copyright (c) Microsoft. All rights reserved.
#ifndef IDECORATOR_HPP
#define IDECORATOR_HPP

#include <cstddef>

#include "Enums.hpp"
#include "CsProtocol_types.hpp"
#include "ILogManager.hpp"

namespace ARIASDK_NS_BEGIN {

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

} ARIASDK_NS_END
#endif
