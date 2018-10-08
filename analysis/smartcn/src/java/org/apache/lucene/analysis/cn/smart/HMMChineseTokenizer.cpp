using namespace std;

#include "HMMChineseTokenizer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "WordSegmenter.h"
#include "hhmm/SegToken.h"

namespace org::apache::lucene::analysis::cn::smart
{
using SegToken = org::apache::lucene::analysis::cn::smart::hhmm::SegToken;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using SegmentingTokenizerBase =
    org::apache::lucene::analysis::util::SegmentingTokenizerBase;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
const shared_ptr<java::text::BreakIterator> HMMChineseTokenizer::sentenceProto =
    java::text::BreakIterator::getSentenceInstance(java::util::Locale::ROOT);

HMMChineseTokenizer::HMMChineseTokenizer()
    : HMMChineseTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY)
{
}

HMMChineseTokenizer::HMMChineseTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::util::SegmentingTokenizerBase(
          factory, (BreakIterator)sentenceProto->clone())
{
}

void HMMChineseTokenizer::setNextSentence(int sentenceStart, int sentenceEnd)
{
  wstring sentence =
      wstring(buffer, sentenceStart, sentenceEnd - sentenceStart);
  tokens =
      wordSegmenter->segmentSentence(sentence, offset + sentenceStart).begin();
}

bool HMMChineseTokenizer::incrementWord()
{
  if (tokens == nullptr || !tokens->hasNext()) {
    return false;
  } else {
    shared_ptr<SegToken> token = tokens->next();
    clearAttributes();
    termAtt->copyBuffer(token->charArray, 0, token->charArray.size());
    offsetAtt->setOffset(correctOffset(token->startOffset),
                         correctOffset(token->endOffset));
    typeAtt->setType(L"word");
    return true;
  }
}

void HMMChineseTokenizer::reset() 
{
  SegmentingTokenizerBase::reset();
  tokens.reset();
}
} // namespace org::apache::lucene::analysis::cn::smart