using namespace std;

#include "BiSegGraph.h"
#include "../Utility.h"
#include "BigramDictionary.h"
#include "PathNode.h"
#include "SegGraph.h"
#include "SegToken.h"
#include "SegTokenPair.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{
using Utility = org::apache::lucene::analysis::cn::smart::Utility;
shared_ptr<BigramDictionary> BiSegGraph::bigramDict =
    BigramDictionary::getInstance();

BiSegGraph::BiSegGraph(shared_ptr<SegGraph> segGraph)
{
  segTokenList = segGraph->makeIndex();
  generateBiSegGraph(segGraph);
}

void BiSegGraph::generateBiSegGraph(shared_ptr<SegGraph> segGraph)
{
  double smooth = 0.1;
  int wordPairFreq = 0;
  int maxStart = segGraph->getMaxStart();
  double oneWordFreq, weight, tinyDouble = 1.0 / Utility::MAX_FREQUENCE;

  int next;
  std::deque<wchar_t> idBuffer;
  // get the deque of tokens ordered and indexed
  segTokenList = segGraph->makeIndex();
  // Because the beginning position of startToken is -1, therefore startToken
  // can be obtained when key = -1
  int key = -1;
  deque<std::shared_ptr<SegToken>> nextTokens;
  while (key < maxStart) {
    if (segGraph->isStartExist(key)) {

      deque<std::shared_ptr<SegToken>> tokenList = segGraph->getStartList(key);

      // Calculate all tokens for a given key.
      for (auto t1 : tokenList) {
        oneWordFreq = t1->weight;
        next = t1->endOffset;
        nextTokens.clear();
        // Find the next corresponding Token.
        // For example: "Sunny seashore", the present Token is "sunny", next one
        // should be "sea" or "seashore". If we cannot find the next Token, then
        // go to the end and repeat the same cycle.
        while (next <= maxStart) {
          // Because the beginning position of endToken is sentenceLen, so equal
          // to sentenceLen can find endToken.
          if (segGraph->isStartExist(next)) {
            nextTokens = segGraph->getStartList(next);
            break;
          }
          next++;
        }
        if (nextTokens.empty()) {
          break;
        }
        for (auto t2 : nextTokens) {
          idBuffer = std::deque<wchar_t>(t1->charArray.size() +
                                          t2->charArray.size() + 1);
          System::arraycopy(t1->charArray, 0, idBuffer, 0,
                            t1->charArray.size());
          idBuffer[t1->charArray.size()] = BigramDictionary::WORD_SEGMENT_CHAR;
          System::arraycopy(t2->charArray, 0, idBuffer,
                            t1->charArray.size() + 1, t2->charArray.size());

          // Two linked Words frequency
          wordPairFreq = bigramDict->getFrequency(idBuffer);

          // Smoothing

          // -log{a*P(Ci-1)+(1-a)P(Ci|Ci-1)} Note 0<a<1
          weight = -log(
              smooth * (1.0 + oneWordFreq) / (Utility::MAX_FREQUENCE + 0.0) +
              (1.0 - smooth) *
                  ((1.0 - tinyDouble) * wordPairFreq / (1.0 + oneWordFreq) +
                   tinyDouble));

          shared_ptr<SegTokenPair> tokenPair =
              make_shared<SegTokenPair>(idBuffer, t1->index, t2->index, weight);
          this->addSegTokenPair(tokenPair);
        }
      }
    }
    key++;
  }
}

bool BiSegGraph::isToExist(int to)
{
  return tokenPairListTable[static_cast<Integer>(to)].size() > 0;
}

deque<std::shared_ptr<SegTokenPair>> BiSegGraph::getToList(int to)
{
  return tokenPairListTable[to];
}

void BiSegGraph::addSegTokenPair(shared_ptr<SegTokenPair> tokenPair)
{
  int to = tokenPair->to;
  if (!isToExist(to)) {
    deque<std::shared_ptr<SegTokenPair>> newlist =
        deque<std::shared_ptr<SegTokenPair>>();
    newlist.push_back(tokenPair);
    tokenPairListTable.emplace(to, newlist);
  } else {
    deque<std::shared_ptr<SegTokenPair>> tokenPairList =
        tokenPairListTable[to];
    tokenPairList.push_back(tokenPair);
  }
}

int BiSegGraph::getToCount() { return tokenPairListTable.size(); }

deque<std::shared_ptr<SegToken>> BiSegGraph::getShortPath()
{
  int current;
  int nodeCount = getToCount();
  deque<std::shared_ptr<PathNode>> path = deque<std::shared_ptr<PathNode>>();
  shared_ptr<PathNode> zeroPath = make_shared<PathNode>();
  zeroPath->weight = 0;
  zeroPath->preNode = 0;
  path.push_back(zeroPath);
  for (current = 1; current <= nodeCount; current++) {
    double weight;
    deque<std::shared_ptr<SegTokenPair>> edges = getToList(current);

    double minWeight = numeric_limits<double>::max();
    shared_ptr<SegTokenPair> minEdge = nullptr;
    for (auto edge : edges) {
      weight = edge->weight;
      shared_ptr<PathNode> preNode = path[edge->from];
      if (preNode->weight + weight < minWeight) {
        minWeight = preNode->weight + weight;
        minEdge = edge;
      }
    }
    shared_ptr<PathNode> newNode = make_shared<PathNode>();
    newNode->weight = minWeight;
    newNode->preNode = minEdge->from;
    path.push_back(newNode);
  }

  // Calculate PathNodes
  int preNode, lastNode;
  lastNode = path.size() - 1;
  current = lastNode;
  deque<int> rpath = deque<int>();
  deque<std::shared_ptr<SegToken>> resultPath =
      deque<std::shared_ptr<SegToken>>();

  rpath.push_back(current);
  while (current != 0) {
    shared_ptr<PathNode> currentPathNode = path[current];
    preNode = currentPathNode->preNode;
    rpath.push_back(static_cast<Integer>(preNode));
    current = preNode;
  }
  for (int j = rpath.size() - 1; j >= 0; j--) {
    optional<int> idInteger = rpath[j];
    int id = idInteger.value();
    shared_ptr<SegToken> t = segTokenList[id];
    resultPath.push_back(t);
  }
  return resultPath;
}

wstring BiSegGraph::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  shared_ptr<deque<deque<std::shared_ptr<SegTokenPair>>>> values =
      tokenPairListTable.values();
  for (auto segList : values) {
    for (auto pair : segList) {
      sb->append(pair + L"\n");
    }
  }
  return sb->toString();
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm