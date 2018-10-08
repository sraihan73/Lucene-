using namespace std;

#include "CJKWidthFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "CJKWidthFilter.h"

namespace org::apache::lucene::analysis::cjk
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CJKWidthFilter = org::apache::lucene::analysis::cjk::CJKWidthFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

CJKWidthFilterFactory::CJKWidthFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
CJKWidthFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<CJKWidthFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
CJKWidthFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::cjk