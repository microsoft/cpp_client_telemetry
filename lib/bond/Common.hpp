//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

namespace bond_lite {

template<typename TWriter, typename TValue>
void Serialize(TWriter& writer, TValue const& value)
{
    Serialize(writer, value, false);
}

template<typename TReader, typename TValue>
bool Deserialize(TReader& reader, TValue& value)
{
    return Deserialize(reader, value, false);
}

} // namespace bond_lite

