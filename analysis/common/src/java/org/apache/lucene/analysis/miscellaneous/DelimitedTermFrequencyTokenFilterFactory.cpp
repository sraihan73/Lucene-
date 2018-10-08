using namespace std;

#include "DelimitedTermFrequencyTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "DelimitedTermFrequencyTokenFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring DelimitedTermFrequencyTokenFilterFactory::DELIMITER_ATTR =
    L"delimiter";

DelimitedTermFrequencyTokenFilterFactory::
    DelimitedTermFrequencyTokenFilterFactory(
        unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      delimiter(getChar(args, DELIMITER_ATTR,
                        DelimitedTermFrequencyTokenFilter::DEFAULT_DELIMITER))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<DelimitedTermFrequencyTokenFilter>
DelimitedTermFrequencyTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<DelimitedTermFrequencyTokenFilter>(input, delimiter);
}
} // namespace org::apache::lucene::analysis::miscellaneous