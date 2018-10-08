using namespace std;

#include "GermanNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "GermanNormalizationFilter.h"

namespace org::apache::lucene::analysis::de
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GermanNormalizationFilter =
    org::apache::lucene::analysis::de::GermanNormalizationFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GermanNormalizationFilterFactory::GermanNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
GermanNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<GermanNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
GermanNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::de