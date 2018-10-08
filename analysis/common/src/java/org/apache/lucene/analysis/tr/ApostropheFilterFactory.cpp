using namespace std;

#include "ApostropheFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ApostropheFilter.h"

namespace org::apache::lucene::analysis::tr
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ApostropheFilterFactory::ApostropheFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameter(s): " + args);
  }
}

shared_ptr<TokenStream>
ApostropheFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ApostropheFilter>(input);
}
} // namespace org::apache::lucene::analysis::tr