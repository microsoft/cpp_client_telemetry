#ifndef VARIANTTYPE_HPP
#define VARIANTTYPE_HPP

//class VariantMap;
//class VariantArray;

// Constructor and getter for Variant type
#define VARIANT_PROP(basetype, field, typeenum)						\
		Variant(basetype v) : field(v), type(typeenum) {} ;			\
		operator basetype() { return (basetype)field; };

/**
 * Variant type for containers
 */
class Variant {

    union {
        int64_t		 iV;
        double		 dV;
        const char*  sV;
        bool		 bV;
        std::string* SV;
    };

    VariantMap      mV;	// map
    VariantArray    aV;	// vector

public:

    enum Type {
        TYPE_NULL,
        TYPE_INT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_STRING2,
        TYPE_BOOL,
        TYPE_OBJ,
        TYPE_ARR
    };

    Type type;

    Variant& Null() {
        static Variant nullVariant;
        return nullVariant;
    }

    Variant() : iV(0), type(TYPE_NULL) {};

    // All integer types
    VARIANT_PROP(int8_t,   iV, TYPE_INT);
    VARIANT_PROP(int16_t,  iV, TYPE_INT);
    VARIANT_PROP(int32_t,  iV, TYPE_INT);
    VARIANT_PROP(int64_t,  iV, TYPE_INT);
    VARIANT_PROP(uint8_t,  iV, TYPE_INT);
    VARIANT_PROP(uint16_t, iV, TYPE_INT);
    VARIANT_PROP(uint32_t, iV, TYPE_INT);
    VARIANT_PROP(uint64_t, iV, TYPE_INT);

    // All floating point types
    VARIANT_PROP(float, dV, TYPE_DOUBLE);
    VARIANT_PROP(double, dV, TYPE_DOUBLE);

    // const char * as a string type
    // TODO: should non-existing key return "" or nullptr?
    // TODO: helper for the case to get const char* from TYPE_STRING2
    VARIANT_PROP(const char*, sV, TYPE_STRING);

    // Bolean
    VARIANT_PROP(bool, bV, TYPE_BOOL);

    // Object (or map)
    Variant(VariantMap& m) :
        mV(m),
        type(TYPE_OBJ) {};

    // C++11 initializer list support for maps
    Variant(std::initializer_list<std::pair<std::string, Variant> > l) :
        type(TYPE_OBJ) {
        for (auto kv : l) {
            mV.insert(kv);
        }
    }

    // Array (or vector)
    Variant(VariantArray& a) :
        aV(a),
        type(TYPE_ARR) {};

    // Destroy all elements
    virtual ~Variant() {
        if (type == TYPE_OBJ) {
            mV.clear();
        }
        else
        if (type == TYPE_ARR) {
            aV.clear();
        }
        else
        if (type == TYPE_STRING2) {
            delete SV;
        }
    }

    operator std::string&()
    {
        return (*SV);
    }

    Variant(std::string v) :
        SV(new std::string(v)),
        type(TYPE_STRING2) {};

    /**
     *
     */
    operator VariantMap&() {
        return mV;
    };

    /**
     *
     */
    operator VariantArray&() {
        return aV;
    };

    /**
     *
     */
    Variant& operator [](const char* k) {
        if (type == TYPE_OBJ)
            return mV[k];
        return Null();
    };

    /**
     *
     */
    Variant& operator ()(size_t idx) {
        if (type == TYPE_ARR)
            return aV[idx];
        return Null();
    };

    /**
     *
     */
    static std::string escape(const std::string &s) {
        std::ostringstream o;
        for (auto c = s.cbegin(); c != s.cend(); c++) {
            switch (*c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= *c && *c <= '\x1f') {
                    o << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
                }
                else {
                    o << *c;
                }
            }
        }
        return o.str();
    }

    /**
     * Serialize variant value. Append to out.
     */
    static void serialize(Variant &v, std::string& out)
    {
        switch (v.type)
        {
        case Variant::TYPE_NULL:
            out += "null";
            break;

        case Variant::TYPE_INT:
            out += std::to_string((int64_t)v);
            break;

        case Variant::TYPE_DOUBLE:
            out += std::to_string((double)v);
            break;

        case Variant::TYPE_STRING:
            out += "\"";
            out += escape((const char *)v);
            out += "\"";
            break;

        case Variant::TYPE_STRING2:
            out += "\"";
            out += escape((std::string&)v);
            out += "\"";
            break;

        case Variant::TYPE_BOOL:
            out += (bool)v ? "true" : "false";
            break;

        case Variant::TYPE_OBJ:
            serialize((VariantMap&)v, out);
            break;

        case Variant::TYPE_ARR: {
            out += "[";
            size_t idx = 0;
            VariantArray & arr = v;
            for (auto &item : arr)
            {
                serialize(item, out);
                idx++;
                if (idx != arr.size())
                    out += ",";
            }
            out += "]";
            break;
        }

        default:
            break;
        }
    }

    /**
     * Serialize variant object (map). Append to out.
     */
    static void serialize(VariantMap &varMap, std::string& out)
    {
        using namespace std;
        out += "{";
        unsigned idx = 0;
        for (auto &kv : varMap)
        {
            const std::string& key = kv.first;
            Variant &v = kv.second;
            out += "\"";
            out += key;
            out += "\":";
            serialize(v, out);
            idx++;
            if (idx != varMap.size())
                out += ",";
        }
        out += "}";
    }

    /**
     * C++11 initializer list support for vectors
     */
    static Variant from_array(std::initializer_list<Variant> l) {
        VariantArray arr(l);
        return Variant(arr);
    }

    template<class Map>
    static Variant from_map(Map& src)
    {
        VariantMap m;
        for (auto &kv : src)
            m[kv.first] = Variant(kv.second);
        return Variant(m);
    }

    template<class Vector>
    static Variant from_vector(Vector& src)
    {
        VariantArray a;
        for (auto &v : src)
            a.push_back(v);
        return Variant(a);
    }

};

#endif
