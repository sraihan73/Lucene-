using namespace std;

#include "KeywordTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::core
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

KeywordTokenizer::KeywordTokenizer() : KeywordTokenizer(DEFAULT_BUFFER_SIZE) {}

KeywordTokenizer::KeywordTokenizer(int bufferSize)
{
  if (bufferSize > MAX_TOKEN_LENGTH_LIMIT || bufferSize <= 0) {
    throw invalid_argument(
        L"maxTokenLen must be greater than 0 and less than " +
        MAX_TOKEN_LENGTH_LIMIT + L" passed: " + to_wstring(bufferSize));
  }
  termAtt->resizeBuffer(bufferSize);
}

KeywordTokenizer::KeywordTokenizer(shared_ptr<AttributeFactory> factory,
                                   int bufferSize)
    : org::apache::lucene::analysis::Tokenizer(factory)
{
  if (bufferSize > MAX_TOKEN_LENGTH_LIMIT || bufferSize <= 0) {
    throw invalid_argument(
        L"maxTokenLen must be greater than 0 and less than " +
        MAX_TOKEN_LENGTH_LIMIT + L" passed: " + to_wstring(bufferSize));
  }
  termAtt->resizeBuffer(bufferSize);
}

bool KeywordTokenizer::incrementToken() 
{
  if (!done) {
    clearAttributes();
    done = true;
    int upto = 0;
    std::deque<wchar_t> buffer = termAtt->buffer();
    while (true) {
      constexpr int length = input->read(buffer, upto, buffer.size() - upto);
      if (length == -1) {
        break;
      }
      upto += length;
      if (upto == buffer.size()) {
        buffer = termAtt->resizeBuffer(1 + buffer.size());
      }
    }
    termAtt->setLength(upto);
    finalOffset = correctOffset(upto);
    offsetAtt->setOffset(correctOffset(0), finalOffset);
    return true;
  }
  return false;
}

void KeywordTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  offsetAtt->setOffset(finalOffset, finalOffset);
}

void KeywordTokenizer::reset() 
{
  Tokenizer::reset();
  this->done = false;
}
} // namespace org::apache::lucene::analysis::core