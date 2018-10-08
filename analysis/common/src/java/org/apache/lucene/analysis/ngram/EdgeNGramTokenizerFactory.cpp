using namespace std;

#include "EdgeNGramTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "EdgeNGramTokenizer.h"

namespace org::apache::lucene::analysis::ngram
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

EdgeNGramTokenizerFactory::EdgeNGramTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxGramSize(getInt(args, L"maxGramSize",
                         EdgeNGramTokenizer::DEFAULT_MAX_GRAM_SIZE)),
      minGramSize(getInt(args, L"minGramSize",
                         EdgeNGramTokenizer::DEFAULT_MIN_GRAM_SIZE))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<Tokenizer>
EdgeNGramTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<EdgeNGramTokenizer>(factory, minGramSize, maxGramSize);
}
} // namespace org::apache::lucene::analysis::ngram