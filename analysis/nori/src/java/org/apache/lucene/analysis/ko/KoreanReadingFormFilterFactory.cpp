using namespace std;

#include "KoreanReadingFormFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "KoreanReadingFormFilter.h"

namespace org::apache::lucene::analysis::ko
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

KoreanReadingFormFilterFactory::KoreanReadingFormFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
KoreanReadingFormFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<KoreanReadingFormFilter>(input);
}
} // namespace org::apache::lucene::analysis::ko