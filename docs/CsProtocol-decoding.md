# SDK protocol decoder

## Payload Decoder Implementation

1DS C++ SDK provides a built-in protocol decoder for the following formats:

* **Package of Records** - either compressed or uncompressed HTTP post body containing binary-serialized events
* **Common Schema Record**

The decoder consists of:

* implementation in [PayloadDecoder.cpp](../lib/decoder/PayloadDecoder.cpp)
* public header in [PayloadDecoder.hpp](../lib/include/public/PayloadDecoder.hpp)

Decoder requires zlib and json.hpp libraries to be present in the build.
Compact SDK build that do not include these libs cannot provide the event decoding functionality.

Callback function in user code that implements a transform from Common Schema Record must include [CsProtocol_types.hpp](../lib/bond/generated/CsProtocol_types.hpp)
Note that the Common Schema Record type does not provide ABI stability and may change release over release without further notice.
User code has to be compiled with matching definition of the Common Schema Record. This is advanced functionality, usage of that
implies a good understanding of SDK debug / event callback mechanism and is limited to applications and SDKs that compile
1DS C++ SDK from source. It is not recommended for an application that dynamically loads the SDK as a prebuilt module to
rely on payload decoder due to no ABI-stability guarantee.

## Payload Decoder usage example

Please refer to usage examples:

* EventDecoderListener.cpp [EventDecoderListener::DecodeBuffer](../tests/functests/EventDecoderListener.cpp) for HTTP(s) request - Package of Records decoding
* APITest.cpp [UTC_Callback_Test](../tests/functests/APITest.cpp) for Common Schema Record decoding
