using namespace std;

#include "LowerCaseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "LowerCaseFilter.h"

namespace org::apache::lucene::analysis::core
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

LowerCaseFilterFactory::LowerCaseFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<LowerCaseFilter>
LowerCaseFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<LowerCaseFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
LowerCaseFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::core