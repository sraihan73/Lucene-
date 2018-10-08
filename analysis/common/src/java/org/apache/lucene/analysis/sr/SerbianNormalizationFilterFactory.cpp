using namespace std;

#include "SerbianNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "SerbianNormalizationFilter.h"
#include "SerbianNormalizationRegularFilter.h"

namespace org::apache::lucene::analysis::sr
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

SerbianNormalizationFilterFactory::SerbianNormalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      haircut(
          get(args, L"haircut", Arrays::asList(L"bald", L"regular"), L"bald"))
{

  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
SerbianNormalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  if (this->haircut == L"regular") {
    return make_shared<SerbianNormalizationRegularFilter>(input);
  } else {
    return make_shared<SerbianNormalizationFilter>(input);
  }
}

shared_ptr<AbstractAnalysisFactory>
SerbianNormalizationFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::sr