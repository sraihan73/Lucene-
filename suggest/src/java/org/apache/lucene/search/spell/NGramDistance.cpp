using namespace std;

#include "NGramDistance.h"

namespace org::apache::lucene::search::spell
{

NGramDistance::NGramDistance(int size) { this->n = size; }

NGramDistance::NGramDistance() : NGramDistance(2) {}

float NGramDistance::getDistance(const wstring &source, const wstring &target)
{
  constexpr int sl = source.length();
  constexpr int tl = target.length();

  if (sl == 0 || tl == 0) {
    if (sl == tl) {
      return 1;
    } else {
      return 0;
    }
  }

  int cost = 0;
  if (sl < n || tl < n) {
    for (int i = 0, ni = min(sl, tl); i < ni; i++) {
      if (source[i] == target[i]) {
        cost++;
      }
    }
    return static_cast<float>(cost) / max(sl, tl);
  }

  std::deque<wchar_t> sa(sl + n - 1);
  std::deque<float> p;  //'previous' cost array, horizontally
  std::deque<float> d;  // cost array, horizontally
  std::deque<float> _d; // placeholder to assist in swapping p and d

  // construct sa with prefix
  for (int i = 0; i < sa.size(); i++) {
    if (i < n - 1) {
      sa[i] = 0; // add prefix
    } else {
      sa[i] = source[i - n + 1];
    }
  }
  p = std::deque<float>(sl + 1);
  d = std::deque<float>(sl + 1);

  // indexes into strings s and t
  int i; // iterates through source
  int j; // iterates through target

  std::deque<wchar_t> t_j(n); // jth n-gram of t

  for (i = 0; i <= sl; i++) {
    p[i] = i;
  }

  for (j = 1; j <= tl; j++) {
    // construct t_j n-gram
    if (j < n) {
      for (int ti = 0; ti < n - j; ti++) {
        t_j[ti] = 0; // add prefix
      }
      for (int ti = n - j; ti < n; ti++) {
        t_j[ti] = target[ti - (n - j)];
      }
    } else {
      t_j = target.substr(j - n, j - (j - n))->toCharArray();
    }
    d[0] = j;
    for (i = 1; i <= sl; i++) {
      cost = 0;
      int tn = n;
      // compare sa to t_j
      for (int ni = 0; ni < n; ni++) {
        if (sa[i - 1 + ni] != t_j[ni]) {
          cost++;
        } else if (sa[i - 1 + ni] == 0) { // discount matches on prefix
          tn--;
        }
      }
      float ec = static_cast<float>(cost) / tn;
      // minimum of cell to the left+1, to the top+1, diagonally left and up
      // +cost
      d[i] = min(min(d[i - 1] + 1, p[i] + 1), p[i - 1] + ec);
    }
    // copy current distance counts to 'previous row' distance counts
    _d = p;
    p = d;
    d = _d;
  }

  // our last action in the above loop was to switch d and p, so p now
  // actually has the most recent cost counts
  return 1.0f - (p[sl] / max(tl, sl));
}

int NGramDistance::hashCode() { return 1427 * n * getClass().hashCode(); }

bool NGramDistance::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (nullptr == obj || getClass() != obj.type()) {
    return false;
  }

  shared_ptr<NGramDistance> o = any_cast<std::shared_ptr<NGramDistance>>(obj);
  return o->n == this->n;
}

wstring NGramDistance::toString() { return L"ngram(" + to_wstring(n) + L")"; }
} // namespace org::apache::lucene::search::spell