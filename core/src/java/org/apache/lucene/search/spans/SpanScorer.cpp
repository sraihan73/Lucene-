using namespace std;

#include "SpanScorer.h"

namespace org::apache::lucene::search::spans
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Similarity = org::apache::lucene::search::similarities::Similarity;

SpanScorer::SpanScorer(shared_ptr<SpanWeight> weight, shared_ptr<Spans> spans,
                       shared_ptr<Similarity::SimScorer> docScorer)
    : org::apache::lucene::search::Scorer(weight),
      spans(Objects::requireNonNull(spans)), docScorer(docScorer)
{
}

shared_ptr<Spans> SpanScorer::getSpans() { return spans; }

int SpanScorer::docID() { return spans->docID(); }

shared_ptr<DocIdSetIterator> SpanScorer::iterator() { return spans; }

shared_ptr<TwoPhaseIterator> SpanScorer::twoPhaseIterator()
{
  return spans->asTwoPhaseIterator();
}

float SpanScorer::scoreCurrentDoc() 
{
  assert((docScorer != nullptr, getClass() + L" has a null docScorer!"));
  return docScorer->score(docID(), freq);
}

void SpanScorer::setFreqCurrentDoc() 
{
  freq = 0.0f;
  numMatches = 0;

  spans->doStartCurrentDoc();

  assert((spans->startPosition() == -1,
          L"incorrect initial start position, " + spans));
  assert((spans->endPosition() == -1,
          L"incorrect initial end position, " + spans));
  int prevStartPos = -1;
  int prevEndPos = -1;

  int startPos = spans->nextStartPosition();
  assert((startPos != Spans::NO_MORE_POSITIONS,
          L"initial startPos NO_MORE_POSITIONS, " + spans));
  do {
    assert(startPos >= prevStartPos);
    int endPos = spans->endPosition();
    assert(endPos != Spans::NO_MORE_POSITIONS);
    // This assertion can fail for Or spans on the same term:
    // assert (startPos != prevStartPos) || (endPos > prevEndPos) : "non
    // increased endPos="+endPos;
    assert(startPos != prevStartPos) || (endPos >= prevEndPos)
        : L"decreased endPos=" + to_wstring(endPos);
    numMatches++;
    if (docScorer == nullptr) { // scores not required, break out here
      freq = 1;
      return;
    }
    freq += docScorer->computeSlopFactor(spans->width());
    spans->doCurrentSpans();
    prevStartPos = startPos;
    prevEndPos = endPos;
    startPos = spans->nextStartPosition();
  } while (startPos != Spans::NO_MORE_POSITIONS);

  assert((spans->startPosition() == Spans::NO_MORE_POSITIONS,
          L"incorrect final start position, " + spans));
  assert((spans->endPosition() == Spans::NO_MORE_POSITIONS,
          L"incorrect final end position, " + spans));
}

void SpanScorer::ensureFreq() 
{
  int currentDoc = docID();
  if (lastScoredDoc != currentDoc) {
    setFreqCurrentDoc();
    lastScoredDoc = currentDoc;
  }
}

float SpanScorer::score() 
{
  ensureFreq();
  return scoreCurrentDoc();
}

float SpanScorer::sloppyFreq() 
{
  ensureFreq();
  return freq;
}
} // namespace org::apache::lucene::search::spans