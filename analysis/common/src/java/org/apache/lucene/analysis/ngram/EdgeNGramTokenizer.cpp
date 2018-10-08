using namespace std;

#include "EdgeNGramTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::ngram
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

EdgeNGramTokenizer::EdgeNGramTokenizer(int minGram, int maxGram)
    : NGramTokenizer(minGram, maxGram, true)
{
}

EdgeNGramTokenizer::EdgeNGramTokenizer(shared_ptr<AttributeFactory> factory,
                                       int minGram, int maxGram)
    : NGramTokenizer(factory, minGram, maxGram, true)
{
}
} // namespace org::apache::lucene::analysis::ngram