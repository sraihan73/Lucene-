using namespace std;

#include "CJKBigramFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"

namespace org::apache::lucene::analysis::cjk
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
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
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
const wstring CJKBigramFilter::DOUBLE_TYPE = L"<DOUBLE>";
const wstring CJKBigramFilter::SINGLE_TYPE = L"<SINGLE>";
const wstring CJKBigramFilter::HAN_TYPE =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::
             IDEOGRAPHIC];
const wstring CJKBigramFilter::HIRAGANA_TYPE =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::HIRAGANA];
const wstring CJKBigramFilter::KATAKANA_TYPE =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::KATAKANA];
const wstring CJKBigramFilter::HANGUL_TYPE =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::HANGUL];
const any CJKBigramFilter::NO = nullptr;

CJKBigramFilter::CJKBigramFilter(shared_ptr<TokenStream> in_)
    : CJKBigramFilter(in_, HAN | HIRAGANA | KATAKANA | HANGUL)
{
}

CJKBigramFilter::CJKBigramFilter(shared_ptr<TokenStream> in_, int flags)
    : CJKBigramFilter(in_, flags, false)
{
}

CJKBigramFilter::CJKBigramFilter(shared_ptr<TokenStream> in_, int flags,
                                 bool outputUnigrams)
    : org::apache::lucene::analysis::TokenFilter(in_),
      doHan((flags & HAN) == 0 ? NO : HAN_TYPE),
      doHiragana((flags & HIRAGANA) == 0 ? NO : HIRAGANA_TYPE),
      doKatakana((flags & KATAKANA) == 0 ? NO : KATAKANA_TYPE),
      doHangul((flags & HANGUL) == 0 ? NO : HANGUL_TYPE),
      outputUnigrams(outputUnigrams)
{
}

bool CJKBigramFilter::incrementToken() 
{
  while (true) {
    if (hasBufferedBigram()) {

      // case 1: we have multiple remaining codepoints buffered,
      // so we can emit a bigram here.

      if (outputUnigrams) {

        // when also outputting unigrams, we output the unigram first,
        // then rewind back to revisit the bigram.
        // so an input of ABC is A + (rewind)AB + B + (rewind)BC + C
        // the logic in hasBufferedUnigram ensures we output the C,
        // even though it did actually have adjacent CJK characters.

        if (ngramState) {
          flushBigram();
        } else {
          flushUnigram();
          index--;
        }
        ngramState = !ngramState;
      } else {
        flushBigram();
      }
      return true;
    } else if (doNext()) {

      // case 2: look at the token type. should we form any n-grams?

      wstring type = typeAtt->type();
      if (type == doHan || type == doHiragana || type == doKatakana ||
          type == doHangul) {

        // acceptable CJK type: we form n-grams from these.
        // as long as the offsets are aligned, we just add these to our current
        // buffer. otherwise, we clear the buffer and start over.

        if (offsetAtt->startOffset() !=
            lastEndOffset) { // unaligned, clear queue
          if (hasBufferedUnigram()) {

            // we have a buffered unigram, and we peeked ahead to see if we
            // could form a bigram, but we can't, because the offsets are
            // unaligned. capture the state of this peeked data to be revisited
            // next time thru the loop, and dump our unigram.

            loneState = captureState();
            flushUnigram();
            return true;
          }
          index = 0;
          bufferLen = 0;
        }
        refill();
      } else {

        // not a CJK type: we just return these as-is.

        if (hasBufferedUnigram()) {

          // we have a buffered unigram, and we peeked ahead to see if we could
          // form a bigram, but we can't, because it's not a CJK type. capture
          // the state of this peeked data to be revisited next time thru the
          // loop, and dump our unigram.

          loneState = captureState();
          flushUnigram();
          return true;
        }
        return true;
      }
    } else {

      // case 3: we have only zero or 1 codepoints buffered,
      // so not enough to form a bigram. But, we also have no
      // more input. So if we have a buffered codepoint, emit
      // a unigram, otherwise, it's end of stream.

      if (hasBufferedUnigram()) {
        flushUnigram(); // flush our remaining unigram
        return true;
      }
      return false;
    }
  }
}

bool CJKBigramFilter::doNext() 
{
  if (loneState != nullptr) {
    restoreState(loneState);
    loneState.reset();
    return true;
  } else {
    if (exhausted) {
      return false;
    } else if (input->incrementToken()) {
      return true;
    } else {
      exhausted = true;
      return false;
    }
  }
}

void CJKBigramFilter::refill()
{
  // compact buffers to keep them smallish if they become large
  // just a safety check, but technically we only need the last codepoint
  if (bufferLen > 64) {
    int last = bufferLen - 1;
    buffer[0] = buffer[last];
    startOffset[0] = startOffset[last];
    endOffset[0] = endOffset[last];
    bufferLen = 1;
    index -= last;
  }

  std::deque<wchar_t> termBuffer = termAtt->buffer();
  int len = termAtt->length();
  int start = offsetAtt->startOffset();
  int end = offsetAtt->endOffset();

  int newSize = bufferLen + len;
  buffer = ArrayUtil::grow(buffer, newSize);
  startOffset = ArrayUtil::grow(startOffset, newSize);
  endOffset = ArrayUtil::grow(endOffset, newSize);
  lastEndOffset = end;

  if (end - start != len) {
    // crazy offsets (modified by synonym or charfilter): just preserve
    for (int i = 0, cp = 0; i < len; i += Character::charCount(cp)) {
      cp = buffer[bufferLen] = Character::codePointAt(termBuffer, i, len);
      startOffset[bufferLen] = start;
      endOffset[bufferLen] = end;
      bufferLen++;
    }
  } else {
    // normal offsets
    for (int i = 0, cp = 0, cpLen = 0; i < len; i += cpLen) {
      cp = buffer[bufferLen] = Character::codePointAt(termBuffer, i, len);
      cpLen = Character::charCount(cp);
      startOffset[bufferLen] = start;
      start = endOffset[bufferLen] = start + cpLen;
      bufferLen++;
    }
  }
}

void CJKBigramFilter::flushBigram()
{
  clearAttributes();
  std::deque<wchar_t> termBuffer = termAtt->resizeBuffer(
      4); // maximum bigram length in code units (2 supplementaries)
  int len1 = Character::toChars(buffer[index], termBuffer, 0);
  int len2 = len1 + Character::toChars(buffer[index + 1], termBuffer, len1);
  termAtt->setLength(len2);
  offsetAtt->setOffset(startOffset[index], endOffset[index + 1]);
  typeAtt->setType(DOUBLE_TYPE);
  // when outputting unigrams, all bigrams are synonyms that span two unigrams
  if (outputUnigrams) {
    posIncAtt->setPositionIncrement(0);
    posLengthAtt->setPositionLength(2);
  }
  index++;
}

void CJKBigramFilter::flushUnigram()
{
  clearAttributes();
  std::deque<wchar_t> termBuffer =
      termAtt->resizeBuffer(2); // maximum unigram length (2 surrogates)
  int len = Character::toChars(buffer[index], termBuffer, 0);
  termAtt->setLength(len);
  offsetAtt->setOffset(startOffset[index], endOffset[index]);
  typeAtt->setType(SINGLE_TYPE);
  index++;
}

bool CJKBigramFilter::hasBufferedBigram() { return bufferLen - index > 1; }

bool CJKBigramFilter::hasBufferedUnigram()
{
  if (outputUnigrams) {
    // when outputting unigrams always
    return bufferLen - index == 1;
  } else {
    // otherwise it's only when we have a lone CJK character
    return bufferLen == 1 && index == 0;
  }
}

void CJKBigramFilter::reset() 
{
  TokenFilter::reset();
  bufferLen = 0;
  index = 0;
  lastEndOffset = 0;
  loneState.reset();
  exhausted = false;
  ngramState = false;
}
} // namespace org::apache::lucene::analysis::cjk