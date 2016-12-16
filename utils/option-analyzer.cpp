#include <json/reader.h>
#include <fstream>
#include <iostream>
#include <string>
#include <assert.h>

#include <pugixml.hpp>

using namespace Json;
using namespace std;
using namespace pugi;

int main(int argc, char **argv) {
  Reader reader;
  Value root;
  ifstream stream(argv[1]);
  reader.parse(stream, root);

  assert(root.isObject());

  Value::Members keys = root.getMemberNames();
  for (Value::Members::iterator i = keys.begin(); i != keys.end(); ++i) {
    //cout << *i << "\n";
    Value node = root[*i];
    assert(node.isObject());
    Value::Members subKeys = node.getMemberNames();
    for (Value::Members::iterator i2 = subKeys.begin(); i2 != subKeys.end(); ++i2) {
      string key = *i2;
      if (key == "default") {
        Value def = node[key];
        ValueType type = def.type();
        switch (type) {
        case nullValue:    // 0
        case intValue:     // 1
        case stringValue:  // 4
        case booleanValue: // 5
        case arrayValue:   // 6
        case objectValue:  // 7
          break;
        default:
          cout << type << "\n";
        }
      } else if (key == "type") {
        Value type = node[key];
        assert(type.type() == stringValue);
      } else if (key == "description") {
        Value description = node[key];
        assert(description.type() == stringValue);
        xml_document doc;
        string document = string("<document>\n") + description.asString() + string("</document>");
        xml_parse_result result = doc.load_string(document.c_str());
        if (!result) {
          cout << *i << ": " << result.description() << "\n";
          cout << document << "\n";
        }
        if (document.length() == 1642) {
          cout << *i << "\n";
          cout << document << "\n";
        }
      } else if (key == "readOnly") {
        Value readOnly = node[key];
        assert(readOnly.type() == booleanValue);
      } else if (key == "example") {
        Value example = node[key];
        switch (example.type()) {
        case intValue:     // 1
        case stringValue:  // 4
        case booleanValue: // 5
        case arrayValue:   // 6
        case objectValue:  // 7
          break;
        default:
          cout << *i << example.type() << "\n";
        }
      } else if (key == "declarations") {
      } else {
        cout << key << "\n";
      }
    }
  }
}