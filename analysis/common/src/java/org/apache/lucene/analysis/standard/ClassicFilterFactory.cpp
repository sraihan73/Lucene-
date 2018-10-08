using namespace std;

#include "ClassicFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ClassicFilter.h"

namespace org::apache::lucene::analysis::standard
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using ClassicFilter = org::apache::lucene::analysis::standard::ClassicFilter;

ClassicFilterFactory::ClassicFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenFilter>
ClassicFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ClassicFilter>(input);
}
} // namespace org::apache::lucene::analysis::standard