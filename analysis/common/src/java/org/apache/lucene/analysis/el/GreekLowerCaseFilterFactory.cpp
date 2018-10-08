using namespace std;

#include "GreekLowerCaseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "GreekLowerCaseFilter.h"

namespace org::apache::lucene::analysis::el
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GreekLowerCaseFilter =
    org::apache::lucene::analysis::el::GreekLowerCaseFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GreekLowerCaseFilterFactory::GreekLowerCaseFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<GreekLowerCaseFilter>
GreekLowerCaseFilterFactory::create(shared_ptr<TokenStream> in_)
{
  return make_shared<GreekLowerCaseFilter>(in_);
}

shared_ptr<AbstractAnalysisFactory>
GreekLowerCaseFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::el