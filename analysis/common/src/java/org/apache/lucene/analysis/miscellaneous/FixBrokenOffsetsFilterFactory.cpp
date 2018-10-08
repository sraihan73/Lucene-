using namespace std;

#include "FixBrokenOffsetsFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FixBrokenOffsetsFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

FixBrokenOffsetsFilterFactory::FixBrokenOffsetsFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
}

shared_ptr<TokenStream>
FixBrokenOffsetsFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FixBrokenOffsetsFilter>(input);
}
} // namespace org::apache::lucene::analysis::miscellaneous