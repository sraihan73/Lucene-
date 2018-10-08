using namespace std;

#include "ThaiTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../util/CharArrayIterator.h"

namespace org::apache::lucene::analysis::th
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharArrayIterator =
    org::apache::lucene::analysis::util::CharArrayIterator;
using SegmentingTokenizerBase =
    org::apache::lucene::analysis::util::SegmentingTokenizerBase;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
const shared_ptr<java::text::BreakIterator> ThaiTokenizer::proto =
    java::text::BreakIterator::getWordInstance(
        make_shared<java::util::Locale>(L"th"));

ThaiTokenizer::StaticConstructor::StaticConstructor()
{
  // check that we have a working dictionary-based break iterator for thai
  proto->setText(L"ภาษาไทย");
  DBBI_AVAILABLE = proto->isBoundary(4);
}

ThaiTokenizer::StaticConstructor ThaiTokenizer::staticConstructor;
const shared_ptr<java::text::BreakIterator> ThaiTokenizer::sentenceProto =
    java::text::BreakIterator::getSentenceInstance(java::util::Locale::ROOT);

ThaiTokenizer::ThaiTokenizer() : ThaiTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY)
{
}

ThaiTokenizer::ThaiTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::util::SegmentingTokenizerBase(
          factory, (BreakIterator)sentenceProto->clone()),
      wordBreaker(std::static_pointer_cast<BreakIterator>(proto->clone()))
{
  if (!DBBI_AVAILABLE) {
    throw make_shared<UnsupportedOperationException>(
        L"This JRE does not have support for Thai segmentation");
  }
}

void ThaiTokenizer::setNextSentence(int sentenceStart, int sentenceEnd)
{
  this->sentenceStart = sentenceStart;
  this->sentenceEnd = sentenceEnd;
  wrapper->setText(buffer, sentenceStart, sentenceEnd - sentenceStart);
  wordBreaker->setText(wrapper);
}

bool ThaiTokenizer::incrementWord()
{
  int start = wordBreaker->current();
  if (start == BreakIterator::DONE) {
    return false; // BreakIterator exhausted
  }

  // find the next set of boundaries, skipping over non-tokens
  int end = wordBreaker->next();
  while (end != BreakIterator::DONE &&
         !isalnum(Character::codePointAt(buffer, sentenceStart + start,
                                         sentenceEnd))) {
    start = end;
    end = wordBreaker->next();
  }

  if (end == BreakIterator::DONE) {
    return false; // BreakIterator exhausted
  }

  clearAttributes();
  termAtt->copyBuffer(buffer, sentenceStart + start, end - start);
  offsetAtt->setOffset(correctOffset(offset + sentenceStart + start),
                       correctOffset(offset + sentenceStart + end));
  return true;
}
} // namespace org::apache::lucene::analysis::th