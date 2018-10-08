using namespace std;

#include "ScoreDoc.h"

namespace org::apache::lucene::search
{

ScoreDoc::ScoreDoc(int doc, float score) : ScoreDoc(doc, score, -1) {}

ScoreDoc::ScoreDoc(int doc, float score, int shardIndex)
{
  this->doc = doc;
  this->score = score;
  this->shardIndex = shardIndex;
}

wstring ScoreDoc::toString()
{
  return L"doc=" + to_wstring(doc) + L" score=" + to_wstring(score) +
         L" shardIndex=" + to_wstring(shardIndex);
}
} // namespace org::apache::lucene::search