using namespace std;

#include "SynonymFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
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
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using FST = org::apache::lucene::util::fst::FST;
const wstring SynonymFilter::TYPE_SYNONYM = L"SYNONYM";

void SynonymFilter::PendingInput::reset()
{
  state.reset();
  consumed = true;
  keepOrig = false;
  matched = false;
}

SynonymFilter::PendingOutputs::PendingOutputs()
{
  outputs = std::deque<std::shared_ptr<CharsRefBuilder>>(1);
  endOffsets = std::deque<int>(1);
  posLengths = std::deque<int>(1);
}

void SynonymFilter::PendingOutputs::reset()
{
  upto = count = 0;
  posIncr = 1;
}

shared_ptr<CharsRef> SynonymFilter::PendingOutputs::pullNext()
{
  assert(upto < count);
  lastEndOffset = endOffsets[upto];
  lastPosLength = posLengths[upto];
  shared_ptr<CharsRefBuilder> *const result = outputs[upto++];
  posIncr = 0;
  if (upto == count) {
    reset();
  }
  return result->get();
}

int SynonymFilter::PendingOutputs::getLastEndOffset() { return lastEndOffset; }

int SynonymFilter::PendingOutputs::getLastPosLength() { return lastPosLength; }

void SynonymFilter::PendingOutputs::add(std::deque<wchar_t> &output,
                                        int offset, int len, int endOffset,
                                        int posLength)
{
  if (count == outputs.size()) {
    outputs = Arrays::copyOf(
        outputs, ArrayUtil::oversize(1 + count,
                                     RamUsageEstimator::NUM_BYTES_OBJECT_REF));
  }
  if (count == endOffsets.size()) {
    const std::deque<int> next =
        std::deque<int>(ArrayUtil::oversize(1 + count, Integer::BYTES));
    System::arraycopy(endOffsets, 0, next, 0, count);
    endOffsets = next;
  }
  if (count == posLengths.size()) {
    const std::deque<int> next =
        std::deque<int>(ArrayUtil::oversize(1 + count, Integer::BYTES));
    System::arraycopy(posLengths, 0, next, 0, count);
    posLengths = next;
  }
  if (outputs[count] == nullptr) {
    outputs[count] = make_shared<CharsRefBuilder>();
  }
  outputs[count]->copyChars(output, offset, len);
  // endOffset can be -1, in which case we should simply
  // use the endOffset of the input token, or X >= 0, in
  // which case we use X as the endOffset for this output
  endOffsets[count] = endOffset;
  posLengths[count] = posLength;
  count++;
}

SynonymFilter::SynonymFilter(shared_ptr<TokenStream> input,
                             shared_ptr<SynonymMap> synonyms, bool ignoreCase)
    : org::apache::lucene::analysis::TokenFilter(input), synonyms(synonyms),
      ignoreCase(ignoreCase),
      rollBufferSize(1 + synonyms->maxHorizontalContext),
      futureInputs(std::deque<std::shared_ptr<PendingInput>>(rollBufferSize)),
      futureOutputs(
          std::deque<std::shared_ptr<PendingOutputs>>(rollBufferSize)),
      scratchArc(make_shared<FST::Arc<>>()), fst(synonyms->fst),
      fstReader(fst->getBytesReader())
{
  if (fst == nullptr) {
    throw invalid_argument(L"fst must be non-null");
  }

  // Must be 1+ so that when roll buffer is at full
  // lookahead we can distinguish this full buffer from
  // the empty buffer:

  for (int pos = 0; pos < rollBufferSize; pos++) {
    futureInputs[pos] = make_shared<PendingInput>();
    futureOutputs[pos] = make_shared<PendingOutputs>();
  }

  // System.out.println("FSTFilt maxH=" + synonyms.maxHorizontalContext);
}

void SynonymFilter::capture()
{
  captureCount++;
  // System.out.println("  capture slot=" + nextWrite);
  shared_ptr<PendingInput> *const input = futureInputs[nextWrite];

  input->state = captureState();
  input->consumed = false;
  input->term->copyChars(termAtt->buffer(), 0, termAtt->length());

  nextWrite = rollIncr(nextWrite);

  // Buffer head should never catch up to tail:
  assert(nextWrite != nextRead);
}

void SynonymFilter::parse() 
{
  // System.out.println("\nS: parse");

  assert(inputSkipCount == 0);

  int curNextRead = nextRead;

  // Holds the longest match we've seen so far:
  shared_ptr<BytesRef> matchOutput = nullptr;
  int matchInputLength = 0;
  int matchEndOffset = -1;

  shared_ptr<BytesRef> pendingOutput = fst->outputs->getNoOutput();
  fst->getFirstArc(scratchArc);

  assert(scratchArc->output == fst->outputs->getNoOutput());

  int tokenCount = 0;

  while (true) {

    // Pull next token's chars:
    const std::deque<wchar_t> buffer;
    constexpr int bufferLen;
    // System.out.println("  cycle nextRead=" + curNextRead + " nextWrite=" +
    // nextWrite);

    int inputEndOffset = 0;

    if (curNextRead == nextWrite) {

      // We used up our lookahead buffer of input tokens
      // -- pull next real input token:

      if (finished) {
        break;
      } else {
        // System.out.println("  input.incrToken");
        assert(futureInputs[nextWrite]->consumed);
        // Not correct: a syn match whose output is longer
        // than its input can set future inputs keepOrig
        // to true:
        // assert !futureInputs[nextWrite].keepOrig;
        if (input->incrementToken()) {
          buffer = termAtt->buffer();
          bufferLen = termAtt->length();
          shared_ptr<PendingInput> *const input = futureInputs[nextWrite];
          lastStartOffset = input->startOffset = offsetAtt->startOffset();
          lastEndOffset = input->endOffset = offsetAtt->endOffset();
          inputEndOffset = input->endOffset;
          // System.out.println("  new token=" + new std::wstring(buffer, 0,
          // bufferLen));
          if (nextRead != nextWrite) {
            capture();
          } else {
            input->consumed = false;
          }

        } else {
          // No more input tokens
          // System.out.println("      set end");
          finished = true;
          break;
        }
      }
    } else {
      // Still in our lookahead
      buffer = futureInputs[curNextRead]->term.chars();
      bufferLen = futureInputs[curNextRead]->term->length();
      inputEndOffset = futureInputs[curNextRead]->endOffset;
      // System.out.println("  old token=" + new std::wstring(buffer, 0, bufferLen));
    }

    tokenCount++;

    // Run each char in this token through the FST:
    int bufUpto = 0;
    while (bufUpto < bufferLen) {
      constexpr int codePoint =
          Character::codePointAt(buffer, bufUpto, bufferLen);
      if (fst->findTargetArc(ignoreCase ? towlower(codePoint) : codePoint,
                             scratchArc, scratchArc, fstReader) == nullptr) {
        // System.out.println("    stop");
        goto byTokenBreak;
      }

      // Accum the output
      pendingOutput = fst->outputs->add(pendingOutput, scratchArc->output);
      // System.out.println("    char=" + buffer[bufUpto] + " output=" +
      // pendingOutput + " arc.output=" + scratchArc.output);
      bufUpto += Character::charCount(codePoint);
    }

    // OK, entire token matched; now see if this is a final
    // state:
    if (scratchArc->isFinal()) {
      matchOutput =
          fst->outputs->add(pendingOutput, scratchArc->nextFinalOutput);
      matchInputLength = tokenCount;
      matchEndOffset = inputEndOffset;
      // System.out.println("  found matchLength=" + matchInputLength + "
      // output=" + matchOutput);
    }

    // See if the FST wants to continue matching (ie, needs to
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
      if (nextRead == nextWrite) {
        capture();
      }
    }

    curNextRead = rollIncr(curNextRead);
  byTokenContinue:;
  }
byTokenBreak:

  if (nextRead == nextWrite && !finished) {
    // System.out.println("  skip write slot=" + nextWrite);
    nextWrite = rollIncr(nextWrite);
  }

  if (matchOutput != nullptr) {
    // System.out.println("  add matchLength=" + matchInputLength + " output=" +
    // matchOutput);
    inputSkipCount = matchInputLength;
    addOutput(matchOutput, matchInputLength, matchEndOffset);
  } else if (nextRead != nextWrite) {
    // Even though we had no match here, we set to 1
    // because we need to skip current input token before
    // trying to match again:
    inputSkipCount = 1;
  } else {
    assert(finished);
  }

  // System.out.println("  parse done inputSkipCount=" + inputSkipCount + "
  // nextRead=" + nextRead + " nextWrite=" + nextWrite);
}

void SynonymFilter::addOutput(shared_ptr<BytesRef> bytes, int matchInputLength,
                              int matchEndOffset)
{
  bytesReader->reset(bytes->bytes, bytes->offset, bytes->length);

  constexpr int code = bytesReader->readVInt();
  constexpr bool keepOrig = (code & 0x1) == 0;
  constexpr int count = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  // System.out.println("  addOutput count=" + count + " keepOrig=" + keepOrig);
  for (int outputIDX = 0; outputIDX < count; outputIDX++) {
    synonyms->words->get(bytesReader->readVInt(), scratchBytes);
    // System.out.println("    outIDX=" + outputIDX + " bytes=" +
    // scratchBytes.length);
    scratchChars->copyUTF8Bytes(scratchBytes);
    int lastStart = 0;
    constexpr int chEnd = lastStart + scratchChars->length();
    int outputUpto = nextRead;
    for (int chIDX = lastStart; chIDX <= chEnd; chIDX++) {
      if (chIDX == chEnd ||
          scratchChars->charAt(chIDX) == SynonymMap::WORD_SEPARATOR) {
        constexpr int outputLen = chIDX - lastStart;
        // Caller is not allowed to have empty string in
        // the output:
        assert(
            (outputLen > 0, L"output contains empty string: " + scratchChars));
        constexpr int endOffset;
        constexpr int posLen;
        if (chIDX == chEnd && lastStart == 0) {
          // This rule had a single output token, so, we set
          // this output's endOffset to the current
          // endOffset (ie, endOffset of the last input
          // token it matched):
          endOffset = matchEndOffset;
          posLen = keepOrig ? matchInputLength : 1;
        } else {
          // This rule has more than one output token; we
          // can't pick any particular endOffset for this
          // case, so, we inherit the endOffset for the
          // input token which this output overlaps:
          endOffset = -1;
          posLen = 1;
        }
        futureOutputs[outputUpto]->add(scratchChars->chars(), lastStart,
                                       outputLen, endOffset, posLen);
        // System.out.println("      " + new std::wstring(scratchChars.chars,
        // lastStart, outputLen) + " outputUpto=" + outputUpto);
        lastStart = 1 + chIDX;
        // System.out.println("  slot=" + outputUpto + " keepOrig=" + keepOrig);
        outputUpto = rollIncr(outputUpto);
        assert((futureOutputs[outputUpto]->posIncr == 1,
                L"outputUpto=" + to_wstring(outputUpto) + L" vs nextWrite=" +
                    to_wstring(nextWrite)));
      }
    }
  }

  int upto = nextRead;
  for (int idx = 0; idx < matchInputLength; idx++) {
    futureInputs[upto]->keepOrig |= keepOrig;
    futureInputs[upto]->matched = true;
    upto = rollIncr(upto);
  }
}

int SynonymFilter::rollIncr(int count)
{
  count++;
  if (count == rollBufferSize) {
    return 0;
  } else {
    return count;
  }
}

int SynonymFilter::getCaptureCount() { return captureCount; }

bool SynonymFilter::incrementToken() 
{

  // System.out.println("\nS: incrToken inputSkipCount=" + inputSkipCount + "
  // nextRead=" + nextRead + " nextWrite=" + nextWrite);

  while (true) {

    // First play back any buffered future inputs/outputs
    // w/o running parsing again:
    while (inputSkipCount != 0) {

      // At each position, we first output the original
      // token

      // TODO: maybe just a PendingState class, holding
      // both input & outputs?
      shared_ptr<PendingInput> *const input = futureInputs[nextRead];
      shared_ptr<PendingOutputs> *const outputs = futureOutputs[nextRead];

      // System.out.println("  cycle nextRead=" + nextRead + " nextWrite=" +
      // nextWrite + " inputSkipCount="+ inputSkipCount + " input.keepOrig=" +
      // input.keepOrig + " input.consumed=" + input.consumed + " input.state=" +
      // input.state);

      if (!input->consumed && (input->keepOrig || !input->matched)) {
        if (input->state != nullptr) {
          // Return a previously saved token (because we
          // had to lookahead):
          restoreState(input->state);
        } else {
          // Pass-through case: return token we just pulled
          // but didn't capture:
          assert((inputSkipCount == 1,
                  L"inputSkipCount=" + to_wstring(inputSkipCount) +
                      L" nextRead=" + to_wstring(nextRead)));
        }
        input->reset();
        if (outputs->count > 0) {
          outputs->posIncr = 0;
        } else {
          nextRead = rollIncr(nextRead);
          inputSkipCount--;
        }
        // System.out.println("  return token=" + termAtt.toString());
        return true;
      } else if (outputs->upto < outputs->count) {
        // Still have pending outputs to replay at this
        // position
        input->reset();
        constexpr int posIncr = outputs->posIncr;
        shared_ptr<CharsRef> *const output = outputs->pullNext();
        clearAttributes();
        termAtt->copyBuffer(output->chars, output->offset, output->length_);
        typeAtt->setType(TYPE_SYNONYM);
        int endOffset = outputs->getLastEndOffset();
        if (endOffset == -1) {
          endOffset = input->endOffset;
        }
        offsetAtt->setOffset(input->startOffset, endOffset);
        posIncrAtt->setPositionIncrement(posIncr);
        posLenAtt->setPositionLength(outputs->getLastPosLength());
        if (outputs->count == 0) {
          // Done with the buffered input and all outputs at
          // this position
          nextRead = rollIncr(nextRead);
          inputSkipCount--;
        }
        // System.out.println("  return token=" + termAtt.toString());
        return true;
      } else {
        // Done with the buffered input and all outputs at
        // this position
        input->reset();
        nextRead = rollIncr(nextRead);
        inputSkipCount--;
      }
    }

    if (finished && nextRead == nextWrite) {
      // End case: if any output syns went beyond end of
      // input stream, enumerate them now:
      shared_ptr<PendingOutputs> *const outputs = futureOutputs[nextRead];
      if (outputs->upto < outputs->count) {
        constexpr int posIncr = outputs->posIncr;
        shared_ptr<CharsRef> *const output = outputs->pullNext();
        futureInputs[nextRead]->reset();
        if (outputs->count == 0) {
          nextWrite = nextRead = rollIncr(nextRead);
        }
        clearAttributes();
        // Keep offset from last input token:
        offsetAtt->setOffset(lastStartOffset, lastEndOffset);
        termAtt->copyBuffer(output->chars, output->offset, output->length_);
        typeAtt->setType(TYPE_SYNONYM);
        // System.out.println("  set posIncr=" + outputs.posIncr + " outputs=" +
        // outputs);
        posIncrAtt->setPositionIncrement(posIncr);
        // System.out.println("  return token=" + termAtt.toString());
        return true;
      } else {
        return false;
      }
    }

    // Find new synonym matches:
    parse();
  }
}

void SynonymFilter::reset() 
{
  TokenFilter::reset();
  captureCount = 0;
  finished = false;
  inputSkipCount = 0;
  nextRead = nextWrite = 0;

  // In normal usage these resets would not be needed,
  // since they reset-as-they-are-consumed, but the app
  // may not consume all input tokens (or we might hit an
  // exception), in which case we have leftover state
  // here:
  for (auto input : futureInputs) {
    input->reset();
  }
  for (auto output : futureOutputs) {
    output->reset();
  }
}
} // namespace org::apache::lucene::analysis::synonym