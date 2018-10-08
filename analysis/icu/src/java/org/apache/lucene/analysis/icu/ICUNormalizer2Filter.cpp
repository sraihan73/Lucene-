using namespace std;

#include "ICUNormalizer2Filter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::icu
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using com::ibm::icu::text::Normalizer;
using com::ibm::icu::text::Normalizer2;

ICUNormalizer2Filter::ICUNormalizer2Filter(shared_ptr<TokenStream> input)
    : ICUNormalizer2Filter(input,
                           Normalizer2::getInstance(nullptr, L"nfkc_cf",
                                                    Normalizer2::Mode::COMPOSE))
{
}

ICUNormalizer2Filter::ICUNormalizer2Filter(shared_ptr<TokenStream> input,
                                           shared_ptr<Normalizer2> normalizer)
    : org::apache::lucene::analysis::TokenFilter(input), normalizer(normalizer)
{
}

bool ICUNormalizer2Filter::incrementToken() 
{
  if (input->incrementToken()) {
    if (normalizer->quickCheck(termAtt) != Normalizer::YES) {
      buffer->setLength(0);
      normalizer->normalize(termAtt, buffer);
      termAtt->setEmpty()->append(buffer);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::icu