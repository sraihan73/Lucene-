using namespace std;

#include "FixedShingleFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FixedShingleFilter.h"

namespace org::apache::lucene::analysis::shingle
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

FixedShingleFilterFactory::FixedShingleFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      shingleSize(getInt(args, L"shingleSize", 2)),
      tokenSeparator(get(args, L"tokenSeparator", L" ")),
      fillerToken(get(args, L"fillerToken", L"_"))
{
}

shared_ptr<TokenStream>
FixedShingleFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FixedShingleFilter>(input, shingleSize, tokenSeparator,
                                         fillerToken);
}
} // namespace org::apache::lucene::analysis::shingle