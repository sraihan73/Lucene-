using namespace std;

#include "PhrasePositions.h"

namespace org::apache::lucene::search
{
using namespace org::apache::lucene::index;

PhrasePositions::PhrasePositions(shared_ptr<PostingsEnum> postings, int o,
                                 int ord,
                                 std::deque<std::shared_ptr<Term>> &terms)
    : ord(ord), postings(postings), terms(terms)
{
  offset = o;
}

void PhrasePositions::firstPosition() 
{
  count = postings->freq(); // read first pos
  nextPosition();
}

bool PhrasePositions::nextPosition() 
{
  if (count-- > 0) { // read subsequent pos's
    position = postings->nextPosition() - offset;
    return true;
  } else {
    return false;
  }
}

wstring PhrasePositions::toString()
{
  wstring s = L"o:" + to_wstring(offset) + L" p:" + to_wstring(position) +
              L" c:" + to_wstring(count);
  if (rptGroup >= 0) {
    s += L" rpt:" + to_wstring(rptGroup) + L",i" + to_wstring(rptInd);
  }
  return s;
}
} // namespace org::apache::lucene::search