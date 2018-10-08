using namespace std;

#include "SegmentingTokenizerBase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "CharArrayIterator.h"

namespace org::apache::lucene::analysis::util
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

SegmentingTokenizerBase::SegmentingTokenizerBase(
    shared_ptr<BreakIterator> iterator)
    : SegmentingTokenizerBase(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, iterator)
{
}

SegmentingTokenizerBase::SegmentingTokenizerBase(
    shared_ptr<AttributeFactory> factory, shared_ptr<BreakIterator> iterator)
    : org::apache::lucene::analysis::Tokenizer(factory), iterator(iterator)
{
}

bool SegmentingTokenizerBase::incrementToken() 
{
  if (length == 0 || !incrementWord()) {
    while (!incrementSentence()) {
      refill();
      if (length <= 0) // no more bytes to read;
      {
        return false;
      }
    }
  }

  return true;
}

void SegmentingTokenizerBase::reset() 
{
  Tokenizer::reset();
  wrapper->setText(buffer, 0, 0);
  iterator->setText(wrapper);
  length = usableLength = offset = 0;
}

void SegmentingTokenizerBase::end() 
{
  Tokenizer::end();
  constexpr int finalOffset =
      correctOffset(length < 0 ? offset : offset + length);
  offsetAtt->setOffset(finalOffset, finalOffset);
}

int SegmentingTokenizerBase::findSafeEnd()
{
  for (int i = length - 1; i >= 0; i--) {
    if (isSafeEnd(buffer[i])) {
      return i + 1;
    }
  }
  return -1;
}

bool SegmentingTokenizerBase::isSafeEnd(wchar_t ch)
{
  switch (ch) {
  case 0x000D:
  case 0x000A:
  case 0x0085:
  case 0x2028:
  case 0x2029:
    return true;
  default:
    return false;
  }
}

void SegmentingTokenizerBase::refill() 
{
  offset += usableLength;
  int leftover = length - usableLength;
  System::arraycopy(buffer, usableLength, buffer, 0, leftover);
  int requested = buffer.size() - leftover;
  int returned = read(input, buffer, leftover, requested);
  length = returned < 0 ? leftover : returned + leftover;
  if (returned < requested) // reader has been emptied, process the rest
  {
    usableLength = length;
  } else { // still more data to be read, find a safe-stopping place
    usableLength = findSafeEnd();
    if (usableLength < 0) {
      usableLength = length; /*
    }
                                    * more than IOBUFFER of text without breaks,
                                    * gonna possibly truncate tokens
                                    */
    }

    wrapper->setText(buffer, 0, max(0, usableLength));
    iterator->setText(wrapper);
  }

  int SegmentingTokenizerBase::read(shared_ptr<Reader> input,
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

  bool SegmentingTokenizerBase::incrementSentence() 
  {
    if (length == 0) // we must refill the buffer
    {
      return false;
    }

    while (true) {
      int start = iterator->current();

      if (start == BreakIterator::DONE) {
        return false; // BreakIterator exhausted
      }

      // find the next set of boundaries
      int end = iterator->next();

      if (end == BreakIterator::DONE) {
        return false; // BreakIterator exhausted
      }

      setNextSentence(start, end);
      if (incrementWord()) {
        return true;
      }
    }
  }
}