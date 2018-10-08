using namespace std;

#include "ScandinavianNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "ScandinavianNormalizationFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ScandinavianNormalizationFilterFactory::ScandinavianNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ScandinavianNormalizationFilter>
ScandinavianNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ScandinavianNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
ScandinavianNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::miscellaneous