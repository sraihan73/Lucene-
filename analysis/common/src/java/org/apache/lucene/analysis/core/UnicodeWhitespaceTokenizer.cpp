using namespace std;

#include "UnicodeWhitespaceTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../util/UnicodeProps.h"

namespace org::apache::lucene::analysis::core
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using UnicodeProps = org::apache::lucene::analysis::util::UnicodeProps;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

UnicodeWhitespaceTokenizer::UnicodeWhitespaceTokenizer() {}

UnicodeWhitespaceTokenizer::UnicodeWhitespaceTokenizer(
    shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::util::CharTokenizer(factory)
{
}

UnicodeWhitespaceTokenizer::UnicodeWhitespaceTokenizer(
    shared_ptr<AttributeFactory> factory, int maxTokenLen)
    : org::apache::lucene::analysis::util::CharTokenizer(factory, maxTokenLen)
{
}

bool UnicodeWhitespaceTokenizer::isTokenChar(int c)
{
  return !UnicodeProps::WHITESPACE->get(c);
}
} // namespace org::apache::lucene::analysis::core