using namespace std;

#include "KoreanTokenizer.h"
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
#include "DecompoundToken.h"
#include "DictionaryToken.h"
#include "GraphvizFormatter.h"
#include "Token.h"
#include "dict/CharacterDefinition.h"
#include "dict/ConnectionCosts.h"
#include "dict/Dictionary.h"
#include "dict/TokenInfoDictionary.h"
#include "dict/TokenInfoFST.h"
#include "dict/UnknownDictionary.h"
#include "dict/UserDictionary.h"
#include "tokenattributes/PartOfSpeechAttribute.h"
#include "tokenattributes/ReadingAttribute.h"

namespace org::apache::lucene::analysis::ko
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharacterDefinition =
    org::apache::lucene::analysis::ko::dict::CharacterDefinition;
using ConnectionCosts =
    org::apache::lucene::analysis::ko::dict::ConnectionCosts;
using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;
using TokenInfoDictionary =
    org::apache::lucene::analysis::ko::dict::TokenInfoDictionary;
using TokenInfoFST = org::apache::lucene::analysis::ko::dict::TokenInfoFST;
using UnknownDictionary =
    org::apache::lucene::analysis::ko::dict::UnknownDictionary;
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;
using PartOfSpeechAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::PartOfSpeechAttribute;
using ReadingAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::ReadingAttribute;
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

KoreanTokenizer::KoreanTokenizer()
    : KoreanTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, nullptr,
                      DEFAULT_DECOMPOUND, false)
{
}

KoreanTokenizer::KoreanTokenizer(shared_ptr<AttributeFactory> factory,
                                 shared_ptr<UserDictionary> userDictionary,
                                 DecompoundMode mode,
                                 bool outputUnknownUnigrams)
    : org::apache::lucene::analysis::Tokenizer(factory),
      fst(dictionary->getFST()), dictionary(TokenInfoDictionary::getInstance()),
      unkDictionary(UnknownDictionary::getInstance()),
      costs(ConnectionCosts::getInstance()), userDictionary(userDictionary),
      characterDefinition(unkDictionary->getCharacterDefinition()),
      fstReader(fst->getBytesReader()), mode(mode),
      outputUnknownUnigrams(outputUnknownUnigrams)
{
  if (userDictionary != nullptr) {
    userFST = userDictionary->getFST();
    userFSTReader = userFST->getBytesReader();
  } else {
    userFST.reset();
    userFSTReader.reset();
  }

  buffer->reset(this->input);

  resetState();

  dictionaryMap->put(Type::KNOWN, dictionary);
  dictionaryMap->put(Type::UNKNOWN, unkDictionary);
  dictionaryMap->put(Type::USER, userDictionary);
}

void KoreanTokenizer::setGraphvizFormatter(shared_ptr<GraphvizFormatter> dotOut)
{
  this->dotOut = dotOut;
}

KoreanTokenizer::~KoreanTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  buffer->reset(input);
}

void KoreanTokenizer::reset() 
{
  Tokenizer::reset();
  buffer->reset(input);
  resetState();
}

void KoreanTokenizer::resetState()
{
  positions->reset();
  pos = 0;
  end_ = false;
  lastBackTracePos = 0;
  pending.clear();

  // Add BOS:
  positions->get(0)->add(0, 0, -1, -1, -1, -1, Type::KNOWN);
}

void KoreanTokenizer::end() 
{
  Tokenizer::end();
  // Set final offset
  int finalOffset = correctOffset(pos);
  offsetAtt->setOffset(finalOffset, finalOffset);
}

void KoreanTokenizer::Position::grow()
{
  costs = ArrayUtil::grow(costs, 1 + count);
  lastRightID = ArrayUtil::grow(lastRightID, 1 + count);
  backPos = ArrayUtil::grow(backPos, 1 + count);
  backWordPos = ArrayUtil::grow(backWordPos, 1 + count);
  backIndex = ArrayUtil::grow(backIndex, 1 + count);
  backID = ArrayUtil::grow(backID, 1 + count);

  // NOTE: sneaky: grow separately because
  // ArrayUtil.grow will otherwise pick a different
  // length than the int[]s we just grew:
  const std::deque<Type> newBackType = std::deque<Type>(backID.size());
  System::arraycopy(backType, 0, newBackType, 0, backType.size());
  backType = newBackType;
}

void KoreanTokenizer::Position::add(int cost, int lastRightID, int backPos,
                                    int backRPos, int backIndex, int backID,
                                    Type backType)
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
  this->backWordPos[count] = backRPos;
  this->backIndex[count] = backIndex;
  this->backID[count] = backID;
  this->backType[count] = backType;
  count++;
}

void KoreanTokenizer::Position::reset() { count = 0; }

int KoreanTokenizer::computeSpacePenalty(POS::Tag leftPOS, int numSpaces)
{
  int spacePenalty = 0;
  if (numSpaces > 0) {
    // TODO we should extract the penalty (left-space-penalty-factor) from the
    // dicrc file.
    switch (leftPOS.innerEnumValue) {
    case org::apache::lucene::analysis::ko::POS::Tag::InnerEnum::E:
    case org::apache::lucene::analysis::ko::POS::Tag::InnerEnum::J:
    case org::apache::lucene::analysis::ko::POS::Tag::InnerEnum::VCP:
    case org::apache::lucene::analysis::ko::POS::Tag::InnerEnum::XSA:
    case org::apache::lucene::analysis::ko::POS::Tag::InnerEnum::XSN:
    case org::apache::lucene::analysis::ko::POS::Tag::InnerEnum::XSV:
      spacePenalty = 3000;
      break;

    default:
      break;
    }
  }
  return spacePenalty;
}

void KoreanTokenizer::add(shared_ptr<Dictionary> dict,
                          shared_ptr<Position> fromPosData, int wordPos,
                          int endPos, int wordID, Type type) 
{
  constexpr POS::Tag leftPOS = dict->getLeftPOS(wordID);
  constexpr int wordCost = dict->getWordCost(wordID);
  constexpr int leftID = dict->getLeftId(wordID);
  int leastCost = numeric_limits<int>::max();
  int leastIDX = -1;
  assert(fromPosData->count > 0);
  for (int idx = 0; idx < fromPosData->count; idx++) {
    // The number of spaces before the term
    int numSpaces = wordPos - fromPosData->pos;

    // Cost is path cost so far, plus word cost (added at
    // end of loop), plus bigram cost and space penalty cost.
    constexpr int cost = fromPosData->costs[idx] +
                         costs->get(fromPosData->lastRightID[idx], leftID) +
                         computeSpacePenalty(leftPOS, numSpaces);
    if (VERBOSE) {
      wcout << L"      fromIDX=" << idx << L": cost=" << cost << L" (prevCost="
            << fromPosData->costs[idx] << L" wordCost=" << wordCost
            << L" bgCost=" << costs->get(fromPosData->lastRightID[idx], leftID)
            << L" spacePenalty=" << computeSpacePenalty(leftPOS, numSpaces)
            << L") leftID=" << leftID << L" leftPOS=" << leftPOS.name() << L")"
            << endl;
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

  positions->get(endPos)->add(leastCost, dict->getRightId(wordID),
                              fromPosData->pos, wordPos, leastIDX, wordID,
                              type);
}

bool KoreanTokenizer::incrementToken() 
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

  int length = token->getLength();
  clearAttributes();
  assert(length > 0);
  // System.out.println("off=" + token.getOffset() + " len=" + length + " vs " +
  // token.getSurfaceForm().length);
  termAtt->copyBuffer(token->getSurfaceForm(), token->getOffset(), length);
  offsetAtt->setOffset(correctOffset(token->getStartOffset()),
                       correctOffset(token->getEndOffset()));
  posAtt->setToken(token);
  readingAtt->setToken(token);
  posIncAtt->setPositionIncrement(token->getPositionIncrement());
  posLengthAtt->setPositionLength(token->getPositionLength());
  if (VERBOSE) {
    wcout << Thread::currentThread().getName()
          << L":    incToken: return token=" << token << endl;
  }
  return true;
}

KoreanTokenizer::WrappedPositionArray::WrappedPositionArray()
{
  for (int i = 0; i < positions.size(); i++) {
    positions[i] = make_shared<Position>();
  }
}

void KoreanTokenizer::WrappedPositionArray::reset()
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

shared_ptr<Position> KoreanTokenizer::WrappedPositionArray::get(int pos)
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

int KoreanTokenizer::WrappedPositionArray::getNextPos() { return nextPos; }

bool KoreanTokenizer::WrappedPositionArray::inBounds(int pos)
{
  return pos < nextPos && pos >= nextPos - count;
}

int KoreanTokenizer::WrappedPositionArray::getIndex(int pos)
{
  int index = nextWrite - (nextPos - pos);
  if (index < 0) {
    index += positions.size();
  }
  return index;
}

void KoreanTokenizer::WrappedPositionArray::freeBefore(int pos)
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

void KoreanTokenizer::parse() 
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
      backtrace(posData, 0);

      // Re-base cost so we don't risk int overflow:
      posData->costs[0] = 0;
      if (pending.size() > 0) {
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
            posData2->backWordPos[0] = posData2->backWordPos[leastIDX];
            posData2->backIndex[0] = posData2->backIndex[leastIDX];
            posData2->backID[0] = posData2->backID[leastIDX];
            posData2->backType[0] = posData2->backType[leastIDX];
          }
          posData2->count = 1;
        }
      }

      backtrace(leastPosData, 0);

      // Re-base cost so we don't risk int overflow:
      Arrays::fill(leastPosData->costs, 0, leastPosData->count, 0);

      if (pos != leastPosData->pos) {
        // We jumped into a future position:
        assert(pos < leastPosData->pos);
        pos = leastPosData->pos;
      }
      if (pending.size() > 0) {
        return;
      } else {
        // This means the backtrace only produced
        // punctuation tokens, so we must keep parsing.
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

    // Move to the first character that is not a whitespace.
    // The whitespaces are added as a prefix for the term that we extract,
    // this information is then used when computing the cost for the term using
    // the space penalty factor.
    // They are removed when the final tokens are generated.
    if (Character::getType(buffer->get(pos)) == Character::SPACE_SEPARATOR) {
      int nextChar = buffer->get(++pos);
      while (nextChar != -1 &&
             Character::getType(nextChar) == Character::SPACE_SEPARATOR) {
        pos++;
        nextChar = buffer->get(pos);
      }
    }
    if (buffer->get(pos) == -1) {
      pos = posData->pos;
    }

    bool anyMatches = false;

    // First try user dict:
    if (userFST != nullptr) {
      userFST->getFirstArc(arc);
      int output = 0;
      for (int posAhead = pos;; posAhead++) {
        constexpr int ch = buffer->get(posAhead);
        if (ch == -1) {
          break;
        }
        if (userFST->findTargetArc(ch, arc, arc, posAhead == pos,
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
          add(userDictionary, posData, pos, posAhead + 1,
              output + arc->nextFinalOutput.intValue(), Type::USER);
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

      for (int posAhead = pos;; posAhead++) {
        constexpr int ch = buffer->get(posAhead);
        if (ch == -1) {
          break;
        }
        // System.out.println("    match " + (char) ch + " posAhead=" +
        // posAhead);

        if (fst->findTargetArc(ch, arc, arc, posAhead == pos, fstReader) ==
            nullptr) {
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
            add(dictionary, posData, pos, posAhead + 1,
                wordIdRef->ints[wordIdRef->offset + ofs], Type::KNOWN);
            anyMatches = true;
          }
        }
      }
    }

    if (unknownWordEndIndex > posData->pos) {
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
        add(unkDictionary, posData, pos, pos + unknownWordLength,
            wordIdRef->ints[wordIdRef->offset + ofs], Type::UNKNOWN);
      }
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

    backtrace(endPosData, leastIDX);
  } else {
    // No characters in the input string; return no tokens!
  }
}

void KoreanTokenizer::backtrace(shared_ptr<Position> endPosData,
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
    int backWordPos = posData->backWordPos[bestIDX];
    assert((backPos >= lastBackTracePos, L"backPos=" + to_wstring(backPos) +
                                             L" vs lastBackTracePos=" +
                                             to_wstring(lastBackTracePos)));
    // the length of the word without the whitespaces at the beginning.
    int length = pos - backWordPos;
    Type backType = posData->backType[bestIDX];
    int backID = posData->backID[bestIDX];
    int nextBestIDX = posData->backIndex[bestIDX];
    // the start of the word after the whitespace at the beginning.
    constexpr int fragmentOffset = backWordPos - lastBackTracePos;
    assert(fragmentOffset >= 0);

    shared_ptr<Dictionary> *const dict = getDict(backType);

    if (outputUnknownUnigrams && backType == Type::UNKNOWN) {
      // outputUnknownUnigrams converts unknown word into unigrams:
      for (int i = length - 1; i >= 0; i--) {
        int charLen = 1;
        if (i > 0 && Character::isLowSurrogate(fragment[fragmentOffset + i])) {
          i--;
          charLen = 2;
        }
        shared_ptr<DictionaryToken> *const token = make_shared<DictionaryToken>(
            Type::UNKNOWN, unkDictionary, CharacterDefinition::NGRAM, fragment,
            fragmentOffset + i, charLen, backWordPos + i,
            backWordPos + i + charLen);
        if (shouldFilterToken(token) == false) {
          pending.push_back(token);
          if (VERBOSE) {
            wcout << L"    add token=" << pending[pending.size() - 1] << endl;
          }
        }
      }
    } else {
      shared_ptr<DictionaryToken> *const token = make_shared<DictionaryToken>(
          backType, dict, backID, fragment, fragmentOffset, length, backWordPos,
          backWordPos + length);
      if (token->getPOSType() == POS::Type::MORPHEME ||
          mode == DecompoundMode::NONE) {
        if (shouldFilterToken(token) == false) {
          pending.push_back(token);
          if (VERBOSE) {
            wcout << L"    add token=" << pending[pending.size() - 1] << endl;
          }
        }
      } else {
        std::deque<std::shared_ptr<Dictionary::Morpheme>> morphemes =
            token->getMorphemes();
        if (morphemes.empty()) {
          pending.push_back(token);
          if (VERBOSE) {
            wcout << L"    add token=" << pending[pending.size() - 1] << endl;
          }
        } else {
          int endOffset = backWordPos + length;
          int posLen = 0;
          // decompose the compound
          for (int i = morphemes.size() - 1; i >= 0; i--) {
            shared_ptr<Dictionary::Morpheme> *const morpheme = morphemes[i];
            shared_ptr<Token> *const compoundToken;
            if (token->getPOSType() == POS::Type::COMPOUND) {
              assert(endOffset - morpheme->surfaceForm.length() >= 0);
              compoundToken = make_shared<DecompoundToken>(
                  morpheme->posTag, morpheme->surfaceForm,
                  endOffset - morpheme->surfaceForm.length(), endOffset);
            } else {
              compoundToken = make_shared<DecompoundToken>(
                  morpheme->posTag, morpheme->surfaceForm,
                  token->getStartOffset(), token->getEndOffset());
            }
            if (i == 0 && mode == DecompoundMode::MIXED) {
              compoundToken->setPositionIncrement(0);
            }
            ++posLen;
            endOffset -= morpheme->surfaceForm.length();
            pending.push_back(compoundToken);
            if (VERBOSE) {
              wcout << L"    add token=" << pending[pending.size() - 1] << endl;
            }
          }
          if (mode == DecompoundMode::MIXED) {
            token->setPositionLength(max(1, posLen));
            pending.push_back(token);
            if (VERBOSE) {
              wcout << L"    add token=" << pending[pending.size() - 1] << endl;
            }
          }
        }
      }
    }

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

shared_ptr<Dictionary> KoreanTokenizer::getDict(Type type)
{
  return dictionaryMap->get(type);
}

bool KoreanTokenizer::shouldFilterToken(shared_ptr<Token> token)
{
  return isPunctuation(token->getSurfaceForm()[token->getOffset()]);
}

bool KoreanTokenizer::isPunctuation(wchar_t ch)
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
} // namespace org::apache::lucene::analysis::ko