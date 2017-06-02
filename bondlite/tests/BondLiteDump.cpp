#include <bond_lite/All.hpp>
#include <bond/generated/BondConstTypes.hpp>
#include <fstream>
#include <stdio.h>

namespace bond_lite {


bool dumpStruct(int indent, bond_lite::CompactBinaryProtocolReader& reader, bool isBase = false);

bool dumpField(int indent, bond_lite::CompactBinaryProtocolReader& reader, uint8_t type)
{
    switch (type) {
        case BT_BOOL: {
            bool x;
            if (!reader.ReadBool(x)) {
                printf("ReadBool() failed\n");
                return false;
            }
            printf("(bool) %s", x ? "true" : "false");
            break;
        }

        case BT_UINT8: {
            uint8_t x;
            if (!reader.ReadUInt8(x)) {
                printf("ReadUInt8() failed\n");
                return false;
            }
            printf("(uint8) %u", x);
            break;
        }

        case BT_UINT16: {
            uint16_t x;
            if (!reader.ReadUInt16(x)) {
                printf("ReadUInt16() failed\n");
                return false;
            }
            printf("(uint16) %u", x);
            break;
        }

        case BT_UINT32: {
            uint32_t x;
            if (!reader.ReadUInt32(x)) {
                printf("ReadUInt32() failed\n");
                return false;
            }
            printf("(uint32) %u", x);
            break;
        }

        case BT_UINT64: {
            uint64_t x;
            if (!reader.ReadUInt64(x)) {
                printf("ReadUInt64() failed\n");
                return false;
            }
            printf("(uint64) %llu", static_cast<unsigned long long>(x));
            break;
        }

        case BT_INT8: {
            int8_t x;
            if (!reader.ReadInt8(x)) {
                printf("ReadInt8() failed\n");
                return false;
            }
            printf("(int8) %d", x);
            break;
        }

        case BT_INT16: {
            int16_t x;
            if (!reader.ReadInt16(x)) {
                printf("ReadInt16() failed\n");
                return false;
            }
            printf("(int16) %d", x);
            break;
        }

        case BT_INT32: {
            int32_t x;
            if (!reader.ReadInt32(x)) {
                printf("ReadInt32() failed\n");
                return false;
            }
            printf("(int32) %d", x);
            break;
        }

        case BT_INT64: {
            int64_t x;
            if (!reader.ReadInt64(x)) {
                printf("ReadInt64() failed\n");
                return false;
            }
            printf("(int64) %lld", static_cast<long long>(x));
            break;
        }

        case BT_STRING: {
            std::string x;
            if (!reader.ReadString(x)) {
                printf("ReadString() failed\n");
                return false;
            }
            printf("(string) \"%s\"", x.c_str());
            break;
        }

        case BT_LIST: {
            uint32_t size;
            if (!reader.ReadContainerBegin(size, type)) {
                printf("ReadContainerBegin() failed\n");
                return false;
            }
            printf("(list) [\n");
            for (unsigned i = 0; i < size; i++) {
                printf("%*s", (indent + 1) * 2, "");
                if (!dumpField(indent + 1, reader, type)) {
                    return false;
                }
                printf("\n");
            }
            printf("%*s]", indent * 2, "");
            break;
        }

        case BT_SET: {
            uint32_t size;
            if (!reader.ReadContainerBegin(size, type)) {
                printf("ReadContainerBegin() failed\n");
                return false;
            }
            printf("(set) <\n");
            for (unsigned i = 0; i < size; i++) {
                printf("%*s", (indent + 1) * 2, "");
                if (!dumpField(indent + 1, reader, type)) {
                    return false;
                }
                printf("\n");
            }
            printf("%*s>", indent * 2, "");
            break;
        }

        case BT_MAP: {
            uint32_t size;
            uint8_t type2;
            if (!reader.ReadMapContainerBegin(size, type, type2)) {
                printf("ReadMapContainerBegin() failed\n");
                return false;
            }
            printf("(map) {\n");
            for (unsigned i = 0; i < size; i++) {
                printf("%*s", (indent + 1) * 2, "");
                if (!dumpField(indent + 1, reader, type)) {
                    return false;
                }
                printf(" = ");
                if (!dumpField(indent + 1, reader, type2)) {
                    return false;
                }
                printf("\n");
            }
            printf("%*s}", indent * 2, "");
            break;
        }

        case BT_STRUCT: {
            printf("(struct) {\n");
            dumpStruct(indent, reader, false);
            printf("%*s}", indent * 2, "");
            break;
        }

        default:
            printf("(unknown:%u)", type);
            return false;
    }

    return true;
}

bool dumpStruct(int indent, bond_lite::CompactBinaryProtocolReader& reader, bool isBase)
{
    if (!reader.ReadStructBegin(isBase)) {
        printf("ReadStructBegin() failed\n");
        return false;
    }

    uint8_t type;
    uint16_t id;
    for (;;) {
        if (!reader.ReadFieldBegin(type, id)) {
            printf("ReadFieldBegin() failed\n");
            return false;
        }

        if (type == BT_STOP || type == BT_STOP_BASE) {
            if (isBase != (type == BT_STOP_BASE)) {
                printf("BT_STOP_BASE in non-base or vice versa\n");
                return false;
            }
            break;
        }

        printf("%*s<%u> = ", (indent + 1) * 2, "", id);
        dumpField(indent + 1, reader, type);
        printf("\n");

        if (!reader.ReadFieldEnd()) {
            printf("ReadFieldEnd() failed\n");
            return false;
        }
    }

    if (!reader.ReadStructEnd(isBase)) {
        printf("ReadStructEnd() failed\n");
        return false;
    }

    return true;
}


} // namespace bond_lite


int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("BondLiteDump <input.bond>\n");
        return -1;
    }

    std::ifstream f(argv[1], std::ios::binary);
    if (!f) {
        printf("Could not open %s\n", argv[1]);
        return -1;
    }
    std::vector<uint8_t> input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    bond_lite::CompactBinaryProtocolReader reader(input);
    printf("(struct) {\n");
    dumpStruct(0, reader);
    printf("}\n");

    return 0;
}
