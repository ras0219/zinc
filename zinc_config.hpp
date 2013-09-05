#ifndef _ZINC_CONFIG_HPP_
#define _ZINC_CONFIG_HPP_

#include <string>

class JSONValue;

namespace zinc {

  struct Configuration {
    Configuration() : config{nullptr} { }
    explicit Configuration(const char* filename) {
      parse_file(filename);
    }
    Configuration(JSONValue* jv) : config(jv) { }

    void parse_file(const char* filename);
    JSONValue* get_path(const std::wstring& path);

    template<class T>
    T get(const std::wstring& path, const T& def);

    std::string get_string_config(const std::wstring& path, const std::string& def = "");
    int get_int_config(const std::wstring& path, int def = 0);
    double get_double_config(const std::wstring& path, double def = 0);
    bool get_bool_config(const std::wstring& path, bool def = false);

    JSONValue *config;
  };

  std::string get_locale_string(const std::wstring& s);

}

#endif
