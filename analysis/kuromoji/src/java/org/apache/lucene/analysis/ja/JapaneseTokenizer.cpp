using namespace std;

#include "JapaneseTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/RollingCharBuffer.h"
#include "GraphvizFormatter.h"
#include "Token.h"
#include "dict/CharacterDefinition.h"
#include "dict/ConnectionCosts.h"
#include "dict/Dictionary.h"
#include "dict/TokenInfoDictionary.h"
#include "dict/TokenInfoFST.h"
#include "dict/UnknownDictionary.h"
#include "dict/UserDictionary.h"

namespace org::apache::lucene::analysis::ja
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharacterDefinition =
    org::apache::lucene::analysis::ja::dict::CharacterDefinition;
using ConnectionCosts =
    org::apache::lucene::analysis::ja::dict::ConnectionCosts;
using Dictionary = org::apache::lucene::analysis::ja::dict::Dictionary;
using TokenInfoDictionary =
    org::apache::lucene::analysis::ja::dict::TokenInfoDictionary;
using TokenInfoFST = org::apache::lucene::analysis::ja::dict::TokenInfoFST;
using UnknownDictionary =
    org::apache::lucene::analysis::ja::dict::UnknownDictionary;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;
using namespace org::apache::lucene::analysis::ja::tokenattributes;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using RollingCharBuffer =
    org::apache::lucene::analysis::util::RollingCharBuffer;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using IntsRef = org::apache::lucene::util::IntsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using FST = org::apache::lucene::util::fst::FST;

JapaneseTokenizer::JapaneseTokenizer(shared_ptr<UserDictionary> userDictionary,
                                     bool discardPunctuation, Mode mode)
    : JapaneseTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, userDictionary,
                        discardPunctuation, mode)
{
}

JapaneseTokenizer::JapaneseTokenizer(shared_ptr<AttributeFactory> factory,
                                     shared_ptr<UserDictionary> userDictionary,
                                     bool discardPunctuation, Mode mode)
    : org::apache::lucene::analysis::Tokenizer(factory),
      fst(dictionary->getFST()), dictionary(TokenInfoDictionary::getInstance()),
      unkDictionary(UnknownDictionary::getInstance()),
      costs(ConnectionCosts::getInstance()), userDictionary(userDictionary),
      characterDefinition(unkDictionary->getCharacterDefinition()),
      fstReader(fst->getBytesReader()), discardPunctuation(discardPunctuation)
{
  if (userDictionary != nullptr) {
    userFST = userDictionary->getFST();
    userFSTReader = userFST->getBytesReader();
  } else {
    userFST.reset();
    userFSTReader.reset();
  }
  switch (mode) {
  case org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode::SEARCH:
    searchMode = true;
    extendedMode = false;
    outputCompounds = true;
    break;
  case org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode::EXTENDED:
    searchMode = true;
    extendedMode = true;
    outputCompounds = false;
    break;
  default:
    searchMode = false;
    extendedMode = false;
    outputCompounds = false;
    break;
  }
  buffer->reset(this->input);

  resetState();

  dictionaryMap->put(Type::KNOWN, dictionary);
  dictionaryMap->put(Type::UNKNOWN, unkDictionary);
  dictionaryMap->put(Type::USER, userDictionary);
}

void JapaneseTokenizer::setGraphvizFormatter(
    shared_ptr<GraphvizFormatter> dotOut)
{
  this->dotOut = dotOut;
}

JapaneseTokenizer::~JapaneseTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  buffer->reset(input);
}

void JapaneseTokenizer::reset() 
{
  Tokenizer::reset();
  buffer->reset(input);
  resetState();
}

void JapaneseTokenizer::resetState()
{
  positions->reset();
  pos = 0;
  end_ = false;
  lastBackTracePos = 0;
  lastTokenPos = -1;
  pending.clear();

  // Add BOS:
  positions->get(0)->add(0, 0, -1, -1, -1, Type::KNOWN);
}

void JapaneseTokenizer::end() 
{
  Tokenizer::end();
  // Set final offset
  int finalOffset = correctOffset(pos);
  offsetAtt->setOffset(finalOffset, finalOffset);
}

int JapaneseTokenizer::computeSecondBestThreshold(int pos,
                                                  int length) 
{
  // TODO: maybe we do something else here, instead of just
  // using the penalty...?  EG we can be more aggressive on
  // when to also test for 2nd best path
  return computePenalty(pos, length);
}

int JapaneseTokenizer::computePenalty(int pos, int length) 
{
  if (length > SEARCH_MODE_KANJI_LENGTH) {
    bool allKanji = true;
    // check if node consists of only kanji
    constexpr int endPos = pos + length;
    for (int pos2 = pos; pos2 < endPos; pos2++) {
      if (!characterDefinition->isKanji(
              static_cast<wchar_t>(buffer->get(pos2)))) {
        allKanji = false;
        break;
      }
    }
    if (allKanji) { // Process only Kanji keywords
      return (length - SEARCH_MODE_KANJI_LENGTH) * SEARCH_MODE_KANJI_PENALTY;
    } else if (length > SEARCH_MODE_OTHER_LENGTH) {
      return (length - SEARCH_MODE_OTHER_LENGTH) * SEARCH_MODE_OTHER_PENALTY;
    }
  }
  return 0;
}

void JapaneseTokenizer::Position::grow()
{
  costs = ArrayUtil::grow(costs, 1 + count);
  lastRightID = ArrayUtil::grow(lastRightID, 1 + count);
  backPos = ArrayUtil::grow(backPos, 1 + count);
  backIndex = ArrayUtil::grow(backIndex, 1 + count);
  backID = ArrayUtil::grow(backID, 1 + count);

  // NOTE: sneaky: grow separately because
  // ArrayUtil.grow will otherwise pick a different
  // length than the int[]s we just grew:
  const std::deque<Type> newBackType = std::deque<Type>(backID.size());
  System::arraycopy(backType, 0, newBackType, 0, backType.size());
  backType = newBackType;
}

void JapaneseTokenizer::Position::growForward()
{
  forwardPos = ArrayUtil::grow(forwardPos, 1 + forwardCount);
  forwardID = ArrayUtil::grow(forwardID, 1 + forwardCount);
  forwardIndex = ArrayUtil::grow(forwardIndex, 1 + forwardCount);

  // NOTE: sneaky: grow separately because
  // ArrayUtil.grow will otherwise pick a different
  // length than the int[]s we just grew:
  const std::deque<Type> newForwardType = std::deque<Type>(forwardPos.size());
  System::arraycopy(forwardType, 0, newForwardType, 0, forwardType.size());
  forwardType = newForwardType;
}

void JapaneseTokenizer::Position::add(int cost, int lastRightID, int backPos,
                                      int backIndex, int backID, Type backType)
{
  // NOTE: this isn't quite a true Viterbi search,
  // because we should check if lastRightID is
  // already present here, and only update if the new
  // cost is less than the current cost, instead of
  // simply appending.  However, that will likely hurt
  // performance (usually we add a lastRightID only once),
  // and it means we actually create the full graph
  // intersection instead of a "normal" Viterbi lattice:
  if (count == costs.size()) {
    grow();
  }
  this->costs[count] = cost;
  this->lastRightID[count] = lastRightID;
  this->backPos[count] = backPos;
  this->backIndex[count] = backIndex;
  this->backID[count] = backID;
  this->backType[count] = backType;
  count++;
}

void JapaneseTokenizer::Position::addForward(int forwardPos, int forwardIndex,
                                             int forwardID, Type forwardType)
{
  if (forwardCount == this->forwardID.size()) {
    growForward();
  }
  this->forwardPos[forwardCount] = forwardPos;
  this->forwardIndex[forwardCount] = forwardIndex;
  this->forwardID[forwardCount] = forwardID;
  this->forwardType[forwardCount] = forwardType;
  forwardCount++;
}

void JapaneseTokenizer::Position::reset()
{
  count = 0;
  // forwardCount naturally resets after it runs:
  assert((forwardCount == 0, L"pos=" + to_wstring(pos) + L" forwardCount=" +
                                 to_wstring(forwardCount)));
}

void JapaneseTokenizer::add(shared_ptr<Dictionary> dict,
                            shared_ptr<Position> fromPosData, int endPos,
                            int wordID, Type type,
                            bool addPenalty) 
{
  constexpr int wordCost = dict->getWordCost(wordID);
  constexpr int leftID = dict->getLeftId(wordID);
  int leastCost = numeric_limits<int>::max();
  int leastIDX = -1;
  assert(fromPosData->count > 0);
  for (int idx = 0; idx < fromPosData->count; idx++) {
    // Cost is path cost so far, plus word cost (added at
    // end of loop), plus bigram cost:
    constexpr int cost = fromPosData->costs[idx] +
                         costs->get(fromPosData->lastRightID[idx], leftID);
    if (VERBOSE) {
      wcout << L"      fromIDX=" << idx << L": cost=" << cost << L" (prevCost="
            << fromPosData->costs[idx] << L" wordCost=" << wordCost
            << L" bgCost=" << costs->get(fromPosData->lastRightID[idx], leftID)
            << L" leftID=" << leftID << L")" << endl;
    }
    if (cost < leastCost) {
      leastCost = cost;
      leastIDX = idx;
      if (VERBOSE) {
        wcout << L"        **" << endl;
      }
    }
  }

  leastCost += wordCost;

  if (VERBOSE) {
    wcout << L"      + cost=" << leastCost << L" wordID=" << wordID
          << L" leftID=" << leftID << L" leastIDX=" << leastIDX << L" toPos="
          << endPos << L" toPos.idx=" << positions->get(endPos).count << endl;
  }

  if ((addPenalty || (!outputCompounds && searchMode)) && type != Type::USER) {
    constexpr int penalty =
        computePenalty(fromPosData->pos, endPos - fromPosData->pos);
    if (VERBOSE) {
      if (penalty > 0) {
        wcout << L"        + penalty=" << penalty << L" cost="
              << (leastCost << penalty) << endl;
      }
    }
    leastCost += penalty;
  }

  // positions.get(endPos).add(leastCost, dict.getRightId(wordID),
  // fromPosData.pos, leastIDX, wordID, type);
  assert(leftID == dict->getRightId(wordID));
  positions->get(endPos)->add(leastCost, leftID, fromPosData->pos, leastIDX,
                              wordID, type);
}

bool JapaneseTokenizer::incrementToken() 
{

  // parse() is able to return w/o producing any new
  // tokens, when the tokens it had produced were entirely
  // punctuation.  So we loop here until we get a real
  // token or we end:
  while (pending.empty()) {
    if (end_) {
      return false;
    }

    // Push Viterbi forward some more:
    parse();
  }

  shared_ptr<Token> *const token = pending.pop_back();

  int position = token->getPosition();
  int length = token->getLength();
  clearAttributes();
  assert(length > 0);
  // System.out.println("off=" + token.getOffset() + " len=" + length + " vs " +
  // token.getSurfaceForm().length);
  termAtt->copyBuffer(token->getSurfaceForm(), token->getOffset(), length);
  offsetAtt->setOffset(correctOffset(position),
                       correctOffset(position + length));
  basicFormAtt->setToken(token);
  posAtt->setToken(token);
  readingAtt->setToken(token);
  inflectionAtt->setToken(token);
  if (token->getPosition() == lastTokenPos) {
    posIncAtt->setPositionIncrement(0);
    posLengthAtt->setPositionLength(token->getPositionLength());
  } else if (outputNBest) {
    // The position length is always calculated if outputNBest is true.
    assert(token->getPosition() > lastTokenPos);
    posIncAtt->setPositionIncrement(1);
    posLengthAtt->setPositionLength(token->getPositionLength());
  } else {
    assert(token->getPosition() > lastTokenPos);
    posIncAtt->setPositionIncrement(1);
    posLengthAtt->setPositionLength(1);
  }
  if (VERBOSE) {
    wcout << Thread::currentThread().getName()
          << L":    incToken: return token=" << token << endl;
  }
  lastTokenPos = token->getPosition();
  return true;
}

JapaneseTokenizer::WrappedPositionArray::WrappedPositionArray()
{
  for (int i = 0; i < positions.size(); i++) {
    positions[i] = make_shared<Position>();
  }
}

void JapaneseTokenizer::WrappedPositionArray::reset()
{
  nextWrite--;
  while (count > 0) {
    if (nextWrite == -1) {
      nextWrite = positions.size() - 1;
    }
    positions[nextWrite--]->reset();
    count--;
  }
  nextWrite = 0;
  nextPos = 0;
  count = 0;
}

shared_ptr<Position> JapaneseTokenizer::WrappedPositionArray::get(int pos)
{
  while (pos >= nextPos) {
    // System.out.println("count=" + count + " vs len=" + positions.length);
    if (count == positions.size()) {
      std::deque<std::shared_ptr<Position>> newPositions(ArrayUtil::oversize(
          1 + count, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
      // System.out.println("grow positions " + newPositions.length);
      System::arraycopy(positions, nextWrite, newPositions, 0,
                        positions.size() - nextWrite);
      System::arraycopy(positions, 0, newPositions,
                        positions.size() - nextWrite, nextWrite);
      for (int i = positions.size(); i < newPositions.size(); i++) {
        newPositions[i] = make_shared<Position>();
      }
      nextWrite = positions.size();
      positions = newPositions;
    }
    if (nextWrite == positions.size()) {
      nextWrite = 0;
    }
    // Should have already been reset:
    assert(positions[nextWrite]->count == 0);
    positions[nextWrite++]->pos = nextPos++;
    count++;
  }
  assert(inBounds(pos));
  constexpr int index = getIndex(pos);
  assert(positions[index]->pos == pos);
  return positions[index];
}

int JapaneseTokenizer::WrappedPositionArray::getNextPos() { return nextPos; }

bool JapaneseTokenizer::WrappedPositionArray::inBounds(int pos)
{
  return pos < nextPos && pos >= nextPos - count;
}

int JapaneseTokenizer::WrappedPositionArray::getIndex(int pos)
{
  int index = nextWrite - (nextPos - pos);
  if (index < 0) {
    index += positions.size();
  }
  return index;
}

void JapaneseTokenizer::WrappedPositionArray::freeBefore(int pos)
{
  constexpr int toFree = count - (nextPos - pos);
  assert(toFree >= 0);
  assert(toFree <= count);
  int index = nextWrite - count;
  if (index < 0) {
    index += positions.size();
  }
  for (int i = 0; i < toFree; i++) {
    if (index == positions.size()) {
      index = 0;
    }
    // System.out.println("  fb idx=" + index);
    positions[index]->reset();
    index++;
  }
  count -= toFree;
}

void JapaneseTokenizer::parse() 
{
  if (VERBOSE) {
    wcout << L"\nPARSE" << endl;
  }

  // Index of the last character of unknown word:
  int unknownWordEndIndex = -1;

  // Advances over each position (character):
  while (true) {

    if (buffer->get(pos) == -1) {
      // End
      break;
    }

    shared_ptr<Position> *const posData = positions->get(pos);
    constexpr bool isFrontier = positions->getNextPos() == pos + 1;

    if (posData->count == 0) {
      // No arcs arrive here; move to next position:
      if (VERBOSE) {
        wcout << L"    no arcs in; skip pos=" << pos << endl;
      }
      pos++;
      continue;
    }

    if (pos > lastBackTracePos && posData->count == 1 && isFrontier) {
      //  if (pos > lastBackTracePos && posData.count == 1 && isFrontier) {
      // We are at a "frontier", and only one node is
      // alive, so whatever the eventual best path is must
      // come through this node.  So we can safely commit
      // to the prefix of the best path at this point:
      if (outputNBest) {
        backtraceNBest(posData, false);
      }
      backtrace(posData, 0);
      if (outputNBest) {
        fixupPendingList();
      }

      // Re-base cost so we don't risk int overflow:
      posData->costs[0] = 0;

      if (pending.size() != 0) {
        return;
      } else {
        // This means the backtrace only produced
        // punctuation tokens, so we must keep parsing.
      }
    }

    if (pos - lastBackTracePos >= MAX_BACKTRACE_GAP) {
      // Safety: if we've buffered too much, force a
      // backtrace now.  We find the least-cost partial
      // path, across all paths, backtrace from it, and
      // then prune all others.  Note that this, in
      // general, can produce the wrong result, if the
      // total best path did not in fact back trace
      // through this partial best path.  But it's the
      // best we can do... (short of not having a
      // safety!).

      // First pass: find least cost partial path so far,
      // including ending at future positions:
      int leastIDX = -1;
      int leastCost = numeric_limits<int>::max();
      shared_ptr<Position> leastPosData = nullptr;
      for (int pos2 = pos; pos2 < positions->getNextPos(); pos2++) {
        shared_ptr<Position> *const posData2 = positions->get(pos2);
        for (int idx = 0; idx < posData2->count; idx++) {
          // System.out.println("    idx=" + idx + " cost=" + cost);
          constexpr int cost = posData2->costs[idx];
          if (cost < leastCost) {
            leastCost = cost;
            leastIDX = idx;
            leastPosData = posData2;
          }
        }
      }

      // We will always have at least one live path:
      assert(leastIDX != -1);

      if (outputNBest) {
        backtraceNBest(leastPosData, false);
      }

      // Second pass: prune all but the best path:
      for (int pos2 = pos; pos2 < positions->getNextPos(); pos2++) {
        shared_ptr<Position> *const posData2 = positions->get(pos2);
        if (posData2 != leastPosData) {
          posData2->reset();
        } else {
          if (leastIDX != 0) {
            posData2->costs[0] = posData2->costs[leastIDX];
            posData2->lastRightID[0] = posData2->lastRightID[leastIDX];
            posData2->backPos[0] = posData2->backPos[leastIDX];
            posData2->backIndex[0] = posData2->backIndex[leastIDX];
            posData2->backID[0] = posData2->backID[leastIDX];
            posData2->backType[0] = posData2->backType[leastIDX];
          }
          posData2->count = 1;
        }
      }

      backtrace(leastPosData, 0);
      if (outputNBest) {
        fixupPendingList();
      }

      // Re-base cost so we don't risk int overflow:
      Arrays::fill(leastPosData->costs, 0, leastPosData->count, 0);

      if (pos != leastPosData->pos) {
        // We jumped into a future position:
        assert(pos < leastPosData->pos);
        pos = leastPosData->pos;
      }

      if (pending.size() != 0) {
        return;
      } else {
        // This means the backtrace only produced
        // punctuation tokens, so we must keep parsing.
        continue;
      }
    }

    if (VERBOSE) {
      wcout << L"\n  extend @ pos=" << pos << L" char="
            << static_cast<wchar_t>(buffer->get(pos)) << L" hex="
            << Integer::toHexString(buffer->get(pos)) << endl;
    }

    if (VERBOSE) {
      wcout << L"    " << posData->count << L" arcs in" << endl;
    }

    bool anyMatches = false;

    // First try user dict:
    if (userFST != nullptr) {
      userFST->getFirstArc(arc);
      int output = 0;
      for (int posAhead = posData->pos;; posAhead++) {
        constexpr int ch = buffer->get(posAhead);
        if (ch == -1) {
          break;
        }
        if (userFST->findTargetArc(ch, arc, arc, posAhead == posData->pos,
                                   userFSTReader) == nullptr) {
          break;
        }
        output += arc->output.intValue();
        if (arc->isFinal()) {
          if (VERBOSE) {
            wcout << L"    USER word "
                  << wstring(buffer->get(pos, posAhead - pos << 1))
                  << L" toPos=" << (posAhead << 1) << endl;
          }
          add(userDictionary, posData, posAhead + 1,
              output + arc->nextFinalOutput.intValue(), Type::USER, false);
          anyMatches = true;
        }
      }
    }

    // TODO: we can be more aggressive about user
    // matches?  if we are "under" a user match then don't
    // extend KNOWN/UNKNOWN paths?

    if (!anyMatches) {
      // Next, try known dictionary matches
      fst->getFirstArc(arc);
      int output = 0;

      for (int posAhead = posData->pos;; posAhead++) {
        constexpr int ch = buffer->get(posAhead);
        if (ch == -1) {
          break;
        }
        // System.out.println("    match " + (char) ch + " posAhead=" +
        // posAhead);

        if (fst->findTargetArc(ch, arc, arc, posAhead == posData->pos,
                               fstReader) == nullptr) {
          break;
        }

        output += arc->output.intValue();

        // Optimization: for known words that are too-long
        // (compound), we should pre-compute the 2nd
        // best segmentation and store it in the
        // dictionary instead of recomputing it each time a
        // match is found.

        if (arc->isFinal()) {
          dictionary->lookupWordIds(output + arc->nextFinalOutput.intValue(),
                                    wordIdRef);
          if (VERBOSE) {
            wcout << L"    KNOWN word "
                  << wstring(buffer->get(pos, posAhead - pos << 1))
                  << L" toPos=" << (posAhead << 1) << L" " << wordIdRef->length
                  << L" wordIDs" << endl;
          }
          for (int ofs = 0; ofs < wordIdRef->length; ofs++) {
            add(dictionary, posData, posAhead + 1,
                wordIdRef->ints[wordIdRef->offset + ofs], Type::KNOWN, false);
            anyMatches = true;
          }
        }
      }
    }

    // In the case of normal mode, it doesn't process unknown word greedily.

    if (!searchMode && unknownWordEndIndex > posData->pos) {
      pos++;
      continue;
    }

    constexpr wchar_t firstCharacter = static_cast<wchar_t>(buffer->get(pos));
    if (!anyMatches || characterDefinition->isInvoke(firstCharacter)) {

      // Find unknown match:
      constexpr int characterId =
          characterDefinition->getCharacterClass(firstCharacter);
      constexpr bool isPunct = isPunctuation(firstCharacter);

      // NOTE: copied from UnknownDictionary.lookup:
      int unknownWordLength;
      if (!characterDefinition->isGroup(firstCharacter)) {
        unknownWordLength = 1;
      } else {
        // Extract unknown word. Characters with the same character class are
        // considered to be part of unknown word
        unknownWordLength = 1;
        for (int posAhead = pos + 1;
             unknownWordLength < MAX_UNKNOWN_WORD_LENGTH; posAhead++) {
          constexpr int ch = buffer->get(posAhead);
          if (ch == -1) {
            break;
          }
          if (characterId == characterDefinition->getCharacterClass(
                                 static_cast<wchar_t>(ch)) &&
              isPunctuation(static_cast<wchar_t>(ch)) == isPunct) {
            unknownWordLength++;
          } else {
            break;
          }
        }
      }

      unkDictionary->lookupWordIds(
          characterId,
          wordIdRef); // characters in input text are supposed to be the same
      if (VERBOSE) {
        wcout << L"    UNKNOWN word len=" << unknownWordLength << L" "
              << wordIdRef->length << L" wordIDs" << endl;
      }
      for (int ofs = 0; ofs < wordIdRef->length; ofs++) {
        add(unkDictionary, posData, posData->pos + unknownWordLength,
            wordIdRef->ints[wordIdRef->offset + ofs], Type::UNKNOWN, false);
      }

      unknownWordEndIndex = posData->pos + unknownWordLength;
    }

    pos++;
  }

  end_ = true;

  if (pos > 0) {

    shared_ptr<Position> *const endPosData = positions->get(pos);
    int leastCost = numeric_limits<int>::max();
    int leastIDX = -1;
    if (VERBOSE) {
      wcout << L"  end: " << endPosData->count << L" nodes" << endl;
    }
    for (int idx = 0; idx < endPosData->count; idx++) {
      // Add EOS cost:
      constexpr int cost =
          endPosData->costs[idx] + costs->get(endPosData->lastRightID[idx], 0);
      // System.out.println("    idx=" + idx + " cost=" + cost + " (pathCost=" +
      // endPosData.costs[idx] + " bgCost=" +
      // costs.get(endPosData.lastRightID[idx], 0) + ") backPos=" +
      // endPosData.backPos[idx]);
      if (cost < leastCost) {
        leastCost = cost;
        leastIDX = idx;
      }
    }

    if (outputNBest) {
      backtraceNBest(endPosData, true);
    }
    backtrace(endPosData, leastIDX);
    if (outputNBest) {
      fixupPendingList();
    }
  } else {
    // No characters in the input string; return no tokens!
  }
}

void JapaneseTokenizer::pruneAndRescore(int startPos, int endPos,
                                        int bestStartIDX) 
{
  if (VERBOSE) {
    wcout << L"  pruneAndRescore startPos=" << startPos << L" endPos=" << endPos
          << L" bestStartIDX=" << bestStartIDX << endl;
  }

  // First pass: walk backwards, building up the forward
  // arcs and pruning inadmissible arcs:
  for (int pos = endPos; pos > startPos; pos--) {
    shared_ptr<Position> *const posData = positions->get(pos);
    if (VERBOSE) {
      wcout << L"    back pos=" << pos << endl;
    }
    for (int arcIDX = 0; arcIDX < posData->count; arcIDX++) {
      constexpr int backPos = posData->backPos[arcIDX];
      if (backPos >= startPos) {
        // Keep this arc:
        // System.out.println("      keep backPos=" + backPos);
        positions->get(backPos).addForward(pos, arcIDX, posData->backID[arcIDX],
                                           posData->backType[arcIDX]);
      } else {
        if (VERBOSE) {
          wcout << L"      prune" << endl;
        }
      }
    }
    if (pos != startPos) {
      posData->count = 0;
    }
  }

  // Second pass: walk forward, re-scoring:
  for (int pos = startPos; pos < endPos; pos++) {
    shared_ptr<Position> *const posData = positions->get(pos);
    if (VERBOSE) {
      wcout << L"    forward pos=" << pos << L" count=" << posData->forwardCount
            << endl;
    }
    if (posData->count == 0) {
      // No arcs arrive here...
      if (VERBOSE) {
        wcout << L"      skip" << endl;
      }
      posData->forwardCount = 0;
      continue;
    }

    if (pos == startPos) {
      // On the initial position, only consider the best
      // path so we "force congruence":  the
      // sub-segmentation is "in context" of what the best
      // path (compound token) had matched:
      constexpr int rightID;
      if (startPos == 0) {
        rightID = 0;
      } else {
        rightID = getDict(posData->backType[bestStartIDX])
                      ->getRightId(posData->backID[bestStartIDX]);
      }
      constexpr int pathCost = posData->costs[bestStartIDX];
      for (int forwardArcIDX = 0; forwardArcIDX < posData->forwardCount;
           forwardArcIDX++) {
        constexpr Type forwardType = posData->forwardType[forwardArcIDX];
        shared_ptr<Dictionary> *const dict2 = getDict(forwardType);
        constexpr int wordID = posData->forwardID[forwardArcIDX];
        constexpr int toPos = posData->forwardPos[forwardArcIDX];
        constexpr int newCost = pathCost + dict2->getWordCost(wordID) +
                                costs->get(rightID, dict2->getLeftId(wordID)) +
                                computePenalty(pos, toPos - pos);
        if (VERBOSE) {
          wcout << L"      + " << forwardType << L" word "
                << wstring(buffer->get(pos, toPos - pos)) << L" toPos=" << toPos
                << L" cost=" << newCost << L" penalty="
                << computePenalty(pos, toPos - pos) << L" toPos.idx="
                << positions->get(toPos).count << endl;
        }
        positions->get(toPos)->add(newCost, dict2->getRightId(wordID), pos,
                                   bestStartIDX, wordID, forwardType);
      }
    } else {
      // On non-initial positions, we maximize score
      // across all arriving lastRightIDs:
      for (int forwardArcIDX = 0; forwardArcIDX < posData->forwardCount;
           forwardArcIDX++) {
        constexpr Type forwardType = posData->forwardType[forwardArcIDX];
        constexpr int toPos = posData->forwardPos[forwardArcIDX];
        if (VERBOSE) {
          wcout << L"      + " << forwardType << L" word "
                << wstring(buffer->get(pos, toPos - pos)) << L" toPos=" << toPos
                << endl;
        }
        add(getDict(forwardType), posData, toPos,
            posData->forwardID[forwardArcIDX], forwardType, true);
      }
    }
    posData->forwardCount = 0;
  }
}

void JapaneseTokenizer::Lattice::setupRoot(int baseOffset, int lastOffset)
{
  assert(baseOffset <= lastOffset);
  int size = lastOffset - baseOffset + 1;
  if (rootCapacity < size) {
    int oversize = ArrayUtil::oversize(size, Integer::BYTES);
    lRoot = std::deque<int>(oversize);
    rRoot = std::deque<int>(oversize);
    rootCapacity = oversize;
  }
  Arrays::fill(lRoot, 0, size, -1);
  Arrays::fill(rRoot, 0, size, -1);
  rootSize = size;
  rootBase = baseOffset;
}

void JapaneseTokenizer::Lattice::reserve(int n)
{
  if (capacity < n) {
    int oversize = ArrayUtil::oversize(n, Integer::BYTES);
    nodeDicType = std::deque<Type>(oversize);
    nodeWordID = std::deque<int>(oversize);
    nodeMark = std::deque<int>(oversize);
    nodeLeftID = std::deque<int>(oversize);
    nodeRightID = std::deque<int>(oversize);
    nodeWordCost = std::deque<int>(oversize);
    nodeLeftCost = std::deque<int>(oversize);
    nodeRightCost = std::deque<int>(oversize);
    nodeLeftNode = std::deque<int>(oversize);
    nodeRightNode = std::deque<int>(oversize);
    nodeLeft = std::deque<int>(oversize);
    nodeRight = std::deque<int>(oversize);
    nodeLeftChain = std::deque<int>(oversize);
    nodeRightChain = std::deque<int>(oversize);
    capacity = oversize;
  }
}

void JapaneseTokenizer::Lattice::setupNodePool(int n)
{
  reserve(n);
  nodeCount = 0;
  if (VERBOSE) {
    printf(L"DEBUG: setupNodePool: n = %d\n", n);
    printf(L"DEBUG: setupNodePool: lattice.capacity = %d\n", capacity);
  }
}

int JapaneseTokenizer::Lattice::addNode(Type dicType, int wordID, int left,
                                        int right)
{
  if (VERBOSE) {
    printf(
        L"DEBUG: addNode: dicType=%s, wordID=%d, left=%d, right=%d, str=%s\n",
        dicType.toString(), wordID, left, right,
        left == -1
            ? L"BOS"
            : right == -1 ? L"EOS" : wstring(fragment, left, right - left));
  }
  assert(nodeCount < capacity);
  assert(left == -1 || right == -1 || left < right);
  assert(left == -1 || (0 <= left && left < rootSize));
  assert(right == -1 || (0 <= right && right < rootSize));

  int node = nodeCount++;

  if (VERBOSE) {
    printf(L"DEBUG: addNode: node=%d\n", node);
  }

  nodeDicType[node] = dicType;
  nodeWordID[node] = wordID;
  nodeMark[node] = 0;

  if (wordID < 0) {
    nodeWordCost[node] = 0;
    nodeLeftCost[node] = 0;
    nodeRightCost[node] = 0;
    nodeLeftID[node] = 0;
    nodeRightID[node] = 0;
  } else {
    shared_ptr<Dictionary> dic = dictionaryMap->get(dicType);
    nodeWordCost[node] = dic->getWordCost(wordID);
    nodeLeftID[node] = dic->getLeftId(wordID);
    nodeRightID[node] = dic->getRightId(wordID);
  }

  if (VERBOSE) {
    printf(L"DEBUG: addNode: wordCost=%d, leftID=%d, rightID=%d\n",
           nodeWordCost[node], nodeLeftID[node], nodeRightID[node]);
  }

  nodeLeft[node] = left;
  nodeRight[node] = right;
  if (0 <= left) {
    nodeLeftChain[node] = lRoot[left];
    lRoot[left] = node;
  } else {
    nodeLeftChain[node] = -1;
  }
  if (0 <= right) {
    nodeRightChain[node] = rRoot[right];
    rRoot[right] = node;
  } else {
    nodeRightChain[node] = -1;
  }
  return node;
}

int JapaneseTokenizer::Lattice::positionCount(
    shared_ptr<WrappedPositionArray> positions, int beg, int end)
{
  int count = 0;
  for (int i = beg; i < end; ++i) {
    count += positions->get(i).count;
  }
  return count;
}

void JapaneseTokenizer::Lattice::setup(
    std::deque<wchar_t> &fragment,
    shared_ptr<EnumMap<Type, std::shared_ptr<Dictionary>>> dictionaryMap,
    shared_ptr<WrappedPositionArray> positions, int prevOffset, int endOffset,
    bool useEOS)
{
  assert(positions->get(prevOffset)->count == 1);
  if (VERBOSE) {
    printf(L"DEBUG: setup: prevOffset=%d, endOffset=%d\n", prevOffset,
           endOffset);
  }

  this->fragment = fragment;
  this->dictionaryMap = dictionaryMap;
  this->useEOS = useEOS;

  // Initialize lRoot and rRoot.
  setupRoot(prevOffset, endOffset);

  // "+ 2" for first/last record.
  setupNodePool(positionCount(positions, prevOffset + 1, endOffset + 1) + 2);

  // substitute for BOS = 0
  shared_ptr<Position> first = positions->get(prevOffset);
  if (addNode(first->backType[0], first->backID[0], -1, 0) != 0) {
    assert(false);
  }

  // EOS = 1
  if (addNode(Type::KNOWN, -1, endOffset - rootBase, -1) != 1) {
    assert(false);
  }

  for (int offset = endOffset; prevOffset < offset; --offset) {
    int right = offset - rootBase;
    // optimize: exclude disconnected nodes.
    if (0 <= lRoot[right]) {
      shared_ptr<Position> pos = positions->get(offset);
      for (int i = 0; i < pos->count; ++i) {
        addNode(pos->backType[i], pos->backID[i], pos->backPos[i] - rootBase,
                right);
      }
    }
  }
}

void JapaneseTokenizer::Lattice::markUnreachable()
{
  for (int index = 1; index < rootSize - 1; ++index) {
    if (rRoot[index] < 0) {
      for (int node = lRoot[index]; 0 <= node; node = nodeLeftChain[node]) {
        if (VERBOSE) {
          printf(L"DEBUG: markUnreachable: node=%d\n", node);
        }
        nodeMark[node] = -1;
      }
    }
  }
}

int JapaneseTokenizer::Lattice::connectionCost(
    shared_ptr<ConnectionCosts> costs, int left, int right)
{
  int leftID = nodeLeftID[right];
  return ((leftID == 0 && !useEOS) ? 0 : costs->get(nodeRightID[left], leftID));
}

void JapaneseTokenizer::Lattice::calcLeftCost(shared_ptr<ConnectionCosts> costs)
{
  for (int index = 0; index < rootSize; ++index) {
    for (int node = lRoot[index]; 0 <= node; node = nodeLeftChain[node]) {
      if (0 <= nodeMark[node]) {
        int leastNode = -1;
        int leastCost = numeric_limits<int>::max();
        for (int leftNode = rRoot[index]; 0 <= leftNode;
             leftNode = nodeRightChain[leftNode]) {
          if (0 <= nodeMark[leftNode]) {
            int cost = nodeLeftCost[leftNode] + nodeWordCost[leftNode] +
                       connectionCost(costs, leftNode, node);
            if (cost < leastCost) {
              leastCost = cost;
              leastNode = leftNode;
            }
          }
        }
        assert(0 <= leastNode);
        nodeLeftNode[node] = leastNode;
        nodeLeftCost[node] = leastCost;
        if (VERBOSE) {
          printf(L"DEBUG: calcLeftCost: node=%d, leftNode=%d, leftCost=%d\n",
                 node, nodeLeftNode[node], nodeLeftCost[node]);
        }
      }
    }
  }
}

void JapaneseTokenizer::Lattice::calcRightCost(
    shared_ptr<ConnectionCosts> costs)
{
  for (int index = rootSize - 1; 0 <= index; --index) {
    for (int node = rRoot[index]; 0 <= node; node = nodeRightChain[node]) {
      if (0 <= nodeMark[node]) {
        int leastNode = -1;
        int leastCost = numeric_limits<int>::max();
        for (int rightNode = lRoot[index]; 0 <= rightNode;
             rightNode = nodeLeftChain[rightNode]) {
          if (0 <= nodeMark[rightNode]) {
            int cost = nodeRightCost[rightNode] + nodeWordCost[rightNode] +
                       connectionCost(costs, node, rightNode);
            if (cost < leastCost) {
              leastCost = cost;
              leastNode = rightNode;
            }
          }
        }
        assert(0 <= leastNode);
        nodeRightNode[node] = leastNode;
        nodeRightCost[node] = leastCost;
        if (VERBOSE) {
          printf(L"DEBUG: calcRightCost: node=%d, rightNode=%d, rightCost=%d\n",
                 node, nodeRightNode[node], nodeRightCost[node]);
        }
      }
    }
  }
}

void JapaneseTokenizer::Lattice::markSameSpanNode(int refNode, int value)
{
  int left = nodeLeft[refNode];
  int right = nodeRight[refNode];
  for (int node = lRoot[left]; 0 <= node; node = nodeLeftChain[node]) {
    if (nodeRight[node] == right) {
      nodeMark[node] = value;
    }
  }
}

deque<int> JapaneseTokenizer::Lattice::bestPathNodeList()
{
  deque<int> deque = deque<int>();
  for (int node = nodeRightNode[0]; node != 1; node = nodeRightNode[node]) {
    deque.push_back(node);
    markSameSpanNode(node, 1);
  }
  return deque;
}

int JapaneseTokenizer::Lattice::cost(int node)
{
  return nodeLeftCost[node] + nodeWordCost[node] + nodeRightCost[node];
}

deque<int> JapaneseTokenizer::Lattice::nBestNodeList(int N)
{
  deque<int> deque = deque<int>();
  int leastCost = numeric_limits<int>::max();
  int leastLeft = -1;
  int leastRight = -1;
  for (int node = 2; node < nodeCount; ++node) {
    if (nodeMark[node] == 0) {
      int cost = this->cost(node);
      if (cost < leastCost) {
        leastCost = cost;
        leastLeft = nodeLeft[node];
        leastRight = nodeRight[node];
        deque.clear();
        deque.push_back(node);
      } else if (cost == leastCost && (nodeLeft[node] != leastLeft ||
                                       nodeRight[node] != leastRight)) {
        deque.push_back(node);
      }
    }
  }
  for (auto node : deque) {
    markSameSpanNode(node, N);
  }
  return deque;
}

int JapaneseTokenizer::Lattice::bestCost() { return nodeLeftCost[1]; }

int JapaneseTokenizer::Lattice::probeDelta(int start, int end)
{
  int left = start - rootBase;
  int right = end - rootBase;
  if (left < 0 || rootSize < right) {
    return numeric_limits<int>::max();
  }
  int probedCost = numeric_limits<int>::max();
  for (int node = lRoot[left]; 0 <= node; node = nodeLeftChain[node]) {
    if (nodeRight[node] == right) {
      probedCost = min(probedCost, cost(node));
    }
  }
  return probedCost - bestCost();
}

void JapaneseTokenizer::Lattice::debugPrint()
{
  if (VERBOSE) {
    for (int node = 0; node < nodeCount; ++node) {
      printf(L"DEBUG NODE: node=%d, mark=%d, cost=%d, left=%d, right=%d\n",
             node, nodeMark[node], cost(node), nodeLeft[node], nodeRight[node]);
    }
  }
}

void JapaneseTokenizer::registerNode(int node, std::deque<wchar_t> &fragment)
{
  int left = lattice->nodeLeft[node];
  int right = lattice->nodeRight[node];
  Type type = lattice->nodeDicType[node];
  if (!discardPunctuation || !isPunctuation(fragment[left])) {
    if (type == Type::USER) {
      // The code below are based on backtrace().
      //
      // Expand the phraseID we recorded into the actual segmentation:
      const std::deque<int> wordIDAndLength =
          userDictionary->lookupSegmentation(lattice->nodeWordID[node]);
      int wordID = wordIDAndLength[0];
      pending.push_back(make_shared<Token>(wordID, fragment, left, right - left,
                                           Type::USER, lattice->rootBase + left,
                                           userDictionary));
      // Output compound
      int current = 0;
      for (int j = 1; j < wordIDAndLength.size(); j++) {
        constexpr int len = wordIDAndLength[j];
        if (len < right - left) {
          pending.push_back(make_shared<Token>(
              wordID + j - 1, fragment, current + left, len, Type::USER,
              lattice->rootBase + current + left, userDictionary));
        }
        current += len;
      }
    } else {
      pending.push_back(make_shared<Token>(
          lattice->nodeWordID[node], fragment, left, right - left, type,
          lattice->rootBase + left, getDict(type)));
    }
  }
}

void JapaneseTokenizer::fixupPendingList()
{
  // Sort for removing same tokens.
  // USER token should be ahead from normal one.
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(pending, new java.util.Comparator<Token>()
  sort(pending.begin(), pending.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  // Remove same token.
  for (int i = 1; i < pending.size(); ++i) {
    shared_ptr<Token> a = pending[i - 1];
    shared_ptr<Token> b = pending[i];
    if (a->getOffset() == b->getOffset() && a->getLength() == b->getLength()) {
      pending.erase(pending.begin() + i);
      // It is important to decrement "i" here, because a next may be removed.
      --i;
    }
  }

  // offset=>position map_obj
  unordered_map<int, int> map_obj = unordered_map<int, int>();
  for (auto t : pending) {
    map_obj.emplace(t->getOffset(), 0);
    map_obj.emplace(t->getOffset() + t->getLength(), 0);
  }

  // Get uniqe and sorted deque of all edge position of tokens.
  std::deque<optional<int>> offsets =
      map_obj.keySet().toArray(std::deque<optional<int>>(0));
  Arrays::sort(offsets);

  // setup all value of map_obj.  It specify N-th position from begin.
  for (int i = 0; i < offsets.size(); ++i) {
    map_obj.emplace(offsets[i], i);
  }

  // We got all position length now.
  for (auto t : pending) {
    t->setPositionLength(map_obj[t->getOffset() + t->getLength()] -
                         map_obj[t->getOffset()]);
  }

  // Make PENDING to be reversed order to fit its usage.
  // If you would like to speedup, you can try reversed order sort
  // at first of this function.
  reverse(pending.begin(), pending.end());
}

JapaneseTokenizer::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<JapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int JapaneseTokenizer::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Token> a, shared_ptr<Token> b)
{
  int aOff = a->getOffset();
  int bOff = b->getOffset();
  if (aOff != bOff) {
    return aOff - bOff;
  }
  int aLen = a->getLength();
  int bLen = b->getLength();
  if (aLen != bLen) {
    return aLen - bLen;
  }
  // order of Type is KNOWN, UNKNOWN, USER,
  // so we use reversed comparison here.
  return b->getType().ordinal() - a->getType().ordinal();
}

int JapaneseTokenizer::probeDelta(
    const wstring &inText, const wstring &requiredToken) 
{
  int start = (int)inText.find(requiredToken);
  if (start < 0) {
    // -1 when no requiredToken.
    return -1;
  }

  int delta = numeric_limits<int>::max();
  int saveNBestCost = nBestCost;
  setReader(make_shared<StringReader>(inText));
  reset();
  try {
    setNBestCost(1);
    int prevRootBase = -1;
    while (incrementToken()) {
      if (lattice->rootBase != prevRootBase) {
        prevRootBase = lattice->rootBase;
        delta = min(delta,
                    lattice->probeDelta(start, start + requiredToken.length()));
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // reset & end
    end();
    // setReader & close
    close();
    setNBestCost(saveNBestCost);
  }

  if (VERBOSE) {
    printf(L"JapaneseTokenizer: delta = %d: %s-%s\n", delta, inText,
           requiredToken);
  }
  return delta == numeric_limits<int>::max() ? -1 : delta;
}

int JapaneseTokenizer::calcNBestCost(const wstring &examples)
{
  int maxDelta = 0;
  for (wstring example : examples.split(L"/")) {
    if (!example.isEmpty()) {
      std::deque<wstring> pair = example.split(L"-");
      if (pair.size() != 2) {
        throw runtime_error(L"Unexpected example form: " + example +
                            L" (expected two '-')");
      } else {
        try {
          maxDelta = max(maxDelta, probeDelta(pair[0], pair[1]));
        } catch (const IOException &e) {
          // C++ TODO: This exception's constructor requires only one argument:
          // ORIGINAL LINE: throw new RuntimeException("Internal error
          // calculating best costs from examples. Got ", e);
          throw runtime_error(
              L"Internal error calculating best costs from examples. Got ");
        }
      }
    }
  }
  return maxDelta;
}

void JapaneseTokenizer::setNBestCost(int value)
{
  nBestCost = value;
  outputNBest = 0 < nBestCost;
}

void JapaneseTokenizer::backtraceNBest(shared_ptr<Position> endPosData,
                                       bool const useEOS) 
{
  if (lattice == nullptr) {
    lattice = make_shared<Lattice>();
  }

  constexpr int endPos = endPosData->pos;
  std::deque<wchar_t> fragment =
      buffer->get(lastBackTracePos, endPos - lastBackTracePos);
  lattice->setup(fragment, dictionaryMap, positions, lastBackTracePos, endPos,
                 useEOS);
  lattice->markUnreachable();
  lattice->calcLeftCost(costs);
  lattice->calcRightCost(costs);

  int bestCost = lattice->bestCost();
  if (VERBOSE) {
    printf(L"DEBUG: 1-BEST COST: %d\n", bestCost);
  }
  for (auto node : lattice->bestPathNodeList()) {
    registerNode(node, fragment);
  }

  for (int n = 2;; ++n) {
    deque<int> nbest = lattice->nBestNodeList(n);
    if (nbest.empty()) {
      break;
    }
    int cost = lattice->cost(nbest[0]);
    if (VERBOSE) {
      printf(L"DEBUG: %d-BEST COST: %d\n", n, cost);
    }
    if (bestCost + nBestCost < cost) {
      break;
    }
    for (auto node : nbest) {
      registerNode(node, fragment);
    }
  }
  if (VERBOSE) {
    lattice->debugPrint();
  }
}

void JapaneseTokenizer::backtrace(shared_ptr<Position> endPosData,
                                  int const fromIDX) 
{
  constexpr int endPos = endPosData->pos;

  if (VERBOSE) {
    wcout << L"\n  backtrace: endPos=" << endPos << L" pos=" << pos << L"; "
          << (pos - lastBackTracePos) << L" characters; last="
          << lastBackTracePos << L" cost=" << endPosData->costs[fromIDX]
          << endl;
  }

  const std::deque<wchar_t> fragment =
      buffer->get(lastBackTracePos, endPos - lastBackTracePos);

  if (dotOut != nullptr) {
    dotOut->onBacktrace(shared_from_this(), positions, lastBackTracePos,
                        endPosData, fromIDX, fragment, end_);
  }

  int pos = endPos;
  int bestIDX = fromIDX;
  shared_ptr<Token> altToken = nullptr;

  // We trace backwards, so this will be the leftWordID of
  // the token after the one we are now on:
  int lastLeftWordID = -1;

  int backCount = 0;

  // TODO: sort of silly to make Token instances here; the
  // back trace has all info needed to generate the
  // token.  So, we could just directly set the attrs,
  // from the backtrace, in incrementToken w/o ever
  // creating Token; we'd have to defer calling freeBefore
  // until after the backtrace was fully "consumed" by
  // incrementToken.

  while (pos > lastBackTracePos) {
    // System.out.println("BT: back pos=" + pos + " bestIDX=" + bestIDX);
    shared_ptr<Position> *const posData = positions->get(pos);
    assert(bestIDX < posData->count);

    int backPos = posData->backPos[bestIDX];
    assert((backPos >= lastBackTracePos, L"backPos=" + to_wstring(backPos) +
                                             L" vs lastBackTracePos=" +
                                             to_wstring(lastBackTracePos)));
    int length = pos - backPos;
    Type backType = posData->backType[bestIDX];
    int backID = posData->backID[bestIDX];
    int nextBestIDX = posData->backIndex[bestIDX];

    if (outputCompounds && searchMode && altToken == nullptr &&
        backType != Type::USER) {

      // In searchMode, if best path had picked a too-long
      // token, we use the "penalty" to compute the allowed
      // max cost of an alternate back-trace.  If we find an
      // alternate back trace with cost below that
      // threshold, we pursue it instead (but also output
      // the long token).
      // System.out.println("    2nd best backPos=" + backPos + " pos=" + pos);

      constexpr int penalty =
          computeSecondBestThreshold(backPos, pos - backPos);

      if (penalty > 0) {
        if (VERBOSE) {
          wcout << L"  compound="
                << wstring(buffer->get(backPos, pos - backPos)) << L" backPos="
                << backPos << L" pos=" << pos << L" penalty=" << penalty
                << L" cost=" << posData->costs[bestIDX] << L" bestIDX="
                << bestIDX << L" lastLeftID=" << lastLeftWordID << endl;
        }

        // Use the penalty to set maxCost on the 2nd best
        // segmentation:
        int maxCost = posData->costs[bestIDX] + penalty;
        if (lastLeftWordID != -1) {
          maxCost +=
              costs->get(getDict(backType)->getRightId(backID), lastLeftWordID);
        }

        // Now, prune all too-long tokens from the graph:
        pruneAndRescore(backPos, pos, posData->backIndex[bestIDX]);

        // Finally, find 2nd best back-trace and resume
        // backtrace there:
        int leastCost = numeric_limits<int>::max();
        int leastIDX = -1;
        for (int idx = 0; idx < posData->count; idx++) {
          int cost = posData->costs[idx];
          // System.out.println("    idx=" + idx + " prevCost=" + cost);

          if (lastLeftWordID != -1) {
            cost += costs->get(getDict(posData->backType[idx])
                                   ->getRightId(posData->backID[idx]),
                               lastLeftWordID);
            // System.out.println("      += bgCost=" +
            // costs.get(getDict(posData.backType[idx]).getRightId(posData.backID[idx]),
            // lastLeftWordID) + " -> " + cost);
          }
          // System.out.println("penalty " + posData.backPos[idx] + " to " +
          // pos); cost += computePenalty(posData.backPos[idx], pos -
          // posData.backPos[idx]);
          if (cost < leastCost) {
            // System.out.println("      ** ");
            leastCost = cost;
            leastIDX = idx;
          }
        }
        // System.out.println("  leastIDX=" + leastIDX);

        if (VERBOSE) {
          wcout << L"  afterPrune: " << posData->count
                << L" arcs arriving; leastCost=" << leastCost
                << L" vs threshold=" << maxCost << L" lastLeftWordID="
                << lastLeftWordID << endl;
        }

        if (leastIDX != -1 && leastCost <= maxCost &&
            posData->backPos[leastIDX] != backPos) {
          // We should have pruned the altToken from the graph:
          assert(posData->backPos[leastIDX] != backPos);

          // Save the current compound token, to output when
          // this alternate path joins back:
          altToken =
              make_shared<Token>(backID, fragment, backPos - lastBackTracePos,
                                 length, backType, backPos, getDict(backType));

          // Redirect our backtrace to 2nd best:
          bestIDX = leastIDX;
          nextBestIDX = posData->backIndex[bestIDX];

          backPos = posData->backPos[bestIDX];
          length = pos - backPos;
          backType = posData->backType[bestIDX];
          backID = posData->backID[bestIDX];
          backCount = 0;
          // System.out.println("  do alt token!");

        } else {
          // I think in theory it's possible there is no
          // 2nd best path, which is fine; in this case we
          // only output the compound token:
          // System.out.println("  no alt token! bestIDX=" + bestIDX);
        }
      }
    }

    constexpr int offset = backPos - lastBackTracePos;
    assert(offset >= 0);

    if (altToken != nullptr && altToken->getPosition() >= backPos) {

      // We've backtraced to the position where the
      // compound token starts; add it now:

      // The pruning we did when we created the altToken
      // ensures that the back trace will align back with
      // the start of the altToken:
      assert((altToken->getPosition() == backPos,
              to_wstring(altToken->getPosition()) + L" vs " +
                  to_wstring(backPos)));

      // NOTE: not quite right: the compound token may
      // have had all punctuation back traced so far, but
      // then the decompounded token at this position is
      // not punctuation.  In this case backCount is 0,
      // but we should maybe add the altToken anyway...?

      if (backCount > 0) {
        backCount++;
        altToken->setPositionLength(backCount);
        if (VERBOSE) {
          wcout << L"    add altToken=" << altToken << endl;
        }
        pending.push_back(altToken);
      } else {
        // This means alt token was all punct tokens:
        if (VERBOSE) {
          wcout << L"    discard all-punctuation altToken=" << altToken << endl;
        }
        assert(discardPunctuation);
      }
      altToken.reset();
    }

    shared_ptr<Dictionary> *const dict = getDict(backType);

    if (backType == Type::USER) {

      // Expand the phraseID we recorded into the actual
      // segmentation:
      const std::deque<int> wordIDAndLength =
          userDictionary->lookupSegmentation(backID);
      int wordID = wordIDAndLength[0];
      int current = 0;
      for (int j = 1; j < wordIDAndLength.size(); j++) {
        constexpr int len = wordIDAndLength[j];
        // System.out.println("    add user: len=" + len);
        pending.push_back(make_shared<Token>(wordID + j - 1, fragment,
                                             current + offset, len, Type::USER,
                                             current + backPos, dict));
        if (VERBOSE) {
          wcout << L"    add USER token=" << pending[pending.size() - 1]
                << endl;
        }
        current += len;
      }

      // Reverse the tokens we just added, because when we
      // serve them up from incrementToken we serve in
      // reverse:
      Collections::reverse(pending.subList(
          pending.size() - (wordIDAndLength.size() - 1), pending.size()));

      backCount += wordIDAndLength.size() - 1;
    } else {

      if (extendedMode && backType == Type::UNKNOWN) {
        // In EXTENDED mode we convert unknown word into
        // unigrams:
        int unigramTokenCount = 0;
        for (int i = length - 1; i >= 0; i--) {
          int charLen = 1;
          if (i > 0 && Character::isLowSurrogate(fragment[offset + i])) {
            i--;
            charLen = 2;
          }
          // System.out.println("    extended tok offset="
          //+ (offset + i));
          if (!discardPunctuation || !isPunctuation(fragment[offset + i])) {
            pending.push_back(make_shared<Token>(
                CharacterDefinition::NGRAM, fragment, offset + i, charLen,
                Type::UNKNOWN, backPos + i, unkDictionary));
            unigramTokenCount++;
          }
        }
        backCount += unigramTokenCount;

      } else if (!discardPunctuation || length == 0 ||
                 !isPunctuation(fragment[offset])) {
        pending.push_back(make_shared<Token>(backID, fragment, offset, length,
                                             backType, backPos, dict));
        if (VERBOSE) {
          wcout << L"    add token=" << pending[pending.size() - 1] << endl;
        }
        backCount++;
      } else {
        if (VERBOSE) {
          wcout << L"    skip punctuation token="
                << wstring(fragment, offset, length) << endl;
        }
      }
    }

    lastLeftWordID = dict->getLeftId(backID);
    pos = backPos;
    bestIDX = nextBestIDX;
  }

  lastBackTracePos = endPos;

  if (VERBOSE) {
    wcout << L"  freeBefore pos=" << endPos << endl;
  }
  // Notify the circular buffers that we are done with
  // these positions:
  buffer->freeBefore(endPos);
  positions->freeBefore(endPos);
}

shared_ptr<Dictionary> JapaneseTokenizer::getDict(Type type)
{
  return dictionaryMap->get(type);
}

bool JapaneseTokenizer::isPunctuation(wchar_t ch)
{
  switch (Character::getType(ch)) {
  case Character::SPACE_SEPARATOR:
  case Character::LINE_SEPARATOR:
  case Character::PARAGRAPH_SEPARATOR:
  case Character::CONTROL:
  case Character::FORMAT:
  case Character::DASH_PUNCTUATION:
  case Character::START_PUNCTUATION:
  case Character::END_PUNCTUATION:
  case Character::CONNECTOR_PUNCTUATION:
  case Character::OTHER_PUNCTUATION:
  case Character::MATH_SYMBOL:
  case Character::CURRENCY_SYMBOL:
  case Character::MODIFIER_SYMBOL:
  case Character::OTHER_SYMBOL:
  case Character::INITIAL_QUOTE_PUNCTUATION:
  case Character::FINAL_QUOTE_PUNCTUATION:
    return true;
  default:
    return false;
  }
}
} // namespace org::apache::lucene::analysis::ja