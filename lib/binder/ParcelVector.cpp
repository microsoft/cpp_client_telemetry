#ifdef ANDROID_NDK_BUILD
/*
 * Copyright (C) 2005 The Android Open Source Project
 * Copyright (C) Microsoft Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This is a huge hack that allows to link against platform libraries
// build with different C++ runtime. The gist of it is to remap the
// C++ types from STL used by NDK to Platform STL by remapping down
// to lowest available common denominator library with common stable ABI.

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <utils/Vector.h>

namespace android
{
    template <typename T>
    status_t readByteVectorInternal(const Parcel* parcel,
                                    std::vector<T>* val)
    {
        val->clear();

        int32_t size;
        status_t status = parcel->readInt32(&size);

        if (status != OK)
        {
            return status;
        }

        if (size < 0)
        {
            status = UNEXPECTED_NULL;
            return status;
        }
        if (size_t(size) > parcel->dataAvail())
        {
            status = BAD_VALUE;
            return status;
        }

        T* data = const_cast<T*>(reinterpret_cast<const T*>(parcel->readInplace(size)));
        if (!data)
        {
            status = BAD_VALUE;
            return status;
        }
        val->reserve(size);
        val->insert(val->end(), data, data + size);

        return status;
    };

    template <typename T>
    status_t writeByteVectorInternal(Parcel* parcel, const std::vector<T>& val)
    {
        status_t status;
        if (val.size() > std::numeric_limits<int32_t>::max())
        {
            status = BAD_VALUE;
            return status;
        }

        status = parcel->writeInt32(val.size());
        if (status != OK)
        {
            return status;
        }

        void* data = parcel->writeInplace(val.size());
        if (!data)
        {
            status = BAD_VALUE;
            return status;
        }

        memcpy(data, val.data(), val.size());
        return status;
    }

    status_t Parcel::readByteVector(std::vector<unsigned char, std::allocator<unsigned char>>* val) const
    {
        return readByteVectorInternal(this, val);
    };

    status_t Parcel::readString16Vector(
        std::unique_ptr<std::vector<std::unique_ptr<String16>>>* val) const
    {
        return readNullableTypedVector(val, &Parcel::readString16);
    }

    status_t Parcel::readString16Vector(std::vector<String16>* val) const
    {
        return readTypedVector(val, &Parcel::readString16);
    }

    status_t Parcel::writeByteVector(const std::vector<uint8_t>& val) {
        return writeByteVectorInternal(this, val);
    }

    status_t Parcel::writeString16Vector(const std::vector<String16>& val)
    {
        return writeTypedVector(val, &Parcel::writeString16);
    }

    status_t Parcel::writeString16Vector(
            const std::unique_ptr<std::vector<std::unique_ptr<String16>>>& val)
    {
        return writeNullableTypedVector(val, &Parcel::writeString16);
    }

};
#endif
