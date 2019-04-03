// Copyright (c) Microsoft. All rights reserved.

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
