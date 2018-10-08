using namespace std;

#include "NGramTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "NGramTokenizer.h"

namespace org::apache::lucene::analysis::ngram
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

NGramTokenizerFactory::NGramTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxGramSize(
          getInt(args, L"maxGramSize", NGramTokenizer::DEFAULT_MAX_NGRAM_SIZE)),
      minGramSize(
          getInt(args, L"minGramSize", NGramTokenizer::DEFAULT_MIN_NGRAM_SIZE))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<Tokenizer>
NGramTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<NGramTokenizer>(factory, minGramSize, maxGramSize);
}
} // namespace org::apache::lucene::analysis::ngram