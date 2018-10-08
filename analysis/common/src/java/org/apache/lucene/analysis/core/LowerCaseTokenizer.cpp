using namespace std;

#include "LowerCaseTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::core
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

LowerCaseTokenizer::LowerCaseTokenizer() {}

LowerCaseTokenizer::LowerCaseTokenizer(shared_ptr<AttributeFactory> factory)
    : LetterTokenizer(factory)
{
}

LowerCaseTokenizer::LowerCaseTokenizer(shared_ptr<AttributeFactory> factory,
                                       int maxTokenLen)
    : LetterTokenizer(factory, maxTokenLen)
{
}

int LowerCaseTokenizer::normalize(int c) { return towlower(c); }
} // namespace org::apache::lucene::analysis::core