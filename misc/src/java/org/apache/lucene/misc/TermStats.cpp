using namespace std;

#include "TermStats.h"

namespace org::apache::lucene::misc
{
using BytesRef = org::apache::lucene::util::BytesRef;

TermStats::TermStats(const wstring &field, shared_ptr<BytesRef> termtext,
                     int df, int64_t tf)
{
  this->termtext = BytesRef::deepCopyOf(termtext);
  this->field = field;
  this->docFreq = df;
  this->totalTermFreq = tf;
}

wstring TermStats::getTermText() { return termtext->utf8ToString(); }

wstring TermStats::toString()
{
  return (L"TermStats: term=" + termtext->utf8ToString() + L" docFreq=" +
          to_wstring(docFreq) + L" totalTermFreq=" + to_wstring(totalTermFreq));
}
} // namespace org::apache::lucene::misc