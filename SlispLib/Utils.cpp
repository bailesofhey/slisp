#include <string>

#include "Utils.h"

using namespace std;

bool Utils::EndsWith(const string &haystack, const string &needle) {
  size_t haystackLen = haystack.length();
  size_t startIdx = haystackLen - needle.length();
  if (startIdx >= 0 && startIdx < haystackLen)
    return haystack.substr(startIdx) == needle;
  return false;
}
