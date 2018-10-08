using namespace std;

#include "ICUTokenizer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../tokenattributes/ScriptAttribute.h"
#include "CompositeBreakIterator.h"
#include "ICUTokenizerConfig.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ScriptAttribute =
    org::apache::lucene::analysis::icu::tokenattributes::ScriptAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::text::BreakIterator;

ICUTokenizer::ICUTokenizer()
    : ICUTokenizer(new DefaultICUTokenizerConfig(true, true))
{
}

ICUTokenizer::ICUTokenizer(shared_ptr<ICUTokenizerConfig> config)
    : ICUTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, config)
{
}

ICUTokenizer::ICUTokenizer(shared_ptr<AttributeFactory> factory,
                           shared_ptr<ICUTokenizerConfig> config)
    : org::apache::lucene::analysis::Tokenizer(factory),
      breaker(make_shared<CompositeBreakIterator>(config)), config(config)
{
}

bool ICUTokenizer::incrementToken() 
{
  clearAttributes();
  if (length == 0) {
    refill();
  }
  while (!incrementTokenBuffer()) {
    refill();
    if (length <= 0) // no more bytes to read;
    {
      return false;
    }
  }
  return true;
}

void ICUTokenizer::reset() 
{
  Tokenizer::reset();
  breaker->setText(buffer, 0, 0);
  length = usableLength = offset = 0;
}

void ICUTokenizer::end() 
{
  Tokenizer::end();
  constexpr int finalOffset = (length < 0) ? offset : offset + length;
  offsetAtt->setOffset(correctOffset(finalOffset), correctOffset(finalOffset));
}

int ICUTokenizer::findSafeEnd()
{
  for (int i = length - 1; i >= 0; i--) {
    if (UCharacter::isWhitespace(buffer[i])) {
      return i + 1;
    }
  }
  return -1;
}

void ICUTokenizer::refill() 
{
  offset += usableLength;
  int leftover = length - usableLength;
  System::arraycopy(buffer, usableLength, buffer, 0, leftover);
  int requested = buffer.size() - leftover;
  int returned = read(input, buffer, leftover, requested);
  length = returned + leftover;
  if (returned < requested) // reader has been emptied, process the rest
  {
    usableLength = length;
  } else { // still more data to be read, find a safe-stopping place
    usableLength = findSafeEnd();
    if (usableLength < 0) {
      usableLength = length; /*
    }
                                    * more than IOBUFFER of text without space,
                                    * gonna possibly truncate tokens
                                    */
    }

    breaker->setText(buffer, 0, max(0, usableLength));
  }

  int ICUTokenizer::read(shared_ptr<Reader> input,
                         std::deque<wchar_t> & buffer, int offset,
                         int length) 
  {
    assert(
        (length >= 0, L"length must not be negative: " + to_wstring(length)));

    int remaining = length;
    while (remaining > 0) {
      int location = length - remaining;
      int count = input->read(buffer, offset + location, remaining);
      if (-1 == count) { // EOF
        break;
      }
      remaining -= count;
    }
    return length - remaining;
  }

  bool ICUTokenizer::incrementTokenBuffer()
  {
    int start = breaker->current();
    assert(start != BreakIterator::DONE);

    // find the next set of boundaries, skipping over non-tokens (rule status 0)
    int end = breaker->next();
    while (end != BreakIterator::DONE && breaker->getRuleStatus() == 0) {
      start = end;
      end = breaker->next();
    }

    if (end == BreakIterator::DONE) {
      return false; // BreakIterator exhausted
    }

    termAtt->copyBuffer(buffer, start, end - start);
    offsetAtt->setOffset(correctOffset(offset + start),
                         correctOffset(offset + end));
    typeAtt->setType(
        config->getType(breaker->getScriptCode(), breaker->getRuleStatus()));
    scriptAtt->setCode(breaker->getScriptCode());

    return true;
  }
}