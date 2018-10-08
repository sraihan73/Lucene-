using namespace std;

#include "NGramFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "NGramTokenFilter.h"

namespace org::apache::lucene::analysis::ngram
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

NGramFilterFactory::NGramFilterFactory(unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      maxGramSize(getInt(args, L"maxGramSize",
                         NGramTokenFilter::DEFAULT_MAX_NGRAM_SIZE)),
      minGramSize(getInt(args, L"minGramSize",
                         NGramTokenFilter::DEFAULT_MIN_NGRAM_SIZE)),
      preserveOriginal(getBoolean(args, L"keepShortTerm",
                                  NGramTokenFilter::DEFAULT_PRESERVE_ORIGINAL))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenFilter>
NGramFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<NGramTokenFilter>(input, minGramSize, maxGramSize,
                                       preserveOriginal);
}
} // namespace org::apache::lucene::analysis::ngram