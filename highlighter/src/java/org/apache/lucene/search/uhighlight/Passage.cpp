using namespace std;

#include "Passage.h"

namespace org::apache::lucene::search::uhighlight
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

void Passage::addMatch(int startOffset, int endOffset,
                       shared_ptr<BytesRef> term, int termFreqInDoc)
{
  assert(startOffset >= this->startOffset && startOffset <= this->endOffset);
  if (numMatches == matchStarts.size()) {
    int newLength = ArrayUtil::oversize(
        numMatches + 1, RamUsageEstimator::NUM_BYTES_OBJECT_REF);
    std::deque<int> newMatchStarts(newLength);
    std::deque<int> newMatchEnds(newLength);
    std::deque<int> newMatchTermFreqInDoc(newLength);
    std::deque<std::shared_ptr<BytesRef>> newMatchTerms(newLength);
    System::arraycopy(matchStarts, 0, newMatchStarts, 0, numMatches);
    System::arraycopy(matchEnds, 0, newMatchEnds, 0, numMatches);
    System::arraycopy(matchTerms, 0, newMatchTerms, 0, numMatches);
    System::arraycopy(matchTermFreqInDoc, 0, newMatchTermFreqInDoc, 0,
                      numMatches);
    matchStarts = newMatchStarts;
    matchEnds = newMatchEnds;
    matchTerms = newMatchTerms;
    matchTermFreqInDoc = newMatchTermFreqInDoc;
  }
  assert(matchStarts.size() == matchEnds.size() &&
         matchEnds.size() == matchTerms.size());
  matchStarts[numMatches] = startOffset;
  matchEnds[numMatches] = endOffset;
  matchTerms[numMatches] = term;
  matchTermFreqInDoc[numMatches] = termFreqInDoc;
  numMatches++;
}

void Passage::reset()
{
  startOffset = endOffset = -1;
  score = 0.0f;
  numMatches = 0;
}

wstring Passage::toString()
{
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>();
  buf->append(L"Passage[")
      ->append(startOffset)
      ->append(L'-')
      ->append(endOffset)
      ->append(L']');
  buf->append(L'{');
  for (int i = 0; i < numMatches; i++) {
    if (i != 0) {
      buf->append(L',');
    }
    buf->append(matchTerms[i]->utf8ToString());
    buf->append(L'[')
        ->append(matchStarts[i] - startOffset)
        ->append(L'-')
        ->append(matchEnds[i] - startOffset)
        ->append(L']');
  }
  buf->append(L'}');
  buf->append(L"score=")->append(score);
  return buf->toString();
}

int Passage::getStartOffset() { return startOffset; }

int Passage::getEndOffset() { return endOffset; }

int Passage::getLength() { return endOffset - startOffset; }

float Passage::getScore() { return score; }

void Passage::setScore(float score) { this->score = score; }

int Passage::getNumMatches() { return numMatches; }

std::deque<int> Passage::getMatchStarts() { return matchStarts; }

std::deque<int> Passage::getMatchEnds() { return matchEnds; }

std::deque<std::shared_ptr<BytesRef>> Passage::getMatchTerms()
{
  return matchTerms;
}

std::deque<int> Passage::getMatchTermFreqsInDoc()
{
  return matchTermFreqInDoc;
}

void Passage::setStartOffset(int startOffset)
{
  this->startOffset = startOffset;
}

void Passage::setEndOffset(int endOffset)
{
  assert(startOffset <= endOffset);
  this->endOffset = endOffset;
}
} // namespace org::apache::lucene::search::uhighlight