using namespace std;

#include "HindiNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "HindiNormalizationFilter.h"

namespace org::apache::lucene::analysis::hi
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HindiNormalizationFilter =
    org::apache::lucene::analysis::hi::HindiNormalizationFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

HindiNormalizationFilterFactory::HindiNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
HindiNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<HindiNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
HindiNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::hi