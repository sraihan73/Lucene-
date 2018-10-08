using namespace std;

#include "MultiBits.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;

MultiBits::MultiBits(std::deque<std::shared_ptr<Bits>> &subs,
                     std::deque<int> &starts, bool defaultValue)
    : subs(subs), starts(starts), defaultValue(defaultValue)
{
  assert(starts.size() == 1 + subs.size());
}

bool MultiBits::checkLength(int reader, int doc)
{
  constexpr int length = starts[1 + reader] - starts[reader];
  assert((doc - starts[reader] < length,
          L"doc=" + to_wstring(doc) + L" reader=" + to_wstring(reader) +
              L" starts[reader]=" + to_wstring(starts[reader]) + L" length=" +
              to_wstring(length)));
  return true;
}

bool MultiBits::get(int doc)
{
  constexpr int reader = ReaderUtil::subIndex(doc, starts);
  assert(reader != -1);
  shared_ptr<Bits> *const bits = subs[reader];
  if (bits == nullptr) {
    return defaultValue;
  } else {
    assert((checkLength(reader, doc)));
    return bits->get(doc - starts[reader]);
  }
}

wstring MultiBits::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(subs.size() + L" subs: ");
  for (int i = 0; i < subs.size(); i++) {
    if (i != 0) {
      b->append(L"; ");
    }
    if (subs[i] == nullptr) {
      b->append(L"s=" + to_wstring(starts[i]) + L" l=null");
    } else {
      b->append(L"s=" + to_wstring(starts[i]) + L" l=" +
                to_wstring(subs[i]->length()) + L" b=" + subs[i]);
    }
  }
  b->append(L" end=" + to_wstring(starts[subs.size()]));
  return b->toString();
}

shared_ptr<SubResult> MultiBits::getMatchingSub(shared_ptr<ReaderSlice> slice)
{
  int reader = ReaderUtil::subIndex(slice->start, starts);
  assert(reader != -1);
  assert((reader < subs.size(), L"slice=" + slice + L" starts[-1]=" +
                                    to_wstring(starts[starts.size() - 1])));
  shared_ptr<SubResult> *const subResult = make_shared<SubResult>();
  if (starts[reader] == slice->start &&
      starts[1 + reader] == slice->start + slice->length) {
    subResult->matches = true;
    subResult->result = subs[reader];
  } else {
    subResult->matches = false;
  }
  return subResult;
}

int MultiBits::length() { return starts[starts.size() - 1]; }
} // namespace org::apache::lucene::index