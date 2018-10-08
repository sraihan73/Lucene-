using namespace std;

#include "EdgeNGramFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "EdgeNGramTokenFilter.h"

namespace org::apache::lucene::analysis::ngram
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

EdgeNGramFilterFactory::EdgeNGramFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      maxGramSize(getInt(args, L"maxGramSize",
                         EdgeNGramTokenFilter::DEFAULT_MAX_GRAM_SIZE)),
      minGramSize(getInt(args, L"minGramSize",
                         EdgeNGramTokenFilter::DEFAULT_MIN_GRAM_SIZE)),
      preserveOriginal(
          getBoolean(args, L"preserveOriginal",
                     EdgeNGramTokenFilter::DEFAULT_PRESERVE_ORIGINAL))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenFilter>
EdgeNGramFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<EdgeNGramTokenFilter>(input, minGramSize, maxGramSize,
                                           preserveOriginal);
}
} // namespace org::apache::lucene::analysis::ngram