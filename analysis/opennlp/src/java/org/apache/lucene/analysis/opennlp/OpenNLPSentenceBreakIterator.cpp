using namespace std;

#include "OpenNLPSentenceBreakIterator.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/CharArrayIterator.h"
#include "tools/NLPSentenceDetectorOp.h"

namespace org::apache::lucene::analysis::opennlp
{
using opennlp::tools::util::Span;
using NLPSentenceDetectorOp =
    org::apache::lucene::analysis::opennlp::tools::NLPSentenceDetectorOp;
using CharArrayIterator =
    org::apache::lucene::analysis::util::CharArrayIterator;

OpenNLPSentenceBreakIterator::OpenNLPSentenceBreakIterator(
    shared_ptr<NLPSentenceDetectorOp> sentenceOp)
{
  this->sentenceOp = sentenceOp;
}

int OpenNLPSentenceBreakIterator::current() { return text->getIndex(); }

int OpenNLPSentenceBreakIterator::first()
{
  currentSentence = 0;
  text->setIndex(text->getBeginIndex());
  return current();
}

int OpenNLPSentenceBreakIterator::last()
{
  if (sentenceStarts.size() > 0) {
    currentSentence = sentenceStarts.size() - 1;
    text->setIndex(text->getEndIndex());
  } else { // there are no sentences; both the first and last positions are the
           // begin index
    currentSentence = 0;
    text->setIndex(text->getBeginIndex());
  }
  return current();
}

int OpenNLPSentenceBreakIterator::next()
{
  if (text->getIndex() == text->getEndIndex() || 0 == sentenceStarts.size()) {
    return DONE;
  } else if (currentSentence < sentenceStarts.size() - 1) {
    text->setIndex(sentenceStarts[++currentSentence]);
    return current();
  } else {
    return last();
  }
}

int OpenNLPSentenceBreakIterator::following(int pos)
{
  if (pos < text->getBeginIndex() || pos > text->getEndIndex()) {
    throw invalid_argument(L"offset out of bounds");
  } else if (0 == sentenceStarts.size()) {
    text->setIndex(text->getBeginIndex());
    return DONE;
  } else if (pos >= sentenceStarts[sentenceStarts.size() - 1]) {
    // this conflicts with the javadocs, but matches actual behavior (Oracle has
    // a bug in something) https://bugs.openjdk.java.net/browse/JDK-8015110
    text->setIndex(text->getEndIndex());
    currentSentence = sentenceStarts.size() - 1;
    return DONE;
  } else { // there are at least two sentences
    currentSentence =
        (sentenceStarts.size() - 1) / 2; // start search from the middle
    moveToSentenceAt(pos, 0, sentenceStarts.size() - 2);
    text->setIndex(sentenceStarts[++currentSentence]);
    return current();
  }
}

void OpenNLPSentenceBreakIterator::moveToSentenceAt(int pos, int minSentence,
                                                    int maxSentence)
{
  if (minSentence != maxSentence) {
    if (pos < sentenceStarts[currentSentence]) {
      int newMaxSentence = currentSentence - 1;
      currentSentence = minSentence + (currentSentence - minSentence) / 2;
      moveToSentenceAt(pos, minSentence, newMaxSentence);
    } else if (pos >= sentenceStarts[currentSentence + 1]) {
      int newMinSentence = currentSentence + 1;
      currentSentence = maxSentence - (maxSentence - currentSentence) / 2;
      moveToSentenceAt(pos, newMinSentence, maxSentence);
    }
  } else {
    assert(currentSentence == minSentence);
    assert(pos >= sentenceStarts[currentSentence]);
    assert(currentSentence == sentenceStarts.size() - 1 &&
           pos <= text->getEndIndex()) ||
        pos < sentenceStarts[currentSentence + 1];
  }
  // we have arrived - nothing to do
}

int OpenNLPSentenceBreakIterator::previous()
{
  if (text->getIndex() == text->getBeginIndex()) {
    return DONE;
  } else {
    if (0 == sentenceStarts.size()) {
      text->setIndex(text->getBeginIndex());
      return DONE;
    }
    if (text->getIndex() == text->getEndIndex()) {
      text->setIndex(sentenceStarts[currentSentence]);
    } else {
      text->setIndex(sentenceStarts[--currentSentence]);
    }
    return current();
  }
}

int OpenNLPSentenceBreakIterator::preceding(int pos)
{
  if (pos < text->getBeginIndex() || pos > text->getEndIndex()) {
    throw invalid_argument(L"offset out of bounds");
  } else if (0 == sentenceStarts.size()) {
    text->setIndex(text->getBeginIndex());
    currentSentence = 0;
    return DONE;
  } else if (pos < sentenceStarts[0]) {
    // this conflicts with the javadocs, but matches actual behavior (Oracle has
    // a bug in something) https://bugs.openjdk.java.net/browse/JDK-8015110
    text->setIndex(text->getBeginIndex());
    currentSentence = 0;
    return DONE;
  } else {
    currentSentence = sentenceStarts.size() / 2; // start search from the middle
    moveToSentenceAt(pos, 0, sentenceStarts.size() - 1);
    if (0 == currentSentence) {
      text->setIndex(text->getBeginIndex());
      return DONE;
    } else {
      text->setIndex(sentenceStarts[--currentSentence]);
      return current();
    }
  }
}

int OpenNLPSentenceBreakIterator::next(int n)
{
  currentSentence += n;
  if (n < 0) {
    if (text->getIndex() == text->getEndIndex()) {
      ++currentSentence;
    }
    if (currentSentence < 0) {
      currentSentence = 0;
      text->setIndex(text->getBeginIndex());
      return DONE;
    } else {
      text->setIndex(sentenceStarts[currentSentence]);
    }
  } else if (n > 0) {
    if (currentSentence >= sentenceStarts.size()) {
      currentSentence = sentenceStarts.size() - 1;
      text->setIndex(text->getEndIndex());
      return DONE;
    } else {
      text->setIndex(sentenceStarts[currentSentence]);
    }
  }
  return current();
}

shared_ptr<CharacterIterator> OpenNLPSentenceBreakIterator::getText()
{
  return text;
}

void OpenNLPSentenceBreakIterator::setText(
    shared_ptr<CharacterIterator> newText)
{
  text = newText;
  text->setIndex(text->getBeginIndex());
  currentSentence = 0;
  std::deque<std::shared_ptr<Span>> spans =
      sentenceOp->splitSentences(characterIteratorToString());
  sentenceStarts = std::deque<int>(spans.size());
  for (int i = 0; i < spans.size(); ++i) {
    // Adjust start positions to match those of the passed-in CharacterIterator
    sentenceStarts[i] = spans[i]->getStart() + text->getBeginIndex();
  }
}

wstring OpenNLPSentenceBreakIterator::characterIteratorToString()
{
  wstring fullText;
  if (std::dynamic_pointer_cast<CharArrayIterator>(text) != nullptr) {
    shared_ptr<CharArrayIterator> charArrayIterator =
        std::static_pointer_cast<CharArrayIterator>(text);
    fullText =
        wstring(charArrayIterator->getText(), charArrayIterator->getStart(),
                charArrayIterator->getLength());
  } else {
    // TODO: is there a better way to extract full text from arbitrary
    // CharacterIterators?
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    for (wchar_t ch = text->first(); ch != CharacterIterator::DONE;
         ch = text->next()) {
      builder->append(ch);
    }
    fullText = builder->toString();
    text->setIndex(text->getBeginIndex());
  }
  return fullText;
}
} // namespace org::apache::lucene::analysis::opennlp