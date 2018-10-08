using namespace std;

#include "PassageScorer.h"

namespace org::apache::lucene::search::uhighlight
{
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

PassageScorer::PassageScorer() : PassageScorer(1.2f, 0.75f, 87.0f)
{
  // 1.2 and 0.75 are well-known bm25 defaults (but maybe not the best here) ?
  // 87 is typical average english sentence length.
}

PassageScorer::PassageScorer(float k1, float b, float pivot)
    : k1(k1), b(b), pivot(pivot)
{
}

float PassageScorer::weight(int contentLength, int totalTermFreq)
{
  // approximate #docs from content length
  float numDocs = 1 + contentLength / pivot;
  // numDocs not numDocs - docFreq (ala DFR), since we approximate numDocs
  return (k1 + 1) *
         static_cast<float>(log(1 + (numDocs + 0.5) / (totalTermFreq + 0.5)));
}

float PassageScorer::tf(int freq, int passageLen)
{
  float norm = k1 * ((1 - b) + b * (passageLen / pivot));
  return freq / (freq + norm);
}

float PassageScorer::norm(int passageStart)
{
  return 1 + 1 / static_cast<float>(log(pivot + passageStart));
}

float PassageScorer::score(shared_ptr<Passage> passage, int contentLength)
{
  float score = 0;
  shared_ptr<BytesRefHash> termsHash = make_shared<BytesRefHash>();
  int hitCount = passage->getNumMatches();
  std::deque<int> termFreqsInPassage(hitCount); // maximum size
  std::deque<int> termFreqsInDoc(hitCount);
  Arrays::fill(termFreqsInPassage, 0);

  for (int i = 0; i < passage->getNumMatches(); i++) {
    int termIndex = termsHash->add(passage->getMatchTerms()[i]);
    if (termIndex < 0) {
      termIndex = -(termIndex + 1);
    } else {
      termFreqsInDoc[termIndex] = passage->getMatchTermFreqsInDoc()[i];
    }
    termFreqsInPassage[termIndex]++;
  }

  for (int i = 0; i < termsHash->size(); i++) {
    score += tf(termFreqsInPassage[i], passage->getLength()) *
             weight(contentLength, termFreqsInDoc[i]);
  }
  score *= norm(passage->getStartOffset());
  return score;
}
} // namespace org::apache::lucene::search::uhighlight