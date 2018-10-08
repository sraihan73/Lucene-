using namespace std;

#include "LetterTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::core
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

LetterTokenizer::LetterTokenizer() {}

LetterTokenizer::LetterTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::util::CharTokenizer(factory)
{
}

LetterTokenizer::LetterTokenizer(shared_ptr<AttributeFactory> factory,
                                 int maxTokenLen)
    : org::apache::lucene::analysis::util::CharTokenizer(factory, maxTokenLen)
{
}

bool LetterTokenizer::isTokenChar(int c) { return isalpha(c); }
} // namespace org::apache::lucene::analysis::core