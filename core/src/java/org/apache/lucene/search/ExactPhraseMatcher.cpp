using namespace std;

#include "ExactPhraseMatcher.h"

namespace org::apache::lucene::search
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;

ExactPhraseMatcher::PostingsAndPosition::PostingsAndPosition(
    shared_ptr<PostingsEnum> postings, int offset)
    : postings(postings), offset(offset)
{
}

ExactPhraseMatcher::ExactPhraseMatcher(
    std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings,
    float matchCost)
    : PhraseMatcher(approximation(postings), matchCost),
      postings(postingsAndPositions::toArray(
          std::deque<std::shared_ptr<PostingsAndPosition>>(
              postingsAndPositions->size())))
{
  deque<std::shared_ptr<PostingsAndPosition>> postingsAndPositions =
      deque<std::shared_ptr<PostingsAndPosition>>();
  for (auto posting : postings) {
    postingsAndPositions.push_back(
        make_shared<PostingsAndPosition>(posting->postings, posting->position));
  }
}

shared_ptr<DocIdSetIterator> ExactPhraseMatcher::approximation(
    std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings)
{
  deque<std::shared_ptr<DocIdSetIterator>> iterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  for (auto posting : postings) {
    iterators.push_back(posting->postings);
  }
  return ConjunctionDISI::intersectIterators(iterators);
}

float ExactPhraseMatcher::maxFreq()
{
  int minFreq = postings[0]->freq;
  for (int i = 1; i < postings.size(); i++) {
    minFreq = min(minFreq, postings[i]->freq);
  }
  return minFreq;
}

bool ExactPhraseMatcher::advancePosition(
    shared_ptr<PostingsAndPosition> posting, int target) 
{
  while (posting->pos < target) {
    if (posting->upTo == posting->freq) {
      return false;
    } else {
      posting->pos = posting->postings.nextPosition();
      posting->upTo += 1;
    }
  }
  return true;
}

void ExactPhraseMatcher::reset() 
{
  for (auto posting : postings) {
    posting->freq = posting->postings.freq();
    posting->pos = -1;
    posting->upTo = 0;
  }
}

bool ExactPhraseMatcher::nextMatch() 
{
  shared_ptr<PostingsAndPosition> *const lead = postings[0];
  if (lead->upTo < lead->freq) {
    lead->pos = lead->postings.nextPosition();
    lead->upTo += 1;
  } else {
    return false;
  }
  while (true) {
    constexpr int phrasePos = lead->pos - lead->offset;
    for (int j = 1; j < postings.size(); ++j) {
      shared_ptr<PostingsAndPosition> *const posting = postings[j];
      constexpr int expectedPos = phrasePos + posting->offset;

      // advance up to the same position as the lead
      if (advancePosition(posting, expectedPos) == false) {
        goto advanceHeadBreak;
      }

      if (posting->pos != expectedPos) { // we advanced too far
        if (advancePosition(lead,
                            posting->pos - posting->offset + lead->offset)) {
          goto advanceHeadContinue;
        } else {
          goto advanceHeadBreak;
        }
      }
    }
    return true;
  advanceHeadContinue:;
  }
advanceHeadBreak:
  return false;
}

float ExactPhraseMatcher::sloppyWeight(
    shared_ptr<Similarity::SimScorer> simScorer)
{
  return 1;
}

int ExactPhraseMatcher::startPosition() { return postings[0]->pos; }

int ExactPhraseMatcher::endPosition()
{
  return postings[postings.size() - 1]->pos;
}

int ExactPhraseMatcher::startOffset() 
{
  return postings[0]->postings.startOffset();
}

int ExactPhraseMatcher::endOffset() 
{
  return postings[postings.size() - 1]->postings.endOffset();
}
} // namespace org::apache::lucene::search