#include <json/json.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <locale>

int main(int argc, char **argv) {
    using namespace std;
    using namespace json;

    locale oldLocale = locale::global(locale(""));

    ifstream f;
    istream &in = argc > 1 ? (f.open(argv[1]), f) : cin;
    ostringstream ss;

    Variant var;
    bool ok = in && ss << in.rdbuf() && in && parse(ss.str(), var);

    locale::global(oldLocale);
    return !ok;
}
