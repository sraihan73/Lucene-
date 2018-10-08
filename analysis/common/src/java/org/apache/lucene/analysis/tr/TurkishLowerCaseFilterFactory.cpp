using namespace std;

#include "TurkishLowerCaseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "TurkishLowerCaseFilter.h"

namespace org::apache::lucene::analysis::tr
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TurkishLowerCaseFilter =
    org::apache::lucene::analysis::tr::TurkishLowerCaseFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

TurkishLowerCaseFilterFactory::TurkishLowerCaseFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
TurkishLowerCaseFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<TurkishLowerCaseFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
TurkishLowerCaseFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::tr