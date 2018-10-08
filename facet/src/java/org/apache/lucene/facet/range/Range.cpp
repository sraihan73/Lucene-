using namespace std;

#include "Range.h"

namespace org::apache::lucene::facet::range
{

Range::Range(const wstring &label) : label(label)
{
  if (label == L"") {
    throw make_shared<NullPointerException>(L"label must not be null");
  }
}

void Range::failNoMatch()
{
  throw invalid_argument(L"range \"" + label + L"\" matches nothing");
}
} // namespace org::apache::lucene::facet::range