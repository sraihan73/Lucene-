using namespace std;

#include "TermSpans.h"

namespace org::apache::lucene::search::spans
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Similarity = org::apache::lucene::search::similarities::Similarity;

TermSpans::TermSpans(shared_ptr<Similarity::SimScorer> scorer,
                     shared_ptr<PostingsEnum> postings, shared_ptr<Term> term,
                     float positionsCost)
    : postings(Objects::requireNonNull(postings)),
      term(Objects::requireNonNull(term)), positionsCost(positionsCost)
{
  this->doc = -1;
  this->position = -1;
  assert(positionsCost > 0); // otherwise the TermSpans should not be created.
}

int TermSpans::nextDoc() 
{
  doc = postings->nextDoc();
  if (doc != DocIdSetIterator::NO_MORE_DOCS) {
    freq = postings->freq();
    assert(freq >= 1);
    count = 0;
  }
  position = -1;
  return doc;
}

int TermSpans::advance(int target) 
{
  assert(target > doc);
  doc = postings->advance(target);
  if (doc != DocIdSetIterator::NO_MORE_DOCS) {
    freq = postings->freq();
    assert(freq >= 1);
    count = 0;
  }
  position = -1;
  return doc;
}

int TermSpans::docID() { return doc; }

int TermSpans::nextStartPosition() 
{
  if (count == freq) {
    assert(position != NO_MORE_POSITIONS);
    return position = NO_MORE_POSITIONS;
  }
  int prevPosition = position;
  position = postings->nextPosition();
  assert(
      (position >= prevPosition, L"prevPosition=" + to_wstring(prevPosition) +
                                     L" > position=" + to_wstring(position)));
  assert(position != NO_MORE_POSITIONS); // int endPosition not possible
  count++;
  readPayload = false;
  return position;
}

int TermSpans::startPosition() { return position; }

int TermSpans::endPosition()
{
  return (position == -1) ? -1
                          : (position != NO_MORE_POSITIONS) ? position + 1
                                                            : NO_MORE_POSITIONS;
}

int TermSpans::width() { return 0; }

int64_t TermSpans::cost() { return postings->cost(); }

void TermSpans::collect(shared_ptr<SpanCollector> collector) 
{
  collector->collectLeaf(postings, position, term);
}

float TermSpans::positionsCost() { return positionsCost_; }

wstring TermSpans::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"spans(" + term->toString() + L")@" +
         StringHelper::toString(
             doc == -1 ? L"START"
                       : (doc == NO_MORE_DOCS)
                             ? L"ENDDOC"
                             : to_wstring(doc) + L" - " +
                                   to_wstring(position == NO_MORE_POSITIONS
                                                  ? L"ENDPOS"
                                                  : position));
}

shared_ptr<PostingsEnum> TermSpans::getPostings() { return postings; }
} // namespace org::apache::lucene::search::spans