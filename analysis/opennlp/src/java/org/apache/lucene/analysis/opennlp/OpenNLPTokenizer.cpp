using namespace std;

#include "OpenNLPTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "tools/NLPSentenceDetectorOp.h"
#include "tools/NLPTokenizerOp.h"

namespace org::apache::lucene::analysis::opennlp
{
using opennlp::tools::util::Span;
using NLPSentenceDetectorOp =
    org::apache::lucene::analysis::opennlp::tools::NLPSentenceDetectorOp;
using NLPTokenizerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPTokenizerOp;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using SegmentingTokenizerBase =
    org::apache::lucene::analysis::util::SegmentingTokenizerBase;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
int OpenNLPTokenizer::EOS_FLAG_BIT = 1;

OpenNLPTokenizer::OpenNLPTokenizer(
    shared_ptr<AttributeFactory> factory,
    shared_ptr<NLPSentenceDetectorOp> sentenceOp,
    shared_ptr<NLPTokenizerOp> tokenizerOp) 
    : org::apache::lucene::analysis::util::SegmentingTokenizerBase(
          factory, new OpenNLPSentenceBreakIterator(sentenceOp))
{
  if (sentenceOp == nullptr || tokenizerOp == nullptr) {
    throw invalid_argument(L"OpenNLPTokenizer: both a Sentence Detector and a "
                           L"Tokenizer are required");
  }
  this->sentenceOp = sentenceOp;
  this->tokenizerOp = tokenizerOp;
}

OpenNLPTokenizer::~OpenNLPTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  termSpans.clear();
  termNum = sentenceStart = 0;
};

void OpenNLPTokenizer::setNextSentence(int sentenceStart, int sentenceEnd)
{
  this->sentenceStart = sentenceStart;
  wstring sentenceText =
      wstring(buffer, sentenceStart, sentenceEnd - sentenceStart);
  termSpans = tokenizerOp->getTerms(sentenceText);
  termNum = 0;
}

bool OpenNLPTokenizer::incrementWord()
{
  if (termSpans.empty() || termNum == termSpans.size()) {
    return false;
  }
  clearAttributes();
  shared_ptr<Span> term = termSpans[termNum];
  termAtt->copyBuffer(buffer, sentenceStart + term->getStart(), term->length());
  offsetAtt->setOffset(correctOffset(offset + sentenceStart + term->getStart()),
                       correctOffset(offset + sentenceStart + term->getEnd()));
  if (termNum == termSpans.size() - 1) {
    flagsAtt->setFlags(
        flagsAtt->getFlags() |
        EOS_FLAG_BIT); // mark the last token in the sentence with EOS_FLAG_BIT
  }
  ++termNum;
  return true;
}

void OpenNLPTokenizer::reset() 
{
  SegmentingTokenizerBase::reset();
  termSpans.clear();
  termNum = sentenceStart = 0;
}
} // namespace org::apache::lucene::analysis::opennlp