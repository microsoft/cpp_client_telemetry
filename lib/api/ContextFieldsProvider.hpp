//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef CONTEXTFIELDSPROVIDER_HPP
#define CONTEXTFIELDSPROVIDER_HPP

#include "ILogger.hpp"
#include "CsProtocol_types.hpp"

#include "utils/Utils.hpp"

#include <mutex>
#include <set>
#include <string>
#include <cassert>

namespace MAT_NS_BEGIN
{

    class ContextFieldsProvider : public ISemanticContext
    {

    public:
        ContextFieldsProvider();
        ContextFieldsProvider(ContextFieldsProvider* parent);
        ContextFieldsProvider(ContextFieldsProvider const& copy);
        ContextFieldsProvider& operator=(ContextFieldsProvider const& copy);

        virtual void SetCommonField(const std::string&  name, const EventProperty&  value) override;
        void writeToRecord(::CsProtocol::Record& record, bool commonOnly = false);
        virtual void SetCustomField(const std::string&  name, const EventProperty&  value) override;

        virtual void SetParentContext(ContextFieldsProvider* parent);
        virtual void SetTicket(TicketType type, std::string const& ticketValue) override;

        virtual void SetEventExperimentIds(std::string const & eventName, std::string const & experimentIds) override;
        virtual void ClearExperimentIds() override;

        virtual std::map<std::string, EventProperty>& GetCommonFields();
        virtual std::map<std::string, EventProperty>& GetCustomFields();

    protected:

        std::mutex              m_lock;
        ContextFieldsProvider*  m_parent;

        std::map<std::string, EventProperty> m_commonContextFields;
        std::map<std::string, EventProperty> m_customContextFields;

        // mapping from an event name to a list of CSV'ed ECS configIds
        std::map<std::string, std::string>   m_commonContextEventToConfigIds;

        std::map<TicketType, std::string>    m_ticketsMap;
    };


} MAT_NS_END
#endif

