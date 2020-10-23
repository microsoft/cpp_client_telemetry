# Event Attributes and Property List flattening

This document illustrates the mapping from 'flat' Event property lists to JavaScript Object Notation (JSON) Pointer.

Flat map of JSON Pointer notation key-value pairs can be unflattened into JSON object.

See [RFC6901](https://tools.ietf.org/html/rfc6901) for more details.

## Supported property list formats at SDK API layer

Supported property list input formats are:

- **UTC-ETW** event key-value pairs - requires **PartA_** prefix for property names.

- **Geneva-ETW** event key-value pairs - requires **env_** prefix for property names.

- **JSONPath** in dotted notation - no special prefix property names, e.g. **ext.device.id**

All of these notations could be converted to RFC6901 first. Then unflattened into JSON object:

```console
+---------------+    +--------------+    +-------------+
| Property List | => | RFC6091 List | => | JSON object |
+---------------+    +--------------+    +-------------+
```

## Reference implementation

This document provides generic guidelines applicable to any programming language.

Reference implementation is provided in C++.

Code below:

- reference implementation of the transformations in C++.
- uses STL ```map<string, string>``` for illustrative purposes. Real-world SDK will operate on ```EventProperties``` map or ```map<string, variant<...>>```
- implementation for other languages is not provided, but should no exceed ~10 lines of code.
- [nlohmann/json](https://github.com/nlohmann/json) library is included in 1DS C++ SDK.

Several existing implementations of of JSON Pointer can be found at [JSON Patch specification page](http://jsonpatch.com/), which also uses JSON Pointer to express individual JSON tree nodes paths. [Asp.Net Core JsonPatch](https://github.com/aspnet/JsonPatch) is Microsoft official implementation for C#.

## Background

1DS SDKs provide several options of populating event contents:

- ```SetProperty(string key, variant value)```
- initializer lists - "fluent", inline, almost JSON-style initialization for C++

SetProperty usage example:

```cpp
    EventProperties evt(eventName);
    evt.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, PDT_ProductAndServicePerformance);
    evt.SetProperty(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_OPTIONAL);
```

Initializer list usage example:

```cpp
   EventProperties evt(eventName,
       {
           { "strKey1",  "hello1" },
           { "strKey2",  "hello2" },
           { "int64Key", int64_t(1L) },
           { "dblKey",   3.14 },
           { "boolKey",  false },
           { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
           { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
           { "guidKey2", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
       });
```

## Expressing deep nested structures with 1DS SDK API

Example:

```cpp
   EventProperties evt(eventName,
       {
           { "ext.device.id",       "c:12345678" },
           { "ext.app.name",        "c:myCustomAppName" }
           { "ext.metadata.fooBar", 12345 }
       });
```

Note that in this example we used JSONPath dotted notation as property name.

Other notable examples of property names:

| Property | Notation |
|--|--|
|ext.foo.bar|JSONPath|
|/ext/foo/bar| JSON Pointer (rfc6901)|
|PartA_ext_foo_bar|UTC-ETW property name|
|env_foo_bar|Geneva-ETW property name|

All of these notations can be converted to JSON Pointer path (rfc6901) in a lossless, one-to-one mapping manner.

Subsequently JSON Pointer path notation can be unflattened into JSON object by the Agent, such as UTC or Geneva Agent.

It may be necessary on ETW path to Agent to utilize underscores instead of dots, such as in UTC and Geneva format examples above. Using underscores instead of dots may also be necessary for several backend flows that do not support fields with dots.

SDK API surface accepts any notation, then automatically translates into corresponding notation supported by lower-level transport at exporter layer.

1DS C++ SDK, in particular, performs transform from EventProperties lists in any notation to UTC-ETW notation in UTC module.

Separate implementation will be provided to transform to CsRecord on Bond (direct upload) path.

## Handling of properties that include a reserved prefix

Reserved prefixes are:

```console
/       - JSON Pointer
ext     - Common Schema extension
env_    - Geneva Agent -specific alias for extension
PartA_  - UTC Agent -specific alias for 'root' level
```

Prefix allows to signify that a property is either top-level property or Part A/B property.

## Handling of properties that do not include a reserved prefix

Properties that do not include a prefix, such as '/', 'ext.', 'env_' - get appended to user data (Part C) property bag.

Note that from SDK API layer perspective, top-level elements can be expressed as:

```console
/ver
/time
/name
```

SDK will subsequently transform those top-level properties into corresponding notation at exporter layer.

For example, when those properties get passed to UTC, they'd be populated as:

```console
PartA_ver
PartA_time
PartA_name
```

Another possible alternative is to simply replace the 'PartA' prefix with '_' as follows:

```console
_ver
_time
_name
```

This will restrict Part C (user, custom data) property names from starting with underscores. But 1DS flows already restrict that. Thus, there is no ambiguity between Part C property and Part A property. Only Part A properties would start with underscore.

Conversion from underscores '_' notation to JSON Pointer notation - forward slashes '/' - is also trivial.

## Implementation of routine that converts any property list key to JSON Pointer path

Function below accepts a property list key and transforms it into JSON Pointer format:

```cpp
std::string to_json_patch(const std::string& key, ListFormat fmt = ListFormat::JSONPath)
{
    switch (fmt)
    {
    case ListFormat::JSONPath: {
        std::string result("/");
        result += key;
        std::replace(result.begin(), result.end(), '.', '/');
        return result;
    }

    case ListFormat::UTC: {
        size_t ofs = (key.find("PartA_") == 0) ? 5 : 0;
        std::string result(key.c_str() + ofs);
        std::replace(result.begin(), result.end(), '_', '/');
        return result;
    }

    case ListFormat::Geneva: {
        size_t ofs = (key.find("env_") == 0) ? 4 : 0;
        std::string result("_ext_");
        result += std::string(key.c_str() + ofs);
        std::replace(result.begin(), result.end(), '_', '/');
        return result;
    }

    default:
        break;
    }
    return key;
}
```

## ETW (UTC) Event property names

This example is applicable to UTC flow.

### Input event map for ETW

```cpp
// Convert from UTC key-value property list to JSON
cout << "**** UTC test:" << endl;
map<string, string> m =
{
    { "PartA_ext_device_id", "c:123456" },
    { "PartA_ext_metadata_fooBar", "one" },
    { "PartA_ext_metadata_fooBaz", "two" }
};
auto j = to_json(m, ListFormat::UTC);
cout << j.dump(2) << endl;
```

C++ code for each key flattening:

```cpp
size_t ofs = (key.find("PartA_") == 0) ? 5 : 0;
std::string result(key.c_str() + ofs);
std::replace(result.begin(), result.end(), '_', '/');
return result;
```

Output: after applying keys as JSON Patch

```json
{
  "ext": {
    "device": {
      "id": "c:123456"
    },
    "metadata": {
      "fooBar": "one",
      "fooBaz": "two"
    }
  }
}
```

## Geneva Event property names

This example is applicable to Geneva Agent flow.

### Input event map for Geneva

```cpp
// Convert from Geneva key-value property list to JSON
cout << "**** Geneva test:" << endl;
map<string, string> m =
{
    { "env_device_id", "c:123456" },
    { "env_metadata_fooBar", "one" },
    { "env_metadata_fooBaz", "two" }
};
auto j = to_json(m, ListFormat::Geneva);
cout << j.dump(2) << endl;
```

C++ code for each key flattening:

```cpp
size_t ofs = (key.find("env_") == 0) ? 4 : 0;
std::string result("_ext_");
result += std::string(key.c_str() + ofs);
std::replace(result.begin(), result.end(), '_', '/');
return result;
```

Output:

```json
{
  "ext": {
    "device": {
      "id": "c:123456"
    },
    "metadata": {
      "fooBar": "one",
      "fooBaz": "two"
    }
  }
}
```

## JSONPath Event property names

This example shows how JSONPath 'flat' property names can be converted to JSON object.

### Input event map for JSONPath

```cpp
// Convert from JSONPath key-value property list to JSON
cout << "**** JSONPath test:" << endl;
map<string, string> m =
{
    { "ext.device.id", "c:123456" },
    { "ext.metadata.fooBar", "one" },
    { "ext.metadata.fooBaz", "two" }
};
auto j = to_json(m, ListFormat::JSONPath);
cout << j.dump(2) << endl;
```

C++ code for each key flattening:

```cpp
std::string result("/");
result += key;
std::replace(result.begin(), result.end(), '.', '/');
return result;
```

Output:

```json
{
  "ext": {
    "device": {
      "id": "c:123456"
    },
    "metadata": {
      "fooBar": "one",
      "fooBaz": "two"
    }
  }
}
```

## Complete code example

```cpp
#include <iostream>
#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <algorithm>

using json = nlohmann::json;
using namespace std;


enum class ListFormat
{
    JSONPath = 0,
    XPath    = 1,
    UTC      = 2,
    Geneva   = 3
};

std::string to_json_patch(const std::string& key, ListFormat fmt = ListFormat::JSONPath)
{
    switch (fmt)
    {
        case ListFormat::JSONPath: {
            std::string result("/");
            result += key;
            std::replace(result.begin(), result.end(), '.', '/');
            return result;
        }
        case ListFormat::UTC: {
            size_t ofs = (key.find("PartA_") == 0) ? 5 : 0;
            std::string result(key.c_str() + ofs);
            std::replace(result.begin(), result.end(), '_', '/');
            return result;
        }
        case ListFormat::Geneva: {
            size_t ofs = (key.find("env_") == 0) ? 4 : 0;
            std::string result("_ext_");
            result += std::string(key.c_str() + ofs);
            std::replace(result.begin(), result.end(), '_', '/');
            return result;
        }
        case ListFormat::XPath:
            // TODO: XPath is slightly different than JSON pointer
            // with respect to array handling
        default:
            break;
    }
    return key;
}

map<string, string> to_flat_map(json & j)
{
    map<string, string> m;
    auto fj = j.flatten(); // https://tools.ietf.org/html/rfc6901
    for (json::iterator it = fj.begin(); it != fj.end(); ++it)
    {
        auto v = it.value();
        if (!v.empty())
        {
            m[it.key()] = v.dump();
        }
    }
    return m;
}

json to_json(map<string, string> m, ListFormat fmt = ListFormat::JSONPath)
{
    json result;
    for (auto &kv : m)
    {
        result[to_json_patch(kv.first, fmt)]=kv.second;
    }
    result = result.unflatten();
    return result;
}

void print_map(const map<string, string>& m)
{
    for(const auto &kv : m)
    {
        cout << kv.first << "=" << kv.second << endl;
    }
}

int main(int argc, char **argv) {

    {
        // Convert from JSON text to flat list
        cout << "**** JSON text to flat map test:" << endl;
        const char *test_json = R"JSON(
{
    "device": {
      "id": "c:123456"
    },
    "metadata": {
      "fooBar": "one",
      "fooBaz": "two"
    },
    "arrayOfStructs": [
      { "key1": "value1" },
      { "key2": "value2" },
      { "key3": "value3" }
    ],
    "foo": ["bar", "baz"],
    "": 0,
    "a/b": 1,
    "c%d": 2,
    "e^f": 3,
    "g|h": 4,
    "i\\j": 5,
    "k\"l": 6,
    " ": 7,
    "m~n": 8
}
)JSON";
        json j2 = json::parse(test_json);
        auto m = to_flat_map(j2);
        print_map(m);
    }

    {
        // Convert from JSON object to flat list
        cout << "**** JSON object to flat map test:" << endl;
        json j = {
            { "hello", "world" },
            {
                "ext",
                {
                        { "device", {} },
                        { "os", {} },
                        { "app", {} },
                }
            }
        };
        auto m = to_flat_map(j);
        print_map(m);
    }

    {
        // Convert from UTC key-value property list to JSON
        cout << "**** UTC test:" << endl;
        map<string, string> m =
        {
            { "PartA_ext_device_id", "c:123456" },
            { "PartA_ext_metadata_fooBar", "one" },
            { "PartA_ext_metadata_fooBaz", "two" }
        };
        auto j = to_json(m, ListFormat::UTC);
        cout << j.dump(2) << endl;
    };

    {
        // Convert from JSONPath key-value property list to JSON
        cout << "**** JSONPath test:" << endl;
        map<string, string> m =
        {
            { "ext.device.id", "c:123456" },
            { "ext.metadata.fooBar", "one" },
            { "ext.metadata.fooBaz", "two" }
        };
        auto j = to_json(m, ListFormat::JSONPath);
        cout << j.dump(2) << endl;
    };

    {
        // Convert from Geneva key-value property list to JSON
        cout << "**** Geneva test:" << endl;
        map<string, string> m =
        {
            { "env_device_id", "c:123456" },
            { "env_metadata_fooBar", "one" },
            { "env_metadata_fooBaz", "two" }
        };
        auto j = to_json(m, ListFormat::Geneva);
        cout << j.dump(2) << endl;
    };

    return 0;
}

```
