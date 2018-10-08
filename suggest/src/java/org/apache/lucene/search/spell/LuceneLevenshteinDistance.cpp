using namespace std;

#include "LuceneLevenshteinDistance.h"

namespace org::apache::lucene::search::spell
{
using IntsRef = org::apache::lucene::util::IntsRef;

LuceneLevenshteinDistance::LuceneLevenshteinDistance() {}

float LuceneLevenshteinDistance::getDistance(const wstring &target,
                                             const wstring &other)
{
  shared_ptr<IntsRef> targetPoints;
  shared_ptr<IntsRef> otherPoints;
  int n;
  std::deque<std::deque<int>> d; // cost array

  // NOTE: if we cared, we could 3*m space instead of m*n space, similar to
  // what LevenshteinDistance does, except cycling thru a ring of three
  // horizontal cost arrays... but this comparator is never actually used by
  // DirectSpellChecker, it's only used for merging results from multiple shards
  // in "distributed spellcheck", and it's inefficient in other ways too...

  // cheaper to do this up front once
  targetPoints = toIntsRef(target);
  otherPoints = toIntsRef(other);
  n = targetPoints->length;
  constexpr int m = otherPoints->length;
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: d = new int[n+1][m+1];
  d = RectangularVectors::ReturnRectangularIntVector(n + 1, m + 1);

  if (n == 0 || m == 0) {
    if (n == m) {
      return 0;
    } else {
      return max(n, m);
    }
  }

  // indexes into strings s and t
  int i; // iterates through s
  int j; // iterates through t

  int t_j; // jth character of t

  int cost; // cost

  for (i = 0; i <= n; i++) {
    d[i][0] = i;
  }

  for (j = 0; j <= m; j++) {
    d[0][j] = j;
  }

  for (j = 1; j <= m; j++) {
    t_j = otherPoints->ints[j - 1];

    for (i = 1; i <= n; i++) {
      cost = targetPoints->ints[i - 1] == t_j ? 0 : 1;
      // minimum of cell to the left+1, to the top+1, diagonally left and up
      // +cost
      d[i][j] =
          min(min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);
      // transposition
      if (i > 1 && j > 1 &&
          targetPoints->ints[i - 1] == otherPoints->ints[j - 2] &&
          targetPoints->ints[i - 2] == otherPoints->ints[j - 1]) {
        d[i][j] = min(d[i][j], d[i - 2][j - 2] + cost);
      }
    }
  }

  return 1.0f - (static_cast<float>(d[n][m]) / min(m, n));
}

shared_ptr<IntsRef> LuceneLevenshteinDistance::toIntsRef(const wstring &s)
{
  shared_ptr<IntsRef> ref = make_shared<IntsRef>(s.length()); // worst case
  int utf16Len = s.length();
  for (int i = 0, cp = 0; i < utf16Len; i += Character::charCount(cp)) {
    cp = ref->ints[ref->length++] = Character::codePointAt(s, i);
  }
  return ref;
}

bool LuceneLevenshteinDistance::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (nullptr == obj) {
    return false;
  }
  return (getClass() == obj.type());
}
} // namespace org::apache::lucene::search::spell