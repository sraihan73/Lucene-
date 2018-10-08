using namespace std;

#include "ICUFoldingFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "ICUFoldingFilter.h"

namespace org::apache::lucene::analysis::icu
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ICUFoldingFilter = org::apache::lucene::analysis::icu::ICUFoldingFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using com::ibm::icu::text::FilteredNormalizer2;
using com::ibm::icu::text::Normalizer2;
using com::ibm::icu::text::UnicodeSet;

ICUFoldingFilterFactory::ICUFoldingFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{

  shared_ptr<Normalizer2> normalizer = ICUFoldingFilter::NORMALIZER;
  wstring filter = get(args, L"filter");
  if (filter != L"") {
    shared_ptr<UnicodeSet> set = make_shared<UnicodeSet>(filter);
    if (!set->isEmpty()) {
      set->freeze();
      normalizer = make_shared<FilteredNormalizer2>(normalizer, set);
    }
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
  this->normalizer = normalizer;
}

shared_ptr<TokenStream>
ICUFoldingFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ICUFoldingFilter>(input, normalizer);
}

shared_ptr<AbstractAnalysisFactory>
ICUFoldingFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::icu