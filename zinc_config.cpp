#include "zinc_config.hpp"
#include "JSON.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace std;
using namespace zinc;

// from StackOverflow
// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
// Modified for unicode
wstring get_file_contents(const char *filename)
{
  wifstream in(filename, ios::in | ios::binary);
  if (in)
  {
    wstring contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

// Dummy
std::string get_locale_string(const std::string & s)
{
  return s;
}

// Real worker
std::string zinc::get_locale_string(const std::wstring & s)
{
  const wchar_t * cs = s.c_str();
  const size_t wn = std::wcsrtombs(NULL, &cs, 0, NULL);

  if (wn == size_t(-1))
  {
    std::cout << "Error in wcsrtombs(): " << errno << std::endl;
    return "";
  }

  std::vector<char> buf(wn + 1);
  const size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn + 1, NULL);

  if (wn_again == size_t(-1))
  {
    std::cout << "Error in wcsrtombs(): " << errno << std::endl;
    return "";
  }

  assert(cs == NULL); // successful conversion

  return std::string(buf.data(), wn);
}

void Configuration::parse_file(const char* filename) {
  wstring contents = get_file_contents(filename);
  config = JSON::Parse(contents.c_str());

  if (config == nullptr)
    throw runtime_error("Failed to parse config file");

  if (not config->IsObject())
    throw runtime_error("Config file must be a JSON object");
}

JSONValue* Configuration::get_path(const wstring& path) {
  JSONValue* obj = config;
  if (path.empty())
    return obj;
  vector<wstring> segments;

  auto p1 = path.begin();
  // Known bug: if path starts with '.', will report "cannot have two consecutive periods"
  for (auto p2 = path.begin(); p2 != path.end(); ++p2) {
    if (*p2 == L'.') {
      if (p1 == p2)
        throw runtime_error("Path invalid - cannot have two consecutive periods");
      segments.emplace_back(p1, p2);
      p1 = p2 + 1;
    }
  }
  if (p1 == path.end())
    throw runtime_error("Path invalid - cannot end in period");
  segments.emplace_back(p1, path.end());

  for (auto s : segments) {
    if (!obj->IsObject())
      // We need to dereference a path here; therefore fail if not object
      return nullptr;
    auto it = obj->AsObject().find(s);
    if (it == obj->AsObject().end())
      // Child does not exist
      return nullptr;
    obj = it->second;
  }
  return obj;
}

string Configuration::get_string_config(const wstring& path, const string& def) {
  JSONValue* ptr = get_path(path);
  if (ptr != nullptr and ptr->IsString())
    return get_locale_string(ptr->AsString());
  return def;
}
int Configuration::get_int_config(const wstring& path, int def) {
  JSONValue* ptr = get_path(path);
  if (ptr != nullptr and ptr->IsNumber())
    return (int)(round(ptr->AsNumber()));
  return def;
}
double Configuration::get_double_config(const wstring& path, double def) {
  JSONValue* ptr = get_path(path);
  if (ptr != nullptr and ptr->IsNumber())
    return ptr->AsNumber();
  return def;
}
bool Configuration::get_bool_config(const wstring& path, bool def) {
  JSONValue* ptr = get_path(path);
  if (ptr != nullptr and ptr->IsBool())
    return ptr->AsBool();
  return def;
}
