using namespace std;

#include "GraphvizFormatter.h"
#include "KoreanTokenizer.h"
#include "dict/ConnectionCosts.h"
#include "dict/Dictionary.h"

namespace org::apache::lucene::analysis::ko
{
using Position = org::apache::lucene::analysis::ko::KoreanTokenizer::Position;
using WrappedPositionArray =
    org::apache::lucene::analysis::ko::KoreanTokenizer::WrappedPositionArray;
using ConnectionCosts =
    org::apache::lucene::analysis::ko::dict::ConnectionCosts;
using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;
const wstring GraphvizFormatter::BOS_LABEL = L"BOS";
const wstring GraphvizFormatter::EOS_LABEL = L"EOS";
const wstring GraphvizFormatter::FONT_NAME = L"Helvetica";

GraphvizFormatter::GraphvizFormatter(shared_ptr<ConnectionCosts> costs)
    : costs(costs), bestPathMap(unordered_map<>())
{
  sb->append(formatHeader());
  sb->append(L"  init [style=invis]\n");
  sb->append(L"  init -> 0.0 [label=\"" + BOS_LABEL + L"\"]\n");
}

wstring GraphvizFormatter::finish()
{
  sb->append(formatTrailer());
  return sb->toString();
}

void GraphvizFormatter::onBacktrace(shared_ptr<KoreanTokenizer> tok,
                                    shared_ptr<WrappedPositionArray> positions,
                                    int lastBackTracePos,
                                    shared_ptr<Position> endPosData,
                                    int fromIDX, std::deque<wchar_t> &fragment,
                                    bool isEnd)
{
  setBestPathMap(positions, lastBackTracePos, endPosData, fromIDX);
  sb->append(
      formatNodes(tok, positions, lastBackTracePos, endPosData, fragment));
  if (isEnd) {
    sb->append(L"  fini [style=invis]\n");
    sb->append(L"  ");
    sb->append(getNodeID(endPosData->pos, fromIDX));
    sb->append(L" -> fini [label=\"" + EOS_LABEL + L"\"]");
  }
}

void GraphvizFormatter::setBestPathMap(
    shared_ptr<WrappedPositionArray> positions, int startPos,
    shared_ptr<Position> endPosData, int fromIDX)
{
  bestPathMap.clear();

  int pos = endPosData->pos;
  int bestIDX = fromIDX;
  while (pos > startPos) {
    shared_ptr<Position> *const posData = positions->get(pos);

    constexpr int backPos = posData->backPos[bestIDX];
    constexpr int backIDX = posData->backIndex[bestIDX];

    const wstring toNodeID = getNodeID(pos, bestIDX);
    const wstring fromNodeID = getNodeID(backPos, backIDX);

    assert(bestPathMap.find(fromNodeID) == bestPathMap.end());
    assert(!bestPathMap.containsValue(toNodeID));
    bestPathMap.emplace(fromNodeID, toNodeID);
    pos = backPos;
    bestIDX = backIDX;
  }
}

wstring
GraphvizFormatter::formatNodes(shared_ptr<KoreanTokenizer> tok,
                               shared_ptr<WrappedPositionArray> positions,
                               int startPos, shared_ptr<Position> endPosData,
                               std::deque<wchar_t> &fragment)
{

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  // Output nodes
  for (int pos = startPos + 1; pos <= endPosData->pos; pos++) {
    shared_ptr<Position> *const posData = positions->get(pos);
    for (int idx = 0; idx < posData->count; idx++) {
      sb->append(L"  ");
      sb->append(getNodeID(pos, idx));
      sb->append(L" [label=\"");
      sb->append(pos);
      sb->append(L": ");
      sb->append(posData->lastRightID[idx]);
      sb->append(L"\"]\n");
    }
  }

  // Output arcs
  for (int pos = endPosData->pos; pos > startPos; pos--) {
    shared_ptr<Position> *const posData = positions->get(pos);
    for (int idx = 0; idx < posData->count; idx++) {
      shared_ptr<Position> *const backPosData =
          positions->get(posData->backPos[idx]);
      const wstring toNodeID = getNodeID(pos, idx);
      const wstring fromNodeID =
          getNodeID(posData->backPos[idx], posData->backIndex[idx]);

      sb->append(L"  ");
      sb->append(fromNodeID);
      sb->append(L" -> ");
      sb->append(toNodeID);

      const wstring attrs;
      if (toNodeID == bestPathMap[fromNodeID]) {
        // This arc is on best path
        attrs =
            L" color=\"#40e050\" fontcolor=\"#40a050\" penwidth=3 fontsize=20";
      } else {
        attrs = L"";
      }

      shared_ptr<Dictionary> *const dict = tok->getDict(posData->backType[idx]);
      constexpr int wordCost = dict->getWordCost(posData->backID[idx]);
      constexpr int bgCost =
          costs->get(backPosData->lastRightID[posData->backIndex[idx]],
                     dict->getLeftId(posData->backID[idx]));

      const wstring surfaceForm =
          wstring(fragment, posData->backPos[idx] - startPos,
                  pos - posData->backPos[idx]);

      sb->append(L" [label=\"");
      sb->append(surfaceForm);
      sb->append(L' ');
      sb->append(wordCost);
      if (bgCost >= 0) {
        sb->append(L'+');
      }
      sb->append(bgCost);
      sb->append(L"\"");
      sb->append(attrs);
      sb->append(L"]\n");
    }
  }
  return sb->toString();
}

wstring GraphvizFormatter::formatHeader()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"digraph viterbi {\n");
  sb->append(L"  graph [ fontsize=30 labelloc=\"t\" label=\"\" splines=true "
             L"overlap=false rankdir = \"LR\"];\n");
  // sb.append("  // A2 paper size\n");
  // sb.append("  size = \"34.4,16.5\";\n");
  // sb.append("  // try to fill paper\n");
  // sb.append("  ratio = fill;\n");
  sb->append(L"  edge [ fontname=\"" + FONT_NAME +
             L"\" fontcolor=\"red\" color=\"#606060\" ]\n");
  sb->append(L"  node [ style=\"filled\" fillcolor=\"#e8e8f0\" "
             L"shape=\"Mrecord\" fontname=\"" +
             FONT_NAME + L"\" ]\n");

  return sb->toString();
}

wstring GraphvizFormatter::formatTrailer() { return L"}"; }

wstring GraphvizFormatter::getNodeID(int pos, int idx)
{
  return to_wstring(pos) + L"." + to_wstring(idx);
}
} // namespace org::apache::lucene::analysis::ko