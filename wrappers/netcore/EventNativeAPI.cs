#pragma warning disable IDE1006 // ignore naming rule violations: we preserve original C API naming for clarity here
#pragma warning disable IDE0044 // ignore readonly suggestion for field passed over P/Invoke

using System;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;
using System.Json;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Collections;

namespace Microsoft
{
    namespace Telemetry
    {
        namespace Core
        {
            public class Constants
            {
                public const string LIBRARY_NAME_POSIX = "libmat";
                public const string LIBRARY_NAME_WIN32 = "ClientTelemetry.dll";
                public const string VERSION = "3.3.0-netcore";
            }

            public enum EventCallType
            {
                EVT_OP_LOAD = 0x00000001,
                EVT_OP_UNLOAD = 0x00000002,
                EVT_OP_OPEN = 0x00000003,
                EVT_OP_CLOSE = 0x00000004,
                EVT_OP_CONFIG = 0x00000005,
                EVT_OP_LOG = 0x00000006,
                EVT_OP_PAUSE = 0x00000007,
                EVT_OP_RESUME = 0x00000008,
                EVT_OP_UPLOAD = 0x00000009,
                EVT_OP_FLUSH = 0x0000000A,
                EVT_OP_VERSION = 0x0000000B,
                EVT_OP_OPEN_WITH_PARAMS = 0x0000000C,
                EVT_OP_MAX = EVT_OP_OPEN_WITH_PARAMS + 1
            }

            public enum EventPropertyType
            {
                /* Basic types */
                TYPE_STRING = 0,
                TYPE_INT64 = 1,
                TYPE_DOUBLE = 2,
                TYPE_TIME = 3,
                TYPE_BOOLEAN = 4,
                TYPE_GUID = 5,
                /* Arrays of basic types */
                TYPE_STRING_ARRAY = 6,
                TYPE_INT64_ARRAY = 7,
                TYPE_DOUBLE_ARRAY = 8,
                TYPE_TIME_ARRAY = 9,
                TYPE_BOOL_ARRAY = 10,
                TYPE_GUID_ARRAY = 11,
                /* NULL-type */
                TYPE_NULL = 12
            }

            public struct PiiValue
            {
                public string value;
                public UInt32 piKind;
            }

            public class EventProperties : Dictionary<string, object>
            {
                public EventProperties()
                {
                }
            };

            [StructLayout(LayoutKind.Explicit, Size = 16, CharSet = CharSet.Ansi)]
            public unsafe struct EventGUIDType
            {
                /**
                 * <summary>
                 * Specifies the first eight hexadecimal digits of the GUID.
                 * </summary>
                 */
                [FieldOffset(0)] UInt32 Data1;

                /* <summary>
                 * Specifies the first group of four hexadecimal digits.
                 * </summary>
                 */
                [FieldOffset(4)] UInt16 Data2;

                /**
                 * <summary>
                 * Specifies the second group of four hexadecimal digits.
                 * </summary>
                 */
                [FieldOffset(6)] UInt16 Data3;

                /** <summary>
                 * An array of eight bytes.
                 * The first two bytes contain the third group of four hexadecimal digits.
                 * The remaining six bytes contain the final 12 hexadecimal digits.
                 * </summary>
                 */
                [FieldOffset(8)]
                fixed byte Data4[8];
            }

            [StructLayout(LayoutKind.Explicit, Size = 24, CharSet = CharSet.Ansi)]
            public struct EventContextType
            {
                [FieldOffset(0)] public UInt32 call;
                [FieldOffset(4)] public ulong handle;
                [FieldOffset(12)] public IntPtr data;
                [FieldOffset(16)] public UInt32 result;
                [FieldOffset(20)] public UInt32 size;
            }

            public enum EventOpenParamType
            {
                OPEN_PARAM_TYPE_HTTP_HANDLER_SEND = 0,
                OPEN_PARAM_TYPE_HTTP_HANDLER_CANCEL = 1,
                OPEN_PARAM_TYPE_TASK_DISPATCHER_QUEUE = 2,
                OPEN_PARAM_TYPE_TASK_DISPATCHER_CANCEL = 3,
                OPEN_PARAM_TYPE_TASK_DISPATCHER_JOIN = 4
            }

            [StructLayout(LayoutKind.Explicit, Size = 12, CharSet = CharSet.Ansi)]
            public struct EventOpenParam
            {
                [FieldOffset(0)] public UInt32 type;
                [FieldOffset(4)] public IntPtr data;
            };

            /**
             * <summary>
             * Wraps logger configuration string and all input parameters to 'evt_open_with_params'
             * </summary>
             */
            // TODO: this structure size depends on architecture - 32-bit or 64-bit..
            // Since all Mac OS X and Linux are mostly 64-bit by now, as well as
            // most Windows - we should assume that the struct layout is optimized
            // for 64-bit. From a practical standpoint - somebody building .NET Core
            // app would likely consider running it on a platform that is modern
            // enough. One way to solve this issue for 32-bit machines is to add a
            // custom SDK build flag that enforces certain struct layout. i.e.
            // positioning the two pointers below as 64-bit integers instead of 32-bit.
            [StructLayout(LayoutKind.Explicit, Size = 20, CharSet = CharSet.Ansi)]
            public struct EventOpenWithParamsDataType
            {
                [FieldOffset(0)] public IntPtr config;
                [FieldOffset(8)] public IntPtr parameters; /* pointer to array of EventOpenParam */
                [FieldOffset(8)] public UInt32 paramsCount;
            }

            [StructLayout(LayoutKind.Explicit, Size = 8, CharSet = CharSet.Ansi)]
            public struct EventPropertyValue
            {
                /* Basic types */
                [FieldOffset(0)] UInt64 as_uint64;
                [FieldOffset(0)] IntPtr as_string;
                [FieldOffset(0)] Int64 as_int64;
                [FieldOffset(0)] double as_double;
                [FieldOffset(0)] bool as_bool;
                [FieldOffset(0)] IntPtr as_guid;
                [FieldOffset(0)] UInt64 as_time;
#if FALSE
                /* We don't support passing arrays yet. Code below needs to be ported from C++ to C# */
                /* Array types are nullptr-terminated array of pointers */
                char**              as_arr_string;
                int64_t**           as_arr_int64;
                bool**              as_arr_bool;
                double**            as_arr_double;
                evt_guid_t**        as_arr_guid;
                uint64_t**          as_arr_time;
#endif
            };

            [StructLayout(LayoutKind.Explicit, Size = 16, CharSet = CharSet.Ansi)]
            public struct EventProperty
            {
                [FieldOffset(0)] IntPtr name;
                [FieldOffset(4)] EventPropertyType type;
                [FieldOffset(8)] EventPropertyValue value;
                [FieldOffset(12)] UInt32 piiKind;
            }

            /**
             * <summary>
             * Identifies HTTP request method type
             * </summary>
             */
            public enum HttpRequestType
            {
                HTTP_REQUEST_TYPE_GET = 0,
                HTTP_REQUEST_TYPE_POST = 1,
            }

            public static class EventNativeAPI
            {
                // Conditional compilation: pass different library name depending on target OS

                [DllImport(Constants.LIBRARY_NAME_POSIX, EntryPoint = "evt_api_call_default")]
                internal static extern UInt32 evt_api_call([In, Out] ref EventContextType context);

                /**
                 * <summary>
                 * Create or open existing SDK instance.
                 * </summary>
                 * <param name="config">SDK configuration.</param>
                 * <returns>SDK instance handle.</returns>
                 */
                public static ulong evt_open(string cfg)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_OPEN,
                        data = Marshal.StringToHGlobalAnsi(cfg)
                    };
                    evt_api_call(ref context);
                    return context.handle;
                }

                /**
                 * <summary>
                 * Destroy or close SDK instance by handle
                 * </summary>
                 * <param name="handle">SDK instance handle.</param>
                 * <returns>Status code.</returns>
                 */
                public static ulong evt_close(ulong inHandle)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_CLOSE,
                        handle = inHandle
                    };
                    return evt_api_call(ref context);
                }

                public static ulong evt_log(ulong inHandle, ref EventProperties properties)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_LOG,
                        handle = inHandle
                    };
                    return evt_api_call(ref context);
                }

                /**
                 * <summary>
                 * Pauses transmission. In that mode events stay in ram or saved to disk, not sent.
                 * </summary>
                 * <param name="handle">SDK handle.</param>
                 * <returns>Status code.</returns>
                 */
                public static ulong evt_pause(ulong inHandle)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_PAUSE,
                        handle = inHandle
                    };
                    return evt_api_call(ref context);
                }

                /**
                 * <summary>
                 * Resumes transmission. Pending telemetry events should be attempted to be sent.
                 * </summary>
                 * <param name="handle">SDK handle.</param>
                 * <returns>Status code.</returns>
                 */
                public static ulong evt_resume(ulong inHandle)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_RESUME,
                        handle = inHandle
                    };
                    return evt_api_call(ref context);
                }

                /** <summary>
                 * Provide a hint to telemetry system to attempt force-upload of events
                 * without waiting for the next batch timer interval. This API does not
                 * guarantee the upload.
                 * </summary>
                 * <param name="handle">SDK handle.</param>
                 * <returns>Status code.</returns>
                 */
                public static ulong evt_upload(ulong inHandle)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_RESUME,
                        handle = inHandle
                    };
                    return evt_api_call(ref context);
                }

                /** <summary>
                 * Save pending telemetry events to offline storage on disk.
                 * </summary>
                 * <param name="handle">SDK handle.</param>
                 * <returns>Status code.</returns>
                 */
                public static ulong evt_flush(ulong inHandle)
                {
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_FLUSH,
                        handle = inHandle
                    };
                    return evt_api_call(ref context);
                }

                /** <summary>
                 * Pass down SDK header version to SDK library. Needed for late binding version checking.
                 * This method provides means of a handshake between library header and a library impl.
                 * It is up to app dev to verify the value returned, making a decision whether some SDK
                 * features are implemented/supported by particular SDK version or not.
                 * </summary>
                 * <param name="libSemver">SDK header semver.</param>
                 * <returns>SDK library semver</returns>
                 */
                public static string evt_version()
                {
                    byte[] data = Encoding.ASCII.GetBytes(Constants.VERSION);
                    var nativeDataPtr = Marshal.AllocHGlobal(data.Length + 1);
                    Marshal.Copy(data, 0, nativeDataPtr, data.Length);
                    Marshal.WriteByte(nativeDataPtr + data.Length, 0);
                    EventContextType context = new EventContextType
                    {
                        call = (Byte)EventCallType.EVT_OP_VERSION,
                        data = nativeDataPtr
                    };
                    evt_api_call(ref context);
                    return Marshal.PtrToStringAnsi(context.data);
                }
            }

        }
    }
}