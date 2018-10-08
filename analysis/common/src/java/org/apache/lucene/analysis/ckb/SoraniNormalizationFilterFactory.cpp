using namespace std;

#include "SoraniNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "SoraniNormalizationFilter.h"

namespace org::apache::lucene::analysis::ckb
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

SoraniNormalizationFilterFactory::SoraniNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<SoraniNormalizationFilter>
SoraniNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<SoraniNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
SoraniNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::ckb