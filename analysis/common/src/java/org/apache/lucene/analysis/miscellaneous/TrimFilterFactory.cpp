using namespace std;

#include "TrimFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "TrimFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TrimFilter = org::apache::lucene::analysis::miscellaneous::TrimFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

TrimFilterFactory::TrimFilterFactory(unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TrimFilter> TrimFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<TrimFilter>(input);
}

shared_ptr<AbstractAnalysisFactory> TrimFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::miscellaneous