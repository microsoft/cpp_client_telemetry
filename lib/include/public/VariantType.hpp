#ifndef VARIANTTYPE_HPP
#define VARIANTTYPE_HPP

// Default implementation doesn't provide locking
#ifndef VARIANT_LOCKGUARD
#define VARIANT_LOCKGUARD(x)
#define VARIANT_LOCK(x)
#endif

// Constructor and getter for Variant type
#define VARIANT_PROP(basetype, field, typeenum)                 \
    Variant(basetype v) : field(v), type(typeenum) {} ;         \
    operator basetype() { return (basetype)field; };            \
    Variant& operator=(basetype v) { field = v; type = typeenum; return *this; };

/**
 * Variant type for containers
 */
class Variant {

    union {
        int64_t     iV;
        double      dV;
        const char* sV;
        bool        bV;
    };

    // Unfortunately keeping object pointers inside the union above causes issues
    // with C++11 static initializer feature. The pointers get corrupted and calling
    // destructor via delete causes a crash with MSVC compiler (both 2015 and 2017).
    std::string     SV;
    VariantMap      mV;	// map
    VariantArray    aV;	// vector

public:

    // Thread-safe variants
    VARIANT_LOCK(lock_object);

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

    Variant& ConstNull() {
        static Variant nullVariant;
        return nullVariant;
    }

    Variant() : iV(0), type(TYPE_NULL) {};

    // All integer types
    VARIANT_PROP(int8_t, iV, TYPE_INT);
    VARIANT_PROP(int16_t, iV, TYPE_INT);
    VARIANT_PROP(int32_t, iV, TYPE_INT);
    VARIANT_PROP(int64_t, iV, TYPE_INT);
    VARIANT_PROP(uint8_t, iV, TYPE_INT);
    VARIANT_PROP(uint16_t, iV, TYPE_INT);
    VARIANT_PROP(uint32_t, iV, TYPE_INT);
    VARIANT_PROP(uint64_t, iV, TYPE_INT);

    // All floating point types
    VARIANT_PROP(float, dV, TYPE_DOUBLE);
    VARIANT_PROP(double, dV, TYPE_DOUBLE);

    Variant(const char* v) : sV(v), type(TYPE_STRING) {};

    operator const char*() {
        if (type == TYPE_STRING)
            return sV;
        if (type == TYPE_STRING2)
            return SV.c_str();
        if (type == TYPE_NULL)
            return "";
        return nullptr;
    };

    Variant& operator=(std::string value)
    {
        VARIANT_LOCKGUARD(lock_object);

        // Cannot assign to const Null (non-existing) element
        assert(&ConstNull() != this);

        type = TYPE_STRING2;
        SV = value;
        sV = SV.c_str();
        return *this;
    }

    Variant& operator=(const char* value)
    {
        VARIANT_LOCKGUARD(lock_object);

        // Cannot assign to const Null (non-existing) element
        assert(&ConstNull() != this);

        type = TYPE_STRING2;
        SV = (value) ? value : "";
        sV = SV.c_str();
        return *this;
    }

    Variant& assign(const Variant& other)
    {
        VARIANT_LOCKGUARD(lock_object);

        // Cannot assign to const Null (non-existing) element
        assert(&ConstNull() != this);

        type = other.type;
        switch (other.type)
        {
        case TYPE_NULL:
            iV = 0;
            break;
        case TYPE_INT:
            iV = other.iV;
            break;
        case TYPE_DOUBLE:
            dV = other.dV;
            break;

        case TYPE_STRING:
            type = TYPE_STRING2;
            SV = (other.sV) ? other.sV : "";
            break;

        case TYPE_STRING2:
            SV = other.SV;
            break;

        case TYPE_BOOL:
            bV = other.bV;
            break;

        case TYPE_OBJ:
            for (const auto& kv : other.mV)
            {
                mV[kv.first] = kv.second;
            }
            break;

        case TYPE_ARR:
            // std::swap(aV, other.aV);
            break;
        }

        return *this;
    }

    Variant & operator=(Variant other)
    {
        return assign(other);
    }

    // Bolean
    VARIANT_PROP(bool, bV, TYPE_BOOL);

    // Object (or map)
    Variant(VariantMap& m) :
        type(TYPE_OBJ)
    {
        for (const auto& kv : m)
        {
            mV[kv.first] = kv.second;
        }
    };

    // C++11 initializer list support for maps
    Variant(const std::initializer_list<std::pair<std::string, Variant> >& l) :
        type(TYPE_OBJ) {
        for (const auto& kv : l) {
            mV[kv.first] = kv.second;
            // mV.insert(kv);
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
    }

    operator std::string&()
    {
        VARIANT_LOCKGUARD(lock_object);
        assert(type == TYPE_STRING2);
        return SV;
    }

    Variant(const std::string& v) :
        SV(v),
        type(TYPE_STRING2)
    {
        sV = SV.c_str();
    };

    /**
     *
     */
    operator VariantMap&() {
        VARIANT_LOCKGUARD(lock_object);
        return mV;
    };

    /**
     *
     */
    operator VariantArray&() {
        VARIANT_LOCKGUARD(lock_object);
        return aV;
    };

    /**
     *
     */
    Variant& operator [](const char* k) {
        VARIANT_LOCKGUARD(lock_object);
        if (type == TYPE_OBJ)
            return mV[k];

        // If we're trying to obtain a property on non-existing
        // element, then first we change the type of it to obj
        // and populate the property with an empty NULL property.
        // Thus essentially creating an object and returning
        // modifyable null variant for the object property.
        if (type == TYPE_NULL)
        {
            type = TYPE_OBJ;
            mV[k] = Variant();
            return mV[k];
        }

        // Otherwise it's an invalid op - return const NULL
        return ConstNull();
    };

    /**
     *
     */
    Variant& operator ()(size_t idx) {
        VARIANT_LOCKGUARD(lock_object);
        if (type == TYPE_ARR)
            return aV[idx];
        // Accessing index of something that is not an array returns a null const variant.
        // We may consider adding an assert here for debug builds.
        return ConstNull();      // return const NULL Variant
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
        VariantArray arr;
        for (const auto& v : l)
            arr.push_back(v);
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
