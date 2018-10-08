using namespace std;

#include "PortugueseMinimalStemmer.h"

namespace org::apache::lucene::analysis::pt
{

const shared_ptr<Step> PortugueseMinimalStemmer::pluralStep =
    parse(PortugueseMinimalStemmer::typeid, L"portuguese.rslp")[L"Plural"];

int PortugueseMinimalStemmer::stem(std::deque<wchar_t> &s, int len)
{
  return pluralStep->apply(s, len);
}
} // namespace org::apache::lucene::analysis::pt