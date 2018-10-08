using namespace std;

#include "IndicNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "IndicNormalizationFilter.h"

namespace org::apache::lucene::analysis::in_
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using IndicNormalizationFilter =
    org::apache::lucene::analysis::in_::IndicNormalizationFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

IndicNormalizationFilterFactory::IndicNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
IndicNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<IndicNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
IndicNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::in_