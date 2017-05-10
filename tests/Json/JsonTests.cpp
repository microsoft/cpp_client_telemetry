#include "gtest/gtest.h"
#include "json/json.h"
#include <locale.h>
#include <limits>

using namespace JSON_NAMESPACE;

const IntType max_int =  std::numeric_limits<IntType>::max();

TEST(json_composer, empty_test) {

	Variant v;
    EXPECT_TRUE(v.empty());
	EXPECT_EQ("", v.serialize());
	
}

TEST(json_composer, escape_sequences_test) {

	json::ObjectType dictTypeBuf;

	dictTypeBuf["string_key_quotation"] = "\"";
	dictTypeBuf["string_key_slash"] = "\\";
	dictTypeBuf["string_key_backspace"] = "\b";
	dictTypeBuf["string_key_form_feed"] = "\f";
	dictTypeBuf["string_key_newline"] = "\n";
	dictTypeBuf["string_key_carriage_return"] = "\r";
	dictTypeBuf["string_key_tab"] = "\t";
	dictTypeBuf["string_key_space"] = " ";
	dictTypeBuf["string_key_empty"] = "";
	
	Variant v = Variant(dictTypeBuf);

	std::string outputString = v.serialize();

	Variant v2;
	EXPECT_TRUE(parse(outputString, v2));
	
}
	
TEST(json_composer, basic_types_test) {

	json::ArrayType arrayTypeBuf;
	json::ObjectType dictTypeBuf;

	dictTypeBuf["string_key"] = json::StringType("string_value");
	dictTypeBuf["string_key_type_casted"] = "string_value";

	const IntType maxInt =  std::numeric_limits<IntType>::max();
	dictTypeBuf["int_key"] = json::IntType(1);
	dictTypeBuf["int_key_max"] = json::IntType(maxInt);
	dictTypeBuf["int_key_max_minus"] = json::IntType(-maxInt);
	dictTypeBuf["int_key_zero"] = json::IntType(0);
	
	dictTypeBuf["float"] = json::FloatType(1.2);
	dictTypeBuf["bool_true"] = json::BoolType(true);
	dictTypeBuf["bool_false"] = json::BoolType(false);

	dictTypeBuf["null_key"] = json::null;
	dictTypeBuf["null_key2"] = null;

	std::vector<Variant> vectorBuf;

	for(int i = 0; i < 10; i++) {
		vectorBuf.push_back(json::IntType((i+1)*(i+1)));
	}

	arrayTypeBuf = vectorBuf;

	dictTypeBuf["array_key_ints"] = arrayTypeBuf;
	dictTypeBuf["array_key_strings"] = arrayTypeBuf;

	//dictTypeBuf["dict_key"] = dictTypeBuf;

	Variant v = Variant(dictTypeBuf);

	std::string outputString = v.serialize();

	Variant v2;
	EXPECT_TRUE(parse(outputString, v2));

}

TEST(json_parser, plain_json_test) {

	Variant v;

	std::string s =  	"{\"id\":1,\"jsonrpc\":2.0123,\"total\":1,\"result\":[{\"bool_false\":false,\"bool_true\":true,\"integer\":1123213,\"integer_max_9223372036854775807\":9223372036854775807,\"integer_max_9223372036854775807+1\":9223372036854775808,\"null\":null,\"float\":1231.23223,\"string\":\"string\",\"string_empty\":\"\",\"string_with_extra_space\":\" abc \",\"bool_true\":true,\"bool_false\":false,\"array_empty\":[],\"array_123\":[1,2,3],\"array_employees\":[{\"firstName\":\"John\",\"lastName\":\"Doe\"},{\"firstName\":\"Anna\",\"lastName\":\"Smith\"},{\"firstName\":\"Peter\",\"lastName\":\"Jones\"}],\"dict_empty\":{},\"dict_strings\":{\"a_key\":\"a_value\",\"b_key\":777},\"picture\":\"http://placehold.it/dasdadadasasdx32\",\"age\":694,\"name\":\"MadisonOgden\",\"gender\":\"female\",\"company\":\"Multitiqua\",\"phone\":\"889-461-2315\",\"email\":\"madison@multitiqua.com\",\"address\":\"801933,Chicago,TudorCityPlace\",\"about\":\"Iustodolorequisfeugiatadin,minimvelzzriliriureex,dolorvelitloremin.Dolor,ipsumpraesenttelaoreetnibh,nullaelitminimeuismodduis,loremiriuread.Dolorut,dolordelenitillumdoloriusto,facilisiselithendreritsuscipittation,aliquamvelit.Expraesenttincidunt,feugiatadipiscingnullaullamcorperzzril,nibhettationfeugiatad,consectetuer.Tationullamcorperdolorex,veroduisdiaminaugue,esseilluminduisveniam,facilisifacilisisuteuismodtation,volutpatiustodolorealiquam.\",\"registered\":\"2005-02-09T08:33:02-02:00\",\"tags\":[\"dolore\",\"enim\",\"euismod\",\"iriure\",\"adipiscing\",\"dolore\",\"dolor\"],\"friends\":[{\"id\":1,\"name\":\"MorganNelson\"},{\"id\":2,\"name\":\"SerenityNeal\"},{\"id\":3,\"name\":\"ZoeOldridge\"}]},{\"id\":2,\"null\":null,\"guid\":\"6dad49b4-0810-4b3b-b363-12220656c71e\",\"picture\":\"http://placehold.it/dasdadadasasdx32\",\"age\":695,\"name\":\"MadisonOgden\",\"gender\":\"female\",\"company\":\"Multitiqua\",\"phone\":\"889-461-2315\",\"email\":\"madison@multitiqua.com\",\"address\":\"801933,Chicago,TudorCityPlace\",\"about\":\"Iustodolorequisfeugiatadin,minimvelzzriliriureex,dolorvelitloremin.Dolor,ipsumpraesenttelaoreetnibh,nullaelitminimeuismodduis,loremiriuread.Dolorut,dolordelenitillumdoloriusto,facilisiselithendreritsuscipittation,aliquamvelit.Expraesenttincidunt,feugiatadipiscingnullaullamcorperzzril,nibhettationfeugiatad,consectetuer.Tationullamcorperdolorex,veroduisdiaminaugue,esseilluminduisveniam,facilisifacilisisuteuismodtation,volutpatiustodolorealiquam.\",\"registered\":\"2005-02-09T08:33:02-02:00\",\"tags\":[\"dolore\",\"enim\",\"euismod\",\"iriure\",\"adipiscing\",\"dolore\",\"dolor\"],\"friends\":[{\"id\":1,\"name\":\"MorganNelson\"},{\"id\":2,\"name\":\"SerenityNeal\"},{\"id\":3,\"name\":\"ZoeOldridge\"}]}]}";
	
	EXPECT_TRUE(parse(s, v));

	Variant v3 = v.descend("result");


	json::ArrayType arrayTypeBuf;
	json::ArrayType arrayTypeBuf2;
	json::StringType strTypeBuf;
	json::FloatType floatTypeBuf;
	json::IntType intTypeBuf;
	json::BoolType boolTypeBuf;
	json::ObjectType dictTypeBuf;

	v3.get(arrayTypeBuf);

	EXPECT_EQ(2, arrayTypeBuf.size());

	EXPECT_FALSE(arrayTypeBuf[0].descend("bool_true").get(arrayTypeBuf));
	EXPECT_FALSE(arrayTypeBuf[0].descend("bool_true").get(strTypeBuf));
	EXPECT_FALSE(arrayTypeBuf[0].descend("bool_true").get(floatTypeBuf));
	EXPECT_FALSE(arrayTypeBuf[0].descend("bool_true").get(intTypeBuf));
	EXPECT_FALSE(arrayTypeBuf[0].descend("bool_true").get(dictTypeBuf));
	EXPECT_FALSE(arrayTypeBuf[0].descend("integer").get(boolTypeBuf));
	
	EXPECT_TRUE(arrayTypeBuf[0].descend("bool_true").get(boolTypeBuf));
	EXPECT_EQ(true, boolTypeBuf);
	
	EXPECT_TRUE(arrayTypeBuf[0].descend("bool_false").get(boolTypeBuf));
	EXPECT_EQ(false, boolTypeBuf);
	
	EXPECT_TRUE(arrayTypeBuf[0].descend("integer").get(intTypeBuf));
	EXPECT_EQ(1123213, intTypeBuf);
	
	EXPECT_TRUE(arrayTypeBuf[0].descend("integer").get(floatTypeBuf));
	EXPECT_EQ(1123213, floatTypeBuf);	

	EXPECT_TRUE(arrayTypeBuf[0].descend("integer_max_9223372036854775807").get(intTypeBuf));
	EXPECT_EQ(max_int,intTypeBuf);

	EXPECT_TRUE(arrayTypeBuf[0].descend("float").get(floatTypeBuf));
	EXPECT_EQ(1231.23223, floatTypeBuf);
	
	EXPECT_TRUE(arrayTypeBuf[0].descend("float").get(intTypeBuf));
	EXPECT_EQ(1231, intTypeBuf);

	EXPECT_TRUE(arrayTypeBuf[0].descend("string").get(strTypeBuf));
	EXPECT_EQ("string", strTypeBuf);

	EXPECT_TRUE(arrayTypeBuf[0].descend("string_empty").get(strTypeBuf));
	EXPECT_EQ("", strTypeBuf);

	EXPECT_TRUE(arrayTypeBuf[0].descend("string_with_extra_space").get(strTypeBuf));
	EXPECT_EQ(" abc ", strTypeBuf);

	EXPECT_TRUE(arrayTypeBuf[0].descend("null").get(json::null));

	EXPECT_TRUE(arrayTypeBuf[0].descend("array_empty").get(arrayTypeBuf2));
	EXPECT_EQ(0, arrayTypeBuf2.size());

	EXPECT_TRUE(arrayTypeBuf[0].descend("array_123").get(arrayTypeBuf2));
	EXPECT_EQ(3, arrayTypeBuf2.size());

	EXPECT_TRUE(arrayTypeBuf[0].descend("array_employees").get(arrayTypeBuf2));
	EXPECT_EQ(3, arrayTypeBuf2.size());

	EXPECT_TRUE(arrayTypeBuf[0].descend("dict_empty").get(dictTypeBuf));
	EXPECT_TRUE(dictTypeBuf.empty());

	EXPECT_TRUE(arrayTypeBuf[0].descend("dict_strings").get(dictTypeBuf));
	EXPECT_EQ(2, dictTypeBuf.size());

	dictTypeBuf.find("a_key")->second.get(strTypeBuf);
	EXPECT_EQ("a_value", strTypeBuf);

	dictTypeBuf.find("b_key")->second.get(intTypeBuf);
	EXPECT_EQ(777, intTypeBuf);
}

TEST(json_parser, aray_shortcut_test) {
    Variant v;

    std::string s = R"( { "array" : [{ "string":"content" }, {}] } )";

    EXPECT_TRUE(parse(s, v));

    EXPECT_EQ(0, v.getArraySize() ); //not an array
    EXPECT_EQ(2, v.descend("array").getArraySize() ); // array
    EXPECT_EQ(0, Variant().getArraySize()); //empty object

    EXPECT_FALSE(v.descend("array")[0].empty());
    EXPECT_FALSE(v.descend("array")[1].empty());
    EXPECT_TRUE(v.descend("array")[2].empty()); // out of scope
    EXPECT_TRUE(v[0].empty()); // not an array
    EXPECT_TRUE(Variant()[0].empty()); // empty object

    json::StringType strTypeBuf;
    EXPECT_TRUE(v.descend("array")[0].descend("string").get(strTypeBuf)); // getting nested value
    EXPECT_EQ("content", strTypeBuf);
}

/* TEST(json_parser, locale_test1) {
	std::string localeName = setlocale(LC_ALL, NULL);

	Variant v;

	std::string s =  	"{\"id\":1,\"jsonrpc\":2.0123,\"total\":1,\"result\":[{\"integer\":1123213,\"integer_max_9223372036854775807\":9223372036854775807,\"integer_max_9223372036854775807+1\":9223372036854775808,\"null\":null,\"float\":1231.23223,\"string\":\"string\",\"string_empty\":\"\",\"string_with_extra_space\":\" abc \",\"bool_true\":true,\"bool_false\":false,\"array_empty\":[],\"array_123\":[1,2,3],\"array_employees\":[{\"firstName\":\"John\",\"lastName\":\"Doe\"},{\"firstName\":\"Anna\",\"lastName\":\"Smith\"},{\"firstName\":\"Peter\",\"lastName\":\"Jones\"}],\"dict_empty\":{},\"dict_strings\":{\"a_key\":\"a_value\",\"b_key\":777},\"picture\":\"http://placehold.it/dasdadadasasdx32\",\"age\":694,\"name\":\"MadisonOgden\",\"gender\":\"female\",\"company\":\"Multitiqua\",\"phone\":\"889-461-2315\",\"email\":\"madison@multitiqua.com\",\"address\":\"801933,Chicago,TudorCityPlace\",\"about\":\"Iustodolorequisfeugiatadin,minimvelzzriliriureex,dolorvelitloremin.Dolor,ipsumpraesenttelaoreetnibh,nullaelitminimeuismodduis,loremiriuread.Dolorut,dolordelenitillumdoloriusto,facilisiselithendreritsuscipittation,aliquamvelit.Expraesenttincidunt,feugiatadipiscingnullaullamcorperzzril,nibhettationfeugiatad,consectetuer.Tationullamcorperdolorex,veroduisdiaminaugue,esseilluminduisveniam,facilisifacilisisuteuismodtation,volutpatiustodolorealiquam.\",\"registered\":\"2005-02-09T08:33:02-02:00\",\"tags\":[\"dolore\",\"enim\",\"euismod\",\"iriure\",\"adipiscing\",\"dolore\",\"dolor\"],\"friends\":[{\"id\":1,\"name\":\"MorganNelson\"},{\"id\":2,\"name\":\"SerenityNeal\"},{\"id\":3,\"name\":\"ZoeOldridge\"}]},{\"id\":2,\"null\":null,\"guid\":\"6dad49b4-0810-4b3b-b363-12220656c71e\",\"picture\":\"http://placehold.it/dasdadadasasdx32\",\"age\":695,\"name\":\"MadisonOgden\",\"gender\":\"female\",\"company\":\"Multitiqua\",\"phone\":\"889-461-2315\",\"email\":\"madison@multitiqua.com\",\"address\":\"801933,Chicago,TudorCityPlace\",\"about\":\"Iustodolorequisfeugiatadin,minimvelzzriliriureex,dolorvelitloremin.Dolor,ipsumpraesenttelaoreetnibh,nullaelitminimeuismodduis,loremiriuread.Dolorut,dolordelenitillumdoloriusto,facilisiselithendreritsuscipittation,aliquamvelit.Expraesenttincidunt,feugiatadipiscingnullaullamcorperzzril,nibhettationfeugiatad,consectetuer.Tationullamcorperdolorex,veroduisdiaminaugue,esseilluminduisveniam,facilisifacilisisuteuismodtation,volutpatiustodolorealiquam.\",\"registered\":\"2005-02-09T08:33:02-02:00\",\"tags\":[\"dolore\",\"enim\",\"euismod\",\"iriure\",\"adipiscing\",\"dolore\",\"dolor\"],\"friends\":[{\"id\":1,\"name\":\"MorganNelson\"},{\"id\":2,\"name\":\"SerenityNeal\"},{\"id\":3,\"name\":\"ZoeOldridge\"}]}]}";

	bool r = parse(s, v);
	std::string s2 = v.serialize();

	Variant v2 = v.descend("jsonrpc");

	json::FloatType floatVariable;

	v2.get(floatVariable);

	std::ostringstream ss;
	ss << floatVariable;

	localeName = setlocale(LC_ALL, "");
	EXPECT_EQ(true, r);

} */

TEST(json_parser, pass1_json_test) {

	Variant v;

	std::string s =  "[\"JSONTestPatternpass1\",{\"objectwith1member\":[\"arraywith1element\"]},{},[],-42,true,false,null,{\"integer\":1234567890,\"real\":-9876.543210,\"e\":0.123456789e-12,\"E\":1.234567890E+34,\"\":23456789012E66,\"zero\":0,\"one\":1,\"space\":\"\",\"quote\":\"\\\"\",\"backslash\":\"\\\\\",\"controls\":\"\\b\\f\\n\\r\\t\",\"slash\":\"/&\\/\",\"alpha\":\"abcdefghijklmnopqrstuvwyz\",\"ALPHA\":\"ABCDEFGHIJKLMNOPQRSTUVWYZ\",\"digit\":\"0123456789\",\"0123456789\":\"digit\",\"special\":\"`1~!@#$%^&*()_+-={':[,]}|;.</>\?\",\"hex\":\"\\u0123\\u4567\\u89AB\\uCDEF\\uabcd\\uef4A\",\"true\":true,\"false\":false,\"null\":null,\"array\":[],\"object\":{},\"address\":\"50St.JamesStreet\",\"url\":\"http://www.JSON.org/\",\"comment\":\"///*<!----\",\"#---->*/\":\"\",\"spaced\":[1,2,3,4,5,6,7],\"compact\":[1,2,3,4,5,6,7],\"jsontext\":\"{\\\"objectwith1member\\\":[\\\"arraywith1element\\\"]}\",\"quotes\":\"&#34;\\u0022%220x22034&#x22;\",\"\\/\\\\\\\"\\uCAFE\\uBABE\\uAB98\\uFCDE\\ubcda\\uef4A\\b\\f\\n\\r\\t`1~!@#$%^&*()_+-=[]{}|;:',./<>\?\":\"Akeycanbeanystring\"},0.5,98.6,99.44,1066,1e1,0.1e1,1e-1,1e00,2e+00,2e-00,\"rosebud\"]";

	EXPECT_EQ(true, parse(s, v));

}

TEST(json_parser, pass2_json_test) {

	Variant v;

	std::string s =  "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\"Not too deep\"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";

	EXPECT_EQ(true, parse(s, v));

}

TEST(json_parser, pass3_json_test) {

	Variant v;

	std::string s =  "{\"JSONTestPatternpass3\":{\"Theoutermostvalue\":\"mustbeanobjectorarray.\",\"Inthistest\":\"Itisanobject.\"}}";

    EXPECT_EQ(true, parse(s, v));

}

TEST(json_parser, fail1_json_test) {

	Variant v;

	std::string s = "\"A JSON payload should be an object or array, not a string.\"";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail2_json_test) {

	Variant v;

	std::string s = "[\"Unclosed array\"";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail3_json_test) {

	Variant v;

	std::string s = "{unquoted_key: \"keys must be quoted\"}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail4_json_test) {

	Variant v;

	std::string s = "[\"extra comma\",]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail5_json_test) {

	Variant v;

	std::string s = "[\"double extra comma\",,]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail6_json_test) {

	Variant v;

	std::string s = "[   , \"<-- missing value\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail7_json_test) {

	Variant v;

	std::string s = "[\"Comma after the close\"],";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail8_json_test) {

	Variant v;

	std::string s = "[\"Extra close\"]]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail9_json_test) {

	Variant v;

	std::string s = "{\"Extra comma\": true,}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail10_json_test) {

	Variant v;

	std::string s = "{\"Extra value after close\": true} \"misplaced quoted value\"";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail11_json_test) {

	Variant v;

	std::string s = "{\"Illegal expression\": 1 + 2}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail12_json_test) {

	Variant v;

	std::string s = "{\"Illegal invocation\": alert()}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail13_json_test) {

	Variant v;

	std::string s = "{\"Numbers cannot have leading zeroes\": 013}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail14_json_test) {

	Variant v;

	std::string s = "{\"Numbers cannot be hex\": 0x14}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail15_json_test) {

	Variant v;

	std::string s = "[\"Illegal backslash escape: \\x15\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail16_json_test) {

	Variant v;

	std::string s = "[\\naked]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail17_json_test) {

	Variant v;

	std::string s = "[\"Illegal backslash escape: \\017\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail18_json_test) {

	Variant v;

	std::string s = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail19_json_test) {

	Variant v;

	std::string s = "{\"Missing colon\" null}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail20_json_test) {

	Variant v;

	std::string s = "{\"Double colon\":: null}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail21_json_test) {

	Variant v;

	std::string s = "{\"Comma instead of colon\", null}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail22_json_test) {

	Variant v;

	std::string s = "[\"Colon instead of comma\": false]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail23_json_test) {

	Variant v;

	std::string s = "[\"Bad value\", truth]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail24_json_test) {

	Variant v;

	std::string s = "[\'single quote\']";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail25_json_test) {

	Variant v;

	std::string s = "[\"\ttab\tcharacter\tin\tstring\t\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail26_json_test) {

	Variant v;

	std::string s = "[\"tab\\\tcharacter\\\tin\\\tstring\\\t\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail27_json_test) {

	Variant v;

	std::string s = "[\"line\nbreak\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail28_json_test) {

	Variant v;

	std::string s = "[\"line\\\nbreak\"]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail29_json_test) {

	Variant v;

	std::string s = "[0e]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail30_json_test) {

	Variant v;

	std::string s = "[0e+]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail31_json_test) {

	Variant v;

	std::string s = "[0e+-1]";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail32_json_test) {

	Variant v;

	std::string s = "{\"Comma instead if closing brace\": true,";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail33_json_test) {

	Variant v;

	std::string s = "[\"mismatch\"}";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, fail34_json_test) {

	Variant v;

	std::string s = "string";
	
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, incorrect_json_test) {

	Variant v;

	std::string s = "{\"name\": ,\"test_httpstack_http_GET_200_ok\"}";
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, incorrect_json_test2) {

	Variant v;

	std::string s = "{\"a_key\":\"a_value\", 1:\"1_value\"}";
	EXPECT_FALSE(parse(s, v));

}

TEST(json_parser, float_whitespace) {
    Variant v;
    const std::string s = "{\"pi\": 3.14\n}";
    EXPECT_TRUE(parse(s, v));
    FloatType pi;
    EXPECT_TRUE(v.descend("pi").get(pi));
	ASSERT_FLOAT_EQ(3.14, pi);
}
