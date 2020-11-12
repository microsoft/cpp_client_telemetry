//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ITelemetryAgent.h"
#include "BpTelemetryAgent.h"

namespace com
{
    namespace microsoft
    {
        namespace telemetry
        {
            IMPLEMENT_META_INTERFACE(TelemetryAgent, "com.microsoft.telemetry.ITelemetryAgent")

        }  // namespace telemetry

    }  // namespace microsoft

}  // namespace com

#include <android-base/macros.h>
#include <binder/Parcel.h>

namespace com
{
    namespace microsoft
    {
        namespace telemetry
        {
            BpTelemetryAgent::BpTelemetryAgent(const ::android::sp<::android::IBinder>& _aidl_impl) :
                BpInterface<ITelemetryAgent>(_aidl_impl)
            {
            }

            ::android::binder::Status BpTelemetryAgent::open(const ::android::String16& config, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeString16(config);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 0 /* open */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->open(config, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::close(int64_t id, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 1 /* close */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->close(id, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::clear(int64_t id, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 2 /* clear */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->clear(id, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::config(int64_t id, const ::android::String16& config, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeString16(config);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 3 /* config */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->config(id, config, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::writeEvent(int64_t id, int64_t contentType, const ::std::vector<uint8_t>& data, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(contentType);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeByteVector(data);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 4 /* writeEvent */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->writeEvent(id, contentType, data, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::writeBlob(int64_t id, ::android::base::unique_fd pfd, const ::android::String16& name, const ::android::String16& contentType, int64_t byteCount, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeUniqueFileDescriptor(pfd);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeString16(name);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeString16(contentType);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(byteCount);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 5 /* writeBlob */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->writeBlob(id, std::move(pfd), name, contentType, byteCount, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::writeMetadata(int64_t id, const ::std::vector<::android::String16>& metadata, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeString16Vector(metadata);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 6 /* writeMetadata */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->writeMetadata(id, metadata, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::pause(int64_t id, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 7 /* pause */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->pause(id, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::resume(int64_t id, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 8 /* resume */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->resume(id, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::upload(int64_t id, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(id);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 9 /* upload */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->upload(id, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::stop(int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 10 /* stop */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->stop(_aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::opmode(int64_t mode, int64_t* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_data.writeInt64(mode);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 11 /* opmode */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->opmode(mode, _aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readInt64(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

            ::android::binder::Status BpTelemetryAgent::version(::android::String16* _aidl_return)
            {
                ::android::Parcel _aidl_data;
                ::android::Parcel _aidl_reply;
                ::android::status_t _aidl_ret_status = ::android::OK;
                ::android::binder::Status _aidl_status;
                _aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = remote()->transact(::android::IBinder::FIRST_CALL_TRANSACTION + 12 /* version */, _aidl_data, &_aidl_reply);
                if (UNLIKELY(_aidl_ret_status == ::android::UNKNOWN_TRANSACTION && ITelemetryAgent::getDefaultImpl()))
                {
                    return ITelemetryAgent::getDefaultImpl()->version(_aidl_return);
                }
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                _aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
                if (!_aidl_status.isOk())
                {
                    return _aidl_status;
                }
                _aidl_ret_status = _aidl_reply.readString16(_aidl_return);
                if (((_aidl_ret_status) != (::android::OK)))
                {
                    goto _aidl_error;
                }
            _aidl_error:
                _aidl_status.setFromStatusT(_aidl_ret_status);
                return _aidl_status;
            }

        }  // namespace telemetry

    }  // namespace microsoft

}  // namespace com
#include "BnTelemetryAgent.h"
#include <binder/Parcel.h>

namespace com
{
    namespace microsoft
    {
        namespace telemetry
        {
            BnTelemetryAgent::BnTelemetryAgent()
            {
            }

            ::android::status_t BnTelemetryAgent::onTransact(uint32_t _aidl_code, const ::android::Parcel& _aidl_data, ::android::Parcel* _aidl_reply, uint32_t _aidl_flags)
            {
                ::android::status_t _aidl_ret_status = ::android::OK;
                switch (_aidl_code)
                {
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 0 /* open */:
                {
                    ::android::String16 in_config;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readString16(&in_config);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(open(in_config, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 1 /* close */:
                {
                    int64_t in_id;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(close(in_id, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 2 /* clear */:
                {
                    int64_t in_id;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(clear(in_id, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 3 /* config */:
                {
                    int64_t in_id;
                    ::android::String16 in_config;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readString16(&in_config);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(config(in_id, in_config, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 4 /* writeEvent */:
                {
                    int64_t in_id;
                    int64_t in_contentType;
                    ::std::vector<uint8_t> in_data;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_contentType);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readByteVector(&in_data);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(writeEvent(in_id, in_contentType, in_data, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 5 /* writeBlob */:
                {
                    int64_t in_id;
                    ::android::base::unique_fd in_pfd;
                    ::android::String16 in_name;
                    ::android::String16 in_contentType;
                    int64_t in_byteCount;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readUniqueFileDescriptor(&in_pfd);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readString16(&in_name);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readString16(&in_contentType);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_byteCount);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(writeBlob(in_id, std::move(in_pfd), in_name, in_contentType, in_byteCount, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 6 /* writeMetadata */:
                {
                    int64_t in_id;
                    ::std::vector<::android::String16> in_metadata;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readString16Vector(&in_metadata);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(writeMetadata(in_id, in_metadata, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 7 /* pause */:
                {
                    int64_t in_id;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(pause(in_id, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 8 /* resume */:
                {
                    int64_t in_id;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(resume(in_id, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 9 /* upload */:
                {
                    int64_t in_id;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_id);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(upload(in_id, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 10 /* stop */:
                {
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    ::android::binder::Status _aidl_status(stop(&_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 11 /* opmode */:
                {
                    int64_t in_mode;
                    int64_t _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    _aidl_ret_status = _aidl_data.readInt64(&in_mode);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    ::android::binder::Status _aidl_status(opmode(in_mode, &_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeInt64(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                case ::android::IBinder::FIRST_CALL_TRANSACTION + 12 /* version */:
                {
                    ::android::String16 _aidl_return;
                    if (!(_aidl_data.checkInterface(this)))
                    {
                        _aidl_ret_status = ::android::BAD_TYPE;
                        break;
                    }
                    ::android::binder::Status _aidl_status(version(&_aidl_return));
                    _aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                    if (!_aidl_status.isOk())
                    {
                        break;
                    }
                    _aidl_ret_status = _aidl_reply->writeString16(_aidl_return);
                    if (((_aidl_ret_status) != (::android::OK)))
                    {
                        break;
                    }
                }
                break;
                default:
                {
                    _aidl_ret_status = ::android::BBinder::onTransact(_aidl_code, _aidl_data, _aidl_reply, _aidl_flags);
                }
                break;
                }
                if (_aidl_ret_status == ::android::UNEXPECTED_NULL)
                {
                    _aidl_ret_status = ::android::binder::Status::fromExceptionCode(::android::binder::Status::EX_NULL_POINTER).writeToParcel(_aidl_reply);
                }
                return _aidl_ret_status;
            }

        }  // namespace telemetry

    }  // namespace microsoft

}  // namespace com
