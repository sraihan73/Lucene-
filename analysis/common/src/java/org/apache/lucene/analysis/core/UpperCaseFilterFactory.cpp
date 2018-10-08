using namespace std;

#include "UpperCaseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "UpperCaseFilter.h"

namespace org::apache::lucene::analysis::core
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using UpperCaseFilter = org::apache::lucene::analysis::core::UpperCaseFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

UpperCaseFilterFactory::UpperCaseFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<UpperCaseFilter>
UpperCaseFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<UpperCaseFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
UpperCaseFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::core