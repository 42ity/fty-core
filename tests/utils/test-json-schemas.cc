#include "catch.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <regex>
#include <cstdlib>
#include <fstream>

#include "json_schemas.h"

using namespace utils::json;

/*!
\brief Counts number of regular files in a given directory, that
satisfy a given regular expression mask

\param[in] directory path to a given directory
\param[in] regular expression mask
/return If successfull, returns number of files. If unsuccessfull. returns
-1 when parameter 'directory' is NULL, -2 when there was an error opening
the given directory
\author Karol Hrdina <karolhrdina@eaton.com>
\todo Check the return value of closedir() call. Return a >-2 value?
*/
int count_files_directory(const char* directory, std::regex *mask) {
    if (directory == NULL) {
		return -1;
	}	
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    
    dir = opendir (directory);
	if (dir == NULL) {
        return -2;
	}
	unsigned int counter = 0;
	while (entry = readdir (dir)) {
		if (mask != NULL) {	// count only files that satisfy given mask
			if (entry->d_type == DT_REG &&
				std::regex_match(entry->d_name, *mask)) {                
    				++counter;
			}
		} else if (entry->d_type == DT_REG) {	
				++counter;
		}
		
    }
    closedir(dir); // TODO Check return value
	return counter;
}

TEST_CASE("Generated enumeration of message types",
          "[json][validator][schema][messages]") {    
    utils::json::MessageTypesEnum
	enumeration = utils::json::MessageTypesEnum::UNKNOWN;	
	REQUIRE(enumeration == utils::json::MessageTypesEnum::UNKNOWN);

	SECTION("Check generated enumeration size") {
		int size = static_cast<int>(utils::json::MessageTypesEnum::UNKNOWN);
		std::string tmp(".+Message\\.json");
        std::regex regex(tmp);
#ifdef TEST_JSON_SCHEMAS_BINARY_PATH
		tmp.assign(TEST_JSON_SCHEMAS_BINARY_PATH).append("/tests/utils/messages/json_schema");
#else
		FAIL(
"Macro 'TEST_JSON_SCHEMAS_BINARY_PATH' not defined. Test needs this macro for \
counting number of test .json files in $(top_srcdir)/test/utils/message/json_schema, \
thus requiring an abosulte path relative to where is the test being ran from.");
#endif
		int return_value = count_files_directory(tmp.c_str(), &regex);
		REQUIRE( return_value >= 0 );
		REQUIRE( return_value == size );
    }

    SECTION("Generated codetable() and enumtable() functions") {
        int size = static_cast<int>(utils::json::MessageTypesEnum::UNKNOWN);
       for (int i = 0; i <= size; ++i) {
            utils::json::MessageTypesEnum e =
                utils::json::codetable (i);
            unsigned int r = utils::json::enumtable (e);
            REQUIRE (r == i);  
        }
   }
}

TEST_CASE("Generated get_map() function",
          "[json][validator][schema][messages]") {
    
    SECTION("get_map() must return a libvariant::Variant object of type \
VariantDefined::MapType for each generated message type") {        
		int size = static_cast<int>(utils::json::MessageTypesEnum::UNKNOWN);
        unsigned int i = 0, ssize = utils::json::get_map().size();
		for(i = 0; i < ssize; ++i) {
			// never do const Variant& instead of VariantRef in productioin code
			// unless you know what you are doing!
			// OR feel the wrath of a certain unspecified team member ;)
			const libvariant::Variant& variant = utils::json::get_map().at(i);
			if (variant.GetType() != libvariant::VariantDefines::MapType) {
                FAIL("A libvariant::Variant object (representing root of one \
particular json schema) obtained from utils::json::get_map() must always be \
of json object type");
            }
		}
	}

    SECTION("each libvariant::Variant::MapType object returned by get_map() must have \
a \"schema\" property (== key)") {
		int size = static_cast<int>(utils::json::MessageTypesEnum::UNKNOWN);
        unsigned int i = 0, ssize = utils::json::get_map().size();
		for(i = 0; i < ssize; ++i) {
			const libvariant::Variant& variant = utils::json::get_map().at(i);
			if (variant.GetType() != libvariant::VariantDefines::MapType && 
				variant.Contains("schema") == false) {                                        
                    FAIL("A libvariant::Variant object (representing root of \
one particular json schema) obtained from utils::json::get_map() must always \
have a \"schema\" property / key defined.");
            }
		}
    }

//    SECTION("")
}

TEST_CASE(
"Testing the validator itself"
"[json][validator][schema][messages]") {

    libvariant::Variant v;
    ValidateResultEnum vr;

    SECTION("NaturalNumMessage") {
        // Valid
        const char integer_message_v1[] = "\
{\
  \"natural\" : 1\
}";
    
        v = libvariant::DeserializeJSON(integer_message_v1);
        v["schema"] = enumtable(MessageTypesEnum::NaturalNum);
        std::string json = libvariant::SerializeJSON(v);

        vr = validate(json.c_str(), MessageTypesEnum::NaturalNum);
        REQUIRE (vr == ValidateResultEnum::Valid);

        // Invalid
        // bad key name (capital letter)
        const char integer_message_i1[] = "\
{\
  \"Natural\" : 10\
}";
        v = libvariant::DeserializeJSON(integer_message_i1);
        v["schema"] = enumtable(MessageTypesEnum::NaturalNum);
        json = libvariant::SerializeJSON(v); 

        vr = validate(json.c_str(), MessageTypesEnum::NaturalNum);
        REQUIRE (vr == ValidateResultEnum::Invalid);
 
        // Invalid
        // bad value     
        const char integer_message_i2[] = "\
{\
  \"natural\" : -1\
}";
        v = libvariant::DeserializeJSON(integer_message_i2);
        v["schema"] = enumtable(MessageTypesEnum::NaturalNum);
        json = libvariant::SerializeJSON(v); 

        vr = validate(json.c_str(), MessageTypesEnum::NaturalNum);
        REQUIRE (vr == ValidateResultEnum::Invalid);

        // Invalid
        // missing schema
        const char integer_message_i3[] = "\
{\
  \"natural\" : 1\
}";
        vr = validate(integer_message_i3, MessageTypesEnum::NaturalNum);
        REQUIRE (vr == ValidateResultEnum::Invalid);
    } // NaturalNumMessage SECTION

    SECTION("StringMessage") {
        // Valid
        const char string_message_v1[] = "\
{\
  \"string\" : \"abcd efgh\"\
}";
    
        v = libvariant::DeserializeJSON(string_message_v1);
        v["schema"] = enumtable(MessageTypesEnum::String);
        std::string json = libvariant::SerializeJSON(v);

        vr = validate(json.c_str(), MessageTypesEnum::String);
        REQUIRE (vr == ValidateResultEnum::Valid);

        // Invalid
        // bad key name (capital letter)
        const char string_message_i1[] = "\
{\
  \"strinG\" : 10\
}";
        v = libvariant::DeserializeJSON(string_message_i1);
        v["schema"] = enumtable(MessageTypesEnum::NaturalNum);
        json = libvariant::SerializeJSON(v); 

        vr = validate(json.c_str(), MessageTypesEnum::String);
        REQUIRE (vr == ValidateResultEnum::Invalid);
 
        // Invalid
        // bad value     
        const char string_message_i2[] = "\
{\
  \"string\" : 123\
}";
        v = libvariant::DeserializeJSON(string_message_i2);
        v["schema"] = enumtable(MessageTypesEnum::String);
        json = libvariant::SerializeJSON(v); 

        vr = validate(json.c_str(), MessageTypesEnum::String);
        REQUIRE (vr == ValidateResultEnum::Invalid);

        // Invalid
        // missing schema
        const char string_message_i3[] = "\
{\
  \"string\" : \"hello world!\"\
}";
        vr = validate(string_message_i3, MessageTypesEnum::String);
        REQUIRE (vr == ValidateResultEnum::Invalid);
    
        // Invalid
        // Additional properties defined
        const char string_message_i4[] = "\
{\
  \"string\" : \"hello world!\",\
  \"addProp\" : {\
     \"propName\" : \"propValue\" \
  }\
}";
        v = libvariant::DeserializeJSON(string_message_i4);
        v["schema"] = enumtable(MessageTypesEnum::String);
        json = libvariant::SerializeJSON(v); 

        vr = validate(json.c_str(), MessageTypesEnum::String);
        REQUIRE (vr == ValidateResultEnum::Invalid);
    } // StringMessage SECTION

    SECTION("IncompleteStringMessage") {        
        // Valid
        // Additional properties are allowed
        const char incompletestring_message_v1[] = "\
{\
  \"string\" : \"Hello World!\",\
  \"additional property\" : 1234\
}";
    
        v = libvariant::DeserializeJSON(incompletestring_message_v1);
        v["schema"] = enumtable(MessageTypesEnum::IncompleteString);
        std::string json = libvariant::SerializeJSON(v);

        vr = validate(json.c_str(), MessageTypesEnum::IncompleteString);
        REQUIRE (vr == ValidateResultEnum::Valid);

        // Valid
        // required properties were not set
        const char incompletestring_message_v2[] = "\
{\
  \"Hakuna Matata!\" : \"Hello World!\",\
  \"additional property\" : 1234,\
  \"xyzx\" : {\
    \"sdf sdf\" : 1234,\
    \"sdf sdfsdf sd\" : \"sdf sdfs df\"\
  }\
}";    
        vr = validate(incompletestring_message_v2, MessageTypesEnum::IncompleteString);
        REQUIRE (vr == ValidateResultEnum::Valid);

        // Invalid
        // However once you define the ones that are specified in a wrong way...
        const char incompletestring_message_i1[] = "\
{\
  \"string\" : \"Hello World!\",\
  \"additional property\" : 1234,\
  \"schema\" : -1\
}";
    
        vr = validate(incompletestring_message_i1, MessageTypesEnum::IncompleteString);
        REQUIRE (vr == ValidateResultEnum::Invalid);

    } // IncompleteStringMessage SECTION

    SECTION("NetworkList") {
        // Valid
        // Construction example
        libvariant::Variant root(libvariant::VariantDefines::MapType);
        libvariant::Variant array(libvariant::VariantDefines::ListType);
        root["rc-id"] = 42;

        libvariant::Variant item(libvariant::VariantDefines::MapType);
        // item 1
        item["type"] = "automatic";        
        item["name"] = "enps025";
        item["ipver"] = "ipv4";
        item["ipaddr"] = "130.38.2.0";
        item["prefixlen"] = 24;
        item["mac"] = "FC:15:B4:00:C3:B3";
        array.Append(item);

        // item 2
        item.Clear();
        item["type"] = "deleted";        
        item["ipver"] = "ipv4";
        item["ipaddr"] = "130.38.2.1";
        item["prefixlen"] = 8;
        array.Append(item);

        // item 3
        item.Clear();
        item["type"] = "manual";        
        item["ipver"] = "ipv4";
        item["ipaddr"] = "192.168.0.2";
        item["prefixlen"] = 24;
        array.Append(item);

        root["networks"] = array;
        root["schema"] = enumtable(MessageTypesEnum::NetworkList);
         
        std::string json = libvariant::SerializeJSON(root);
        INFO("json:\n");
        INFO(json.c_str());

        vr = validate(json.c_str(), MessageTypesEnum::NetworkList);
        REQUIRE (vr == ValidateResultEnum::Valid);
        libvariant::Variant back;
        vr = validate_parse(json.c_str(), MessageTypesEnum::NetworkList, back);
        REQUIRE (vr == ValidateResultEnum::Valid);
        REQUIRE (root.Compare(back) == 0 );


   
    } // NetworkList SECTION

}

// This test case needs to be modified when a new file is added into
// 'messages/json_schema' directory
TEST_CASE(
"TODO description",
"[json][validator][schema][messages]") {

/*
    THIS IS INSANE -- I DON'T WANT TO DO THIS 
	SECTION("Test individual messages") {
#ifdef TEST_JSON_SCHEMAS_BINARY_PATH
		std::string tmp(TEST_JSON_SCHEMAS_BINARY_PATH);
		//tmp.assign(TEST_JSON_SCHEMAS_BINARY_PATH).append("/tests/utils/messages/json_schema");
#else
		FAIL("Macro 'TEST_JSON_SCHEMAS_BINARY_PATH' not defined. \
Test needs this macro for counting number of test .json files in \
$(top_srcdir)/test/utils/message/json_schema, thus requiring an abosulte path \
relative to where is the test being ran from.");
#endif
		std::ifstream file(
            tmp.append("/tests/utils/messages/json_schema/Test1Message.json"),
            std::ifstream::binary);
		// FFFFFFfffffffffffuuuuu...i hate streams		
		if (file) {
			// get length of file:
            file.seekg(0, file.end);
            int length = file.tellg();
            file.seekg(0, file.beg);

			std::string str;
            str.resize(length, ' '); // reserve space
            char* begin = &*str.begin();
 
            file.read(begin, length);
            file.close();
            
            std::string tmp;
            tmp.append(
"s/(\\s*\"schema\"\\s*:\\s*\{\\s*\"type\"\\s*:\\s*\")string\"(\\s*,\\s*\"enum\"\\s*:\\s*\[\\s*)\"\\s*");
            tmp.append("Test1");
            tmp.append(
"\\s*\"(\\s*\]\\s*\})/\1number\", \"multipleOf\" : 1.0, \"minumum\" : 1\2");
            tmp.append(static_cast<int>(utils::json::MessageTypesEnum::Test));
            tmp.append("\3/");
            std::regex regex(tmp);

			INFO("orig: '" << utils::json::Test1Message().back() << "'\n");
			INFO("orig: '" << str.back() << "'\n");

			INFO("orig: " << utils::json::Test1Message().size() << "\n")
			INFO("read: " << str.size() << "'\n")
			INFO("orig:\n'" << utils::json::Test1Message() << "'\n")
			INFO("read:\n'" << str << "'\n")
			REQUIRE( str.compare(utils::json::Test1Message()) == 0); 
      
        } else {
            FAIL("Could not open Test1Message.json");
        }
	}
*/
	SECTION("") {}
}

/*
TEST_CASE(
"Check correct generation json schema messages",
"[json][validator][schema][message]") {
	utils::json::

	SECTION(
	"Individual message functions") {
	}

	SECTION(
	"Schemas are valid?") {
	}

}
*/
