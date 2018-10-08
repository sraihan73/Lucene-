using namespace std;

#include "CharTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharacterUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::util
{
using CharacterBuffer =
    org::apache::lucene::analysis::CharacterUtils::CharacterBuffer;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using LetterTokenizer = org::apache::lucene::analysis::core::LetterTokenizer;
using LowerCaseTokenizer =
    org::apache::lucene::analysis::core::LowerCaseTokenizer;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

CharTokenizer::CharTokenizer() : maxTokenLen(DEFAULT_MAX_WORD_LEN) {}

CharTokenizer::CharTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::Tokenizer(factory),
      maxTokenLen(DEFAULT_MAX_WORD_LEN)
{
}

CharTokenizer::CharTokenizer(shared_ptr<AttributeFactory> factory,
                             int maxTokenLen)
    : org::apache::lucene::analysis::Tokenizer(factory),
      maxTokenLen(maxTokenLen)
{
  if (maxTokenLen > MAX_TOKEN_LENGTH_LIMIT || maxTokenLen <= 0) {
    throw invalid_argument(
        L"maxTokenLen must be greater than 0 and less than " +
        MAX_TOKEN_LENGTH_LIMIT + L" passed: " + to_wstring(maxTokenLen));
  }
}

shared_ptr<CharTokenizer>
CharTokenizer::fromTokenCharPredicate(function<bool(int)> &tokenCharPredicate)
{
  return fromTokenCharPredicate(DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                tokenCharPredicate);
}

shared_ptr<CharTokenizer>
CharTokenizer::fromTokenCharPredicate(shared_ptr<AttributeFactory> factory,
                                      function<bool(int)> &tokenCharPredicate)
{
  return fromTokenCharPredicate(factory, tokenCharPredicate,
                                function<int(int)>::identity());
}

shared_ptr<CharTokenizer>
CharTokenizer::fromTokenCharPredicate(function<bool(int)> &tokenCharPredicate,
                                      function<int(int)> &normalizer)
{
  return fromTokenCharPredicate(DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                tokenCharPredicate, normalizer);
}

shared_ptr<CharTokenizer>
CharTokenizer::fromTokenCharPredicate(shared_ptr<AttributeFactory> factory,
                                      function<bool(int)> &tokenCharPredicate,
                                      function<int(int)> &normalizer)
{
  Objects::requireNonNull(tokenCharPredicate, L"predicate must not be null.");
  Objects::requireNonNull(normalizer, L"normalizer must not be null");
  return make_shared<CharTokenizerAnonymousInnerClass>(
      factory, tokenCharPredicate, normalizer);
}

CharTokenizer::CharTokenizerAnonymousInnerClass::
    CharTokenizerAnonymousInnerClass(shared_ptr<AttributeFactory> factory,
                                     function<bool(int)> &tokenCharPredicate,
                                     function<int(int)> &normalizer)
    : CharTokenizer(factory)
{
  this->tokenCharPredicate = tokenCharPredicate;
  this->normalizer = normalizer;
}

bool CharTokenizer::CharTokenizerAnonymousInnerClass::isTokenChar(int c)
{
  return tokenCharPredicate(c);
}

int CharTokenizer::CharTokenizerAnonymousInnerClass::normalize(int c)
{
  return normalizer(c);
}

shared_ptr<CharTokenizer> CharTokenizer::fromSeparatorCharPredicate(
    function<bool(int)> &separatorCharPredicate)
{
  return fromSeparatorCharPredicate(DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                    separatorCharPredicate);
}

shared_ptr<CharTokenizer> CharTokenizer::fromSeparatorCharPredicate(
    shared_ptr<AttributeFactory> factory,
    function<bool(int)> &separatorCharPredicate)
{
  return fromSeparatorCharPredicate(factory, separatorCharPredicate,
                                    function<int(int)>::identity());
}

shared_ptr<CharTokenizer> CharTokenizer::fromSeparatorCharPredicate(
    function<bool(int)> &separatorCharPredicate, function<int(int)> &normalizer)
{
  return fromSeparatorCharPredicate(DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                    separatorCharPredicate, normalizer);
}

shared_ptr<CharTokenizer> CharTokenizer::fromSeparatorCharPredicate(
    shared_ptr<AttributeFactory> factory,
    function<bool(int)> &separatorCharPredicate, function<int(int)> &normalizer)
{
  return fromTokenCharPredicate(factory, separatorCharPredicate.negate(),
                                normalizer);
}

int CharTokenizer::normalize(int c) { return c; }

bool CharTokenizer::incrementToken() 
{
  clearAttributes();
  int length = 0;
  int start = -1; // this variable is always initialized
  int end = -1;
  std::deque<wchar_t> buffer = termAtt->buffer();
  while (true) {
    if (bufferIndex >= dataLen) {
      offset += dataLen;
      CharacterUtils::fill(
          ioBuffer, input); // read supplementary char aware with CharacterUtils
      if (ioBuffer->getLength() == 0) {
        dataLen = 0; // so next offset += dataLen won't decrement offset
        if (length > 0) {
          break;
        } else {
          finalOffset = correctOffset(offset);
          return false;
        }
      }
      dataLen = ioBuffer->getLength();
      bufferIndex = 0;
    }
    // use CharacterUtils here to support < 3.1 UTF-16 code unit behavior if the
    // char based methods are gone
    constexpr int c = Character::codePointAt(ioBuffer->getBuffer(), bufferIndex,
                                             ioBuffer->getLength());
    constexpr int charCount = Character::charCount(c);
    bufferIndex += charCount;

    if (isTokenChar(c)) { // if it's a token char
      if (length == 0) {  // start of token
        assert(start == -1);
        start = offset + bufferIndex - charCount;
        end = start;
      } else if (length >=
                 buffer.size() -
                     1) { // check if a supplementary could run out of bounds
        buffer = termAtt->resizeBuffer(
            2 + length); // make sure a supplementary fits in the buffer
      }
      end += charCount;
      length += Character::toChars(normalize(c), buffer,
                                   length); // buffer it, normalized
      if (length >= maxTokenLen) { // buffer overflow! make sure to check for >=
                                   // surrogate pair could break == test
        break;
      }
    } else if (length > 0) { // at non-Letter w/ chars
      break;                 // return 'em
    }
  }

  termAtt->setLength(length);
  assert(start != -1);
  offsetAtt->setOffset(correctOffset(start), finalOffset = correctOffset(end));
  return true;
}

void CharTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  offsetAtt->setOffset(finalOffset, finalOffset);
}

void CharTokenizer::reset() 
{
  Tokenizer::reset();
  bufferIndex = 0;
  offset = 0;
  dataLen = 0;
  finalOffset = 0;
  ioBuffer->reset(); // make sure to reset the IO buffer!!
}
} // namespace org::apache::lucene::analysis::util