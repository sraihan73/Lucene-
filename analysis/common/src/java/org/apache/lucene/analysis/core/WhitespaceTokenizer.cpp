using namespace std;

#include "WhitespaceTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::core
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

WhitespaceTokenizer::WhitespaceTokenizer() {}

WhitespaceTokenizer::WhitespaceTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::util::CharTokenizer(factory)
{
}

WhitespaceTokenizer::WhitespaceTokenizer(shared_ptr<AttributeFactory> factory,
                                         int maxTokenLen)
    : org::apache::lucene::analysis::util::CharTokenizer(factory, maxTokenLen)
{
}

bool WhitespaceTokenizer::isTokenChar(int c) { return !isspace(c); }
} // namespace org::apache::lucene::analysis::core