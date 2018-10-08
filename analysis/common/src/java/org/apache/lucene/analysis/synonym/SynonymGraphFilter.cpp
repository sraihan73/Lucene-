using namespace std;

#include "SynonymGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "SynonymMap.h"

namespace org::apache::lucene::analysis::synonym
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FlattenGraphFilter =
    org::apache::lucene::analysis::core::FlattenGraphFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;
using FST = org::apache::lucene::util::fst::FST;
const wstring SynonymGraphFilter::TYPE_SYNONYM = L"SYNONYM";

SynonymGraphFilter::RollingBufferAnonymousInnerClass::
    RollingBufferAnonymousInnerClass()
{
}

shared_ptr<BufferedInputToken>
SynonymGraphFilter::RollingBufferAnonymousInnerClass::newInstance()
{
  return make_shared<BufferedInputToken>();
}

void SynonymGraphFilter::BufferedInputToken::reset()
{
  state.reset();
  term->clear();

  // Intentionally invalid to ferret out bugs:
  startOffset = -1;
  endOffset = -1;
}

SynonymGraphFilter::BufferedOutputToken::BufferedOutputToken(
    shared_ptr<AttributeSource::State> state, const wstring &term,
    int startNode, int endNode)
    : term(term), state(state), startNode(startNode), endNode(endNode)
{
}

SynonymGraphFilter::SynonymGraphFilter(shared_ptr<TokenStream> input,
                                       shared_ptr<SynonymMap> synonyms,
                                       bool ignoreCase)
    : org::apache::lucene::analysis::TokenFilter(input), synonyms(synonyms),
      ignoreCase(ignoreCase), fst(synonyms->fst),
      fstReader(fst->getBytesReader()), scratchArc(make_shared<FST::Arc<>>())
{
  if (fst == nullptr) {
    throw invalid_argument(L"fst must be non-null");
  }
}

bool SynonymGraphFilter::incrementToken() 
{
  // System.out.println("\nS: incrToken lastNodeOut=" + lastNodeOut + "
  // nextNodeOut=" + nextNodeOut);

  assert(lastNodeOut <= nextNodeOut);

  if (outputBuffer.empty() == false) {
    // We still have pending outputs from a prior synonym match:
    releaseBufferedToken();
    // System.out.println("  syn: ret buffered=" + this);
    assert(liveToken == false);
    return true;
  }

  // Try to parse a new synonym match at the current token:

  if (parse()) {
    // A new match was found:
    releaseBufferedToken();
    // System.out.println("  syn: after parse, ret buffered=" + this);
    assert(liveToken == false);
    return true;
  }

  if (lookaheadNextRead == lookaheadNextWrite) {

    // Fast path: parse pulled one token, but it didn't match
    // the start for any synonym, so we now return it "live" w/o having
    // cloned all of its atts:
    if (finished) {
      // System.out.println("  syn: ret END");
      return false;
    }

    assert(liveToken);
    liveToken = false;

    // NOTE: no need to change posInc since it's relative, i.e. whatever
    // node our output is upto will just increase by the incoming posInc.
    // We also don't need to change posLen, but only because we cannot
    // consume a graph, so the incoming token can never span a future
    // synonym match.

  } else {
    // We still have buffered lookahead tokens from a previous
    // parse attempt that required lookahead; just replay them now:
    // System.out.println("  restore buffer");
    assert((lookaheadNextRead < lookaheadNextWrite,
            L"read=" + to_wstring(lookaheadNextRead) + L" write=" +
                to_wstring(lookaheadNextWrite)));
    shared_ptr<BufferedInputToken> token = lookahead->get(lookaheadNextRead);
    lookaheadNextRead++;

    restoreState(token->state);

    lookahead::freeBefore(lookaheadNextRead);

    // System.out.println("  after restore offset=" + offsetAtt.startOffset() +
    // "-" + offsetAtt.endOffset());
    assert(liveToken == false);
  }

  lastNodeOut += posIncrAtt->getPositionIncrement();
  nextNodeOut = lastNodeOut + posLenAtt->getPositionLength();

  // System.out.println("  syn: ret lookahead=" + this);

  return true;
}

void SynonymGraphFilter::releaseBufferedToken() 
{
  // System.out.println("  releaseBufferedToken");

  shared_ptr<BufferedOutputToken> token = outputBuffer.pollFirst();

  if (token->state != nullptr) {
    // This is an original input token (keepOrig=true case):
    // System.out.println("    hasState");
    restoreState(token->state);
    // System.out.println("    startOffset=" + offsetAtt.startOffset() + "
    // endOffset=" + offsetAtt.endOffset());
  } else {
    clearAttributes();
    // System.out.println("    no state");
    termAtt->append(token->term);

    // We better have a match already:
    assert(matchStartOffset != -1);

    offsetAtt->setOffset(matchStartOffset, matchEndOffset);
    // System.out.println("    startOffset=" + matchStartOffset + " endOffset="
    // + matchEndOffset);
    typeAtt->setType(TYPE_SYNONYM);
  }

  // System.out.println("    lastNodeOut=" + lastNodeOut);
  // System.out.println("    term=" + termAtt);

  posIncrAtt->setPositionIncrement(token->startNode - lastNodeOut);
  lastNodeOut = token->startNode;
  posLenAtt->setPositionLength(token->endNode - token->startNode);
}

bool SynonymGraphFilter::parse() 
{
  // System.out.println(Thread.currentThread().getName() + ": S: parse: " +
  // System.identityHashCode(this));

  // Holds the longest match we've seen so far:
  shared_ptr<BytesRef> matchOutput = nullptr;
  int matchInputLength = 0;

  shared_ptr<BytesRef> pendingOutput = fst->outputs->getNoOutput();
  fst->getFirstArc(scratchArc);

  assert(scratchArc->output == fst->outputs->getNoOutput());

  // How many tokens in the current match
  int matchLength = 0;
  bool doFinalCapture = false;

  int lookaheadUpto = lookaheadNextRead;
  matchStartOffset = -1;

  while (true) {
    // System.out.println("  cycle lookaheadUpto=" + lookaheadUpto + " maxPos="
    // + lookahead.getMaxPos());

    // Pull next token's chars:
    const std::deque<wchar_t> buffer;
    constexpr int bufferLen;
    constexpr int inputEndOffset;

    if (lookaheadUpto <= lookahead::getMaxPos()) {
      // Still in our lookahead buffer
      shared_ptr<BufferedInputToken> token = lookahead->get(lookaheadUpto);
      lookaheadUpto++;
      buffer = token->term->chars();
      bufferLen = token->term->length();
      inputEndOffset = token->endOffset;
      // System.out.println("    use buffer now max=" + lookahead.getMaxPos());
      if (matchStartOffset == -1) {
        matchStartOffset = token->startOffset;
      }
    } else {

      // We used up our lookahead buffer of input tokens
      // -- pull next real input token:

      assert(finished || liveToken == false);

      if (finished) {
        // System.out.println("    break: finished");
        break;
      } else if (input->incrementToken()) {
        // System.out.println("    input.incrToken");
        liveToken = true;
        buffer = termAtt->buffer();
        bufferLen = termAtt->length();
        if (matchStartOffset == -1) {
          matchStartOffset = offsetAtt->startOffset();
        }
        inputEndOffset = offsetAtt->endOffset();

        lookaheadUpto++;
      } else {
        // No more input tokens
        finished = true;
        // System.out.println("    break: now set finished");
        break;
      }
    }

    matchLength++;
    // System.out.println("    cycle term=" + new std::wstring(buffer, 0, bufferLen));

    // Run each char in this token through the FST:
    int bufUpto = 0;
    while (bufUpto < bufferLen) {
      constexpr int codePoint =
          Character::codePointAt(buffer, bufUpto, bufferLen);
      if (fst->findTargetArc(ignoreCase ? towlower(codePoint) : codePoint,
                             scratchArc, scratchArc, fstReader) == nullptr) {
        goto byTokenBreak;
      }

      // Accum the output
      pendingOutput = fst->outputs->add(pendingOutput, scratchArc->output);
      bufUpto += Character::charCount(codePoint);
    }

    assert(bufUpto == bufferLen);

    // OK, entire token matched; now see if this is a final
    // state in the FST (a match):
    if (scratchArc->isFinal()) {
      matchOutput =
          fst->outputs->add(pendingOutput, scratchArc->nextFinalOutput);
      matchInputLength = matchLength;
      matchEndOffset = inputEndOffset;
      // System.out.println("    ** match");
    }

    // See if the FST can continue matching (ie, needs to
    // see the next input token):
    if (fst->findTargetArc(SynonymMap::WORD_SEPARATOR, scratchArc, scratchArc,
                           fstReader) == nullptr) {
      // No further rules can match here; we're done
      // searching for matching rules starting at the
      // current input position.
      break;
    } else {
      // More matching is possible -- accum the output (if
      // any) of the WORD_SEP arc:
      pendingOutput = fst->outputs->add(pendingOutput, scratchArc->output);
      doFinalCapture = true;
      if (liveToken) {
        capture();
      }
    }
  byTokenContinue:;
  }
byTokenBreak:

  if (doFinalCapture && liveToken && finished == false) {
    // Must capture the final token if we captured any prior tokens:
    capture();
  }

  if (matchOutput != nullptr) {

    if (liveToken) {
      // Single input token synonym; we must buffer it now:
      capture();
    }

    // There is a match!
    bufferOutputTokens(matchOutput, matchInputLength);
    lookaheadNextRead += matchInputLength;
    // System.out.println("  precmatch; set lookaheadNextRead=" +
    // lookaheadNextRead + " now max=" + lookahead.getMaxPos());
    lookahead::freeBefore(lookaheadNextRead);
    // System.out.println("  match; set lookaheadNextRead=" + lookaheadNextRead
    // + " now max=" + lookahead.getMaxPos());
    return true;
  } else {
    // System.out.println("  no match; lookaheadNextRead=" + lookaheadNextRead);
    return false;
  }

  // System.out.println("  parse done inputSkipCount=" + inputSkipCount + "
  // nextRead=" + nextRead + " nextWrite=" + nextWrite);
}

void SynonymGraphFilter::bufferOutputTokens(shared_ptr<BytesRef> bytes,
                                            int matchInputLength)
{
  bytesReader->reset(bytes->bytes, bytes->offset, bytes->length);

  constexpr int code = bytesReader->readVInt();
  constexpr bool keepOrig = (code & 0x1) == 0;
  // System.out.println("  buffer: keepOrig=" + keepOrig + " matchInputLength="
  // + matchInputLength);

  // How many nodes along all paths; we need this to assign the
  // node ID for the final end node where all paths merge back:
  int totalPathNodes;
  if (keepOrig) {
    assert(matchInputLength > 0);
    totalPathNodes = matchInputLength - 1;
  } else {
    totalPathNodes = 0;
  }

  // How many synonyms we will insert over this match:
  constexpr int count = static_cast<int>(static_cast<unsigned int>(code) >> 1);

  // TODO: we could encode this instead into the FST:

  // 1st pass: count how many new nodes we need
  deque<deque<wstring>> paths = deque<deque<wstring>>();
  for (int outputIDX = 0; outputIDX < count; outputIDX++) {
    int wordID = bytesReader->readVInt();
    synonyms->words->get(wordID, scratchBytes);
    scratchChars->copyUTF8Bytes(scratchBytes);
    int lastStart = 0;

    deque<wstring> path = deque<wstring>();
    paths.push_back(path);
    int chEnd = scratchChars->length();
    for (int chUpto = 0; chUpto <= chEnd; chUpto++) {
      if (chUpto == chEnd ||
          scratchChars->charAt(chUpto) == SynonymMap::WORD_SEPARATOR) {
        path.push_back(
            wstring(scratchChars->chars(), lastStart, chUpto - lastStart));
        lastStart = 1 + chUpto;
      }
    }

    assert(path.size() > 0);
    totalPathNodes += path.size() - 1;
  }
  // System.out.println("  totalPathNodes=" + totalPathNodes);

  // 2nd pass: buffer tokens for the graph fragment

  // NOTE: totalPathNodes will be 0 in the case where the matched
  // input is a single token and all outputs are also a single token

  // We "spawn" a side-path for each of the outputs for this matched
  // synonym, all ending back at this end node:

  int startNode = nextNodeOut;

  int endNode = startNode + totalPathNodes + 1;
  // System.out.println("  " + paths.size() + " new side-paths");

  // First, fanout all tokens departing start node for these new side paths:
  int newNodeCount = 0;
  for (auto path : paths) {
    int pathEndNode;
    // System.out.println("    path size=" + path.size());
    if (path.size() == 1) {
      // Single token output, so there are no intermediate nodes:
      pathEndNode = endNode;
    } else {
      pathEndNode = nextNodeOut + newNodeCount + 1;
      newNodeCount += path.size() - 1;
    }
    outputBuffer.push_back(make_shared<BufferedOutputToken>(
        nullptr, path[0], startNode, pathEndNode));
  }

  // We must do the original tokens last, else the offsets "go backwards":
  if (keepOrig) {
    shared_ptr<BufferedInputToken> token = lookahead->get(lookaheadNextRead);
    int inputEndNode;
    if (matchInputLength == 1) {
      // Single token matched input, so there are no intermediate nodes:
      inputEndNode = endNode;
    } else {
      inputEndNode = nextNodeOut + newNodeCount + 1;
    }

    // System.out.println("    keepOrig first token: " + token.term);

    // C++ TODO: There is no native C++ equivalent to 'toString':
    outputBuffer.push_back(make_shared<BufferedOutputToken>(
        token->state, token->term->toString(), startNode, inputEndNode));
  }

  nextNodeOut = endNode;

  // Do full side-path for each syn output:
  for (int pathID = 0; pathID < paths.size(); pathID++) {
    deque<wstring> path = paths[pathID];
    if (path.size() > 1) {
      int lastNode = outputBuffer.get(pathID).endNode;
      for (int i = 1; i < path.size() - 1; i++) {
        outputBuffer.push_back(make_shared<BufferedOutputToken>(
            nullptr, path[i], lastNode, lastNode + 1));
        lastNode++;
      }
      outputBuffer.push_back(make_shared<BufferedOutputToken>(
          nullptr, path[path.size() - 1], lastNode, endNode));
    }
  }

  if (keepOrig && matchInputLength > 1) {
    // Do full "side path" with the original tokens:
    int lastNode = outputBuffer.get(paths.size()).endNode;
    for (int i = 1; i < matchInputLength - 1; i++) {
      shared_ptr<BufferedInputToken> token =
          lookahead->get(lookaheadNextRead + i);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      outputBuffer.push_back(make_shared<BufferedOutputToken>(
          token->state, token->term->toString(), lastNode, lastNode + 1));
      lastNode++;
    }
    shared_ptr<BufferedInputToken> token =
        lookahead->get(lookaheadNextRead + matchInputLength - 1);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    outputBuffer.push_back(make_shared<BufferedOutputToken>(
        token->state, token->term->toString(), lastNode, endNode));
  }

  /*
  System.out.println("  after buffer: " + outputBuffer.size() + " tokens:");
  for(BufferedOutputToken token : outputBuffer) {
    System.out.println("    tok: " + token.term + " startNode=" +
  token.startNode + " endNode=" + token.endNode);
  }
  */
}

void SynonymGraphFilter::capture()
{
  assert(liveToken);
  liveToken = false;
  shared_ptr<BufferedInputToken> token = lookahead->get(lookaheadNextWrite);
  lookaheadNextWrite++;

  token->state = captureState();
  token->startOffset = offsetAtt->startOffset();
  token->endOffset = offsetAtt->endOffset();
  assert(token->term->length() == 0);
  token->term->append(termAtt);

  captureCount++;
  maxLookaheadUsed = max(maxLookaheadUsed, lookahead::getBufferSize());
  // System.out.println("  maxLookaheadUsed=" + maxLookaheadUsed);
}

void SynonymGraphFilter::reset() 
{
  TokenFilter::reset();
  lookahead::reset();
  lookaheadNextWrite = 0;
  lookaheadNextRead = 0;
  captureCount = 0;
  lastNodeOut = -1;
  nextNodeOut = 0;
  matchStartOffset = -1;
  matchEndOffset = -1;
  finished = false;
  liveToken = false;
  outputBuffer.clear();
  maxLookaheadUsed = 0;
  // System.out.println("S: reset");
}

int SynonymGraphFilter::getCaptureCount() { return captureCount; }

int SynonymGraphFilter::getMaxLookaheadUsed() { return maxLookaheadUsed; }
} // namespace org::apache::lucene::analysis::synonym