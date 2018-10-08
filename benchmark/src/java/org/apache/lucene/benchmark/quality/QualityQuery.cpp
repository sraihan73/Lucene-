using namespace std;

#include "QualityQuery.h"

namespace org::apache::lucene::benchmark::quality
{

QualityQuery::QualityQuery(const wstring &queryID,
                           unordered_map<wstring, wstring> &nameValPairs)
{
  this->queryID = queryID;
  this->nameValPairs = nameValPairs;
}

std::deque<wstring> QualityQuery::getNames()
{
  return nameValPairs.keySet().toArray(std::deque<wstring>(0));
}

wstring QualityQuery::getValue(const wstring &name)
{
  return nameValPairs[name];
}

wstring QualityQuery::getQueryID() { return queryID; }

int QualityQuery::compareTo(shared_ptr<QualityQuery> other)
{
  try {
    // compare as ints when ids ints
    int n = stoi(queryID);
    int nOther = stoi(other->queryID);
    return n - nOther;
  } catch (const NumberFormatException &e) {
    // fall back to string comparison
    return queryID.compare(other->queryID);
  }
}
} // namespace org::apache::lucene::benchmark::quality