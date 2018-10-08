using namespace std;

#include "PersianNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "PersianNormalizationFilter.h"

namespace org::apache::lucene::analysis::fa
{
using PersianNormalizationFilter =
    org::apache::lucene::analysis::fa::PersianNormalizationFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PersianNormalizationFilterFactory::PersianNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<PersianNormalizationFilter>
PersianNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PersianNormalizationFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
PersianNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::fa