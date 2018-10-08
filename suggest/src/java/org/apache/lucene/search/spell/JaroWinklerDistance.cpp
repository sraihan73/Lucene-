using namespace std;

#include "JaroWinklerDistance.h"

namespace org::apache::lucene::search::spell
{

JaroWinklerDistance::JaroWinklerDistance() {}

std::deque<int> JaroWinklerDistance::matches(const wstring &s1,
                                              const wstring &s2)
{
  wstring max, min;
  if (s1.length() > s2.length()) {
    max = s1;
    min = s2;
  } else {
    max = s2;
    min = s1;
  }
  int range = max(max.length() / 2 - 1, 0);
  std::deque<int> matchIndexes(min.length());
  Arrays::fill(matchIndexes, -1);
  std::deque<bool> matchFlags(max.length());
  int matches = 0;
  for (int mi = 0; mi < min.length(); mi++) {
    wchar_t c1 = min[mi];
    for (int xi = max(mi - range, 0), xn = min(mi + range + 1, max.length());
         xi < xn; xi++) {
      if (!matchFlags[xi] && c1 == max[xi]) {
        matchIndexes[mi] = xi;
        matchFlags[xi] = true;
        matches++;
        break;
      }
    }
  }
  std::deque<wchar_t> ms1(matches);
  std::deque<wchar_t> ms2(matches);
  for (int i = 0, si = 0; i < min.length(); i++) {
    if (matchIndexes[i] != -1) {
      ms1[si] = min[i];
      si++;
    }
  }
  for (int i = 0, si = 0; i < max.length(); i++) {
    if (matchFlags[i]) {
      ms2[si] = max[i];
      si++;
    }
  }
  int transpositions = 0;
  for (int mi = 0; mi < ms1.size(); mi++) {
    if (ms1[mi] != ms2[mi]) {
      transpositions++;
    }
  }
  int prefix = 0;
  for (int mi = 0; mi < min.length(); mi++) {
    if (s1[mi] == s2[mi]) {
      prefix++;
    } else {
      break;
    }
  }
  return std::deque<int>{matches, transpositions / 2, prefix, max.length()};
}

float JaroWinklerDistance::getDistance(const wstring &s1, const wstring &s2)
{
  std::deque<int> mtp = matches(s1, s2);
  float m = mtp[0];
  if (m == 0) {
    return 0.0f;
  }
  float j = ((m / s1.length() + m / s2.length() + (m - mtp[1]) / m)) / 3;
  float jw =
      j < getThreshold() ? j : j + min(0.1f, 1.0f / mtp[3]) * mtp[2] * (1 - j);
  return jw;
}

void JaroWinklerDistance::setThreshold(float threshold)
{
  this->threshold = threshold;
}

float JaroWinklerDistance::getThreshold() { return threshold; }

int JaroWinklerDistance::hashCode()
{
  return 113 * Float::floatToIntBits(threshold) * getClass().hashCode();
}

bool JaroWinklerDistance::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (nullptr == obj || getClass() != obj.type()) {
    return false;
  }

  shared_ptr<JaroWinklerDistance> o =
      any_cast<std::shared_ptr<JaroWinklerDistance>>(obj);
  return (Float::floatToIntBits(o->threshold) ==
          Float::floatToIntBits(this->threshold));
}

wstring JaroWinklerDistance::toString()
{
  return L"jarowinkler(" + to_wstring(threshold) + L")";
}
} // namespace org::apache::lucene::search::spell