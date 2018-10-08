using namespace std;

#include "ICUFoldingFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::icu
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using com::ibm::icu::text::Normalizer2;
const shared_ptr<com::ibm::icu::text::Normalizer2>
    ICUFoldingFilter::NORMALIZER =
        com::ibm::icu::text::Normalizer2::getInstance(
            ICUFoldingFilter::typeid->getResourceAsStream(L"utr30.nrm"),
            L"utr30", com::ibm::icu::text::Normalizer2::Mode::COMPOSE);

ICUFoldingFilter::ICUFoldingFilter(shared_ptr<TokenStream> input)
    : ICUNormalizer2Filter(input, NORMALIZER)
{
}

ICUFoldingFilter::ICUFoldingFilter(shared_ptr<TokenStream> input,
                                   shared_ptr<Normalizer2> normalizer)
    : ICUNormalizer2Filter(input, normalizer)
{
}
} // namespace org::apache::lucene::analysis::icu