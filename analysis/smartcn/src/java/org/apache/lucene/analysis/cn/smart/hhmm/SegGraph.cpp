using namespace std;

#include "SegGraph.h"
#include "SegToken.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{

bool SegGraph::isStartExist(int s) { return tokenListTable[s].size() > 0; }

deque<std::shared_ptr<SegToken>> SegGraph::getStartList(int s)
{
  return tokenListTable[s];
}

int SegGraph::getMaxStart() { return maxStart; }

deque<std::shared_ptr<SegToken>> SegGraph::makeIndex()
{
  deque<std::shared_ptr<SegToken>> result =
      deque<std::shared_ptr<SegToken>>();
  int s = -1, count = 0, size = tokenListTable.size();
  deque<std::shared_ptr<SegToken>> tokenList;
  int index = 0;
  while (count < size) {
    if (isStartExist(s)) {
      tokenList = tokenListTable[s];
      for (auto st : tokenList) {
        st->index = index;
        result.push_back(st);
        index++;
      }
      count++;
    }
    s++;
  }
  return result;
}

void SegGraph::addToken(shared_ptr<SegToken> token)
{
  int s = token->startOffset;
  if (!isStartExist(s)) {
    deque<std::shared_ptr<SegToken>> newlist =
        deque<std::shared_ptr<SegToken>>();
    newlist.push_back(token);
    tokenListTable.emplace(s, newlist);
  } else {
    deque<std::shared_ptr<SegToken>> tokenList = tokenListTable[s];
    tokenList.push_back(token);
  }
  if (s > maxStart) {
    maxStart = s;
  }
}

deque<std::shared_ptr<SegToken>> SegGraph::toTokenList()
{
  deque<std::shared_ptr<SegToken>> result =
      deque<std::shared_ptr<SegToken>>();
  int s = -1, count = 0, size = tokenListTable.size();
  deque<std::shared_ptr<SegToken>> tokenList;

  while (count < size) {
    if (isStartExist(s)) {
      tokenList = tokenListTable[s];
      for (auto st : tokenList) {
        result.push_back(st);
      }
      count++;
    }
    s++;
  }
  return result;
}

wstring SegGraph::toString()
{
  deque<std::shared_ptr<SegToken>> tokenList = this->toTokenList();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (auto t : tokenList) {
    sb->append(t + L"\n");
  }
  return sb->toString();
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm