using namespace std;

#include "ArabicNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "ArabicNormalizationFilter.h"

namespace org::apache::lucene::analysis::ar
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ArabicNormalizationFilter =
    org::apache::lucene::analysis::ar::ArabicNormalizationFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ArabicNormalizationFilterFactory::ArabicNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ArabicNormalizationFilter>
ArabicNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ArabicNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
ArabicNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::ar