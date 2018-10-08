using namespace std;

#include "GalicianMinimalStemmer.h"

namespace org::apache::lucene::analysis::gl
{
using RSLPStemmerBase = org::apache::lucene::analysis::pt::RSLPStemmerBase;
const shared_ptr<Step> GalicianMinimalStemmer::pluralStep =
    parse(GalicianMinimalStemmer::typeid, L"galician.rslp")[L"Plural"];

int GalicianMinimalStemmer::stem(std::deque<wchar_t> &s, int len)
{
  return pluralStep->apply(s, len);
}
} // namespace org::apache::lucene::analysis::gl