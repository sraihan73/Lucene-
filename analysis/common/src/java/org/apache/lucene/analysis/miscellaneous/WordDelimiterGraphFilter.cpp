using namespace std;

#include "WordDelimiterGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "WordDelimiterIterator.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

WordDelimiterGraphFilter::WordDelimiterGraphFilter(
    shared_ptr<TokenStream> in_, std::deque<char> &charTypeTable,
    int configurationFlags, shared_ptr<CharArraySet> protWords)
    : org::apache::lucene::analysis::TokenFilter(in_), protWords(protWords),
      flags(configurationFlags),
      iterator(make_shared<WordDelimiterIterator>(
          charTypeTable, has(SPLIT_ON_CASE_CHANGE), has(SPLIT_ON_NUMERICS),
          has(STEM_ENGLISH_POSSESSIVE)))
{
  if ((configurationFlags &
       ~(GENERATE_WORD_PARTS | GENERATE_NUMBER_PARTS | CATENATE_WORDS |
         CATENATE_NUMBERS | CATENATE_ALL | PRESERVE_ORIGINAL |
         SPLIT_ON_CASE_CHANGE | SPLIT_ON_NUMERICS | STEM_ENGLISH_POSSESSIVE |
         IGNORE_KEYWORDS)) != 0) {
    throw invalid_argument(L"flags contains unrecognized flag: " +
                           to_wstring(configurationFlags));
  }
}

WordDelimiterGraphFilter::WordDelimiterGraphFilter(
    shared_ptr<TokenStream> in_, int configurationFlags,
    shared_ptr<CharArraySet> protWords)
    : WordDelimiterGraphFilter(in_,
                               WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE,
                               configurationFlags, protWords)
{
}

void WordDelimiterGraphFilter::bufferWordParts() 
{

  saveState();

  // if length by start + end offsets doesn't match the term's text then set
  // offsets for all our word parts/concats to the incoming offsets.  this can
  // happen if WDGF is applied to an injected synonym, or to a stem'd form, etc:
  hasIllegalOffsets = (savedEndOffset - savedStartOffset != savedTermLength);

  bufferedLen = 0;
  lastConcatCount = 0;
  wordPos = 0;

  if (iterator->isSingleWord()) {
    buffer(wordPos, wordPos + 1, iterator->current, iterator->end);
    wordPos++;
    iterator->next();
  } else {

    // iterate all words parts, possibly buffering them, building up
    // concatenations and possibly buffering them too:
    while (iterator->end != WordDelimiterIterator::DONE) {
      int wordType = iterator->type();

      // do we already have queued up incompatible concatenations?
      if (concat->isNotEmpty() && (concat->type & wordType) == 0) {
        flushConcatenation(concat);
      }

      // add subwords depending upon options
      if (shouldConcatenate(wordType)) {
        concatenate(concat);
      }

      // add all subwords (catenateAll)
      if (has(CATENATE_ALL)) {
        concatenate(concatAll);
      }

      // if we should output the word or number part
      if (shouldGenerateParts(wordType)) {
        buffer(wordPos, wordPos + 1, iterator->current, iterator->end);
        wordPos++;
      }
      iterator->next();
    }

    if (concat->isNotEmpty()) {
      // flush final concatenation
      flushConcatenation(concat);
    }

    if (concatAll->isNotEmpty()) {
      // only if we haven't output this same combo above, e.g. PowerShot with
      // CATENATE_WORDS:
      if (concatAll->subwordCount > lastConcatCount) {
        if (wordPos == concatAll->startPos) {
          // we are not generating parts, so we must advance wordPos now
          wordPos++;
        }
        concatAll->write();
      }
      concatAll->clear();
    }
  }

  if (has(PRESERVE_ORIGINAL)) {
    if (wordPos == 0) {
      // can happen w/ strange flag combos and inputs :)
      wordPos++;
    }
    // add the original token now so that we can set the correct end position
    buffer(0, wordPos, 0, savedTermLength);
  }

  sorter->sort(0, bufferedLen);
  wordPos = 0;

  // set back to 0 for iterating from the buffer
  bufferedPos = 0;
}

bool WordDelimiterGraphFilter::incrementToken() 
{
  while (true) {
    if (savedState == nullptr) {

      // process a new input token
      if (input->incrementToken() == false) {
        return false;
      }
      if (has(IGNORE_KEYWORDS) && keywordAttribute->isKeyword()) {
        return true;
      }
      int termLength = termAttribute->length();
      std::deque<wchar_t> termBuffer = termAttribute->buffer();

      accumPosInc += posIncAttribute->getPositionIncrement();

      // iterate & cache all word parts up front:
      iterator->setText(termBuffer, termLength);
      iterator->next();

      // word of no delimiters, or protected word: just return it
      if ((iterator->current == 0 && iterator->end == termLength) ||
          (protWords != nullptr &&
           protWords->contains(termBuffer, 0, termLength))) {
        posIncAttribute->setPositionIncrement(accumPosInc);
        accumPosInc = 0;
        return true;
      }

      // word of simply delimiters: swallow this token, creating a hole, and
      // move on to next token
      if (iterator->end == WordDelimiterIterator::DONE) {
        if (has(PRESERVE_ORIGINAL) == false) {
          continue;
        } else {
          return true;
        }
      }

      // otherwise, we have delimiters, process & buffer all parts:
      bufferWordParts();
    }

    if (bufferedPos < bufferedLen) {
      clearAttributes();
      restoreState(savedState);

      std::deque<wchar_t> termPart = bufferedTermParts[bufferedPos];
      int startPos = bufferedParts[4 * bufferedPos];
      int endPos = bufferedParts[4 * bufferedPos + 1];
      int startPart = bufferedParts[4 * bufferedPos + 2];
      int endPart = bufferedParts[4 * bufferedPos + 3];
      bufferedPos++;

      int startOffset;
      int endOffset;

      if (hasIllegalOffsets) {
        startOffset = savedStartOffset;
        endOffset = savedEndOffset;
      } else {
        startOffset = savedStartOffset + startPart;
        endOffset = savedStartOffset + endPart;
      }

      // never let offsets go backwards:
      startOffset = max(startOffset, lastStartOffset);
      endOffset = max(endOffset, lastStartOffset);

      offsetAttribute->setOffset(startOffset, endOffset);
      lastStartOffset = startOffset;

      if (termPart.empty()) {
        termAttribute->copyBuffer(savedTermBuffer, startPart,
                                  endPart - startPart);
      } else {
        termAttribute->copyBuffer(termPart, 0, termPart.size());
      }

      posIncAttribute->setPositionIncrement(accumPosInc + startPos - wordPos);
      accumPosInc = 0;
      posLenAttribute->setPositionLength(endPos - startPos);
      wordPos = startPos;
      return true;
    }

    // no saved concatenations, on to the next input word
    savedState.reset();
  }
}

void WordDelimiterGraphFilter::reset() 
{
  TokenFilter::reset();
  accumPosInc = 0;
  savedState.reset();
  lastStartOffset = 0;
  concat->clear();
  concatAll->clear();
}

WordDelimiterGraphFilter::PositionSorter::PositionSorter(
    shared_ptr<WordDelimiterGraphFilter> outerInstance)
    : outerInstance(outerInstance)
{
}

int WordDelimiterGraphFilter::PositionSorter::compare(int i, int j)
{
  // sort by smaller start position
  int iPosStart = outerInstance->bufferedParts[4 * i];
  int jPosStart = outerInstance->bufferedParts[4 * j];
  int cmp = Integer::compare(iPosStart, jPosStart);
  if (cmp != 0) {
    return cmp;
  }

  // tie break by longest pos length:
  int iPosEnd = outerInstance->bufferedParts[4 * i + 1];
  int jPosEnd = outerInstance->bufferedParts[4 * j + 1];
  return Integer::compare(jPosEnd, iPosEnd);
}

void WordDelimiterGraphFilter::PositionSorter::swap(int i, int j)
{
  int iOffset = 4 * i;
  int jOffset = 4 * j;
  for (int x = 0; x < 4; x++) {
    int tmp = outerInstance->bufferedParts[iOffset + x];
    outerInstance->bufferedParts[iOffset + x] =
        outerInstance->bufferedParts[jOffset + x];
    outerInstance->bufferedParts[jOffset + x] = tmp;
  }

  std::deque<wchar_t> tmp2 = outerInstance->bufferedTermParts[i];
  outerInstance->bufferedTermParts[i] = outerInstance->bufferedTermParts[j];
  outerInstance->bufferedTermParts[j] = tmp2;
}

void WordDelimiterGraphFilter::buffer(int startPos, int endPos, int startPart,
                                      int endPart)
{
  buffer(nullptr, startPos, endPos, startPart, endPart);
}

void WordDelimiterGraphFilter::buffer(std::deque<wchar_t> &termPart,
                                      int startPos, int endPos, int startPart,
                                      int endPart)
{
  /*
  System.out.println("buffer: pos=" + startPos + "-" + endPos + " part=" +
  startPart + "-" + endPart); if (termPart != null) { System.out.println("
  termIn=" + new std::wstring(termPart)); } else { System.out.println("  term=" + new
  std::wstring(savedTermBuffer, startPart, endPart-startPart));
  }
  */
  assert((endPos > startPos, L"startPos=" + to_wstring(startPos) + L" endPos=" +
                                 to_wstring(endPos)));
  assert((endPart > startPart ||
              (endPart == 0 && startPart == 0 && savedTermLength == 0),
          L"startPart=" + to_wstring(startPart) + L" endPart=" +
              to_wstring(endPart)));
  if ((bufferedLen + 1) * 4 > bufferedParts.size()) {
    bufferedParts = ArrayUtil::grow(bufferedParts, (bufferedLen + 1) * 4);
  }
  if (bufferedTermParts.size() == bufferedLen) {
    int newSize = ArrayUtil::oversize(bufferedLen + 1,
                                      RamUsageEstimator::NUM_BYTES_OBJECT_REF);
    std::deque<std::deque<wchar_t>> newArray(newSize);
    System::arraycopy(bufferedTermParts, 0, newArray, 0,
                      bufferedTermParts.size());
    bufferedTermParts = newArray;
  }
  bufferedTermParts[bufferedLen] = termPart;
  bufferedParts[bufferedLen * 4] = startPos;
  bufferedParts[bufferedLen * 4 + 1] = endPos;
  bufferedParts[bufferedLen * 4 + 2] = startPart;
  bufferedParts[bufferedLen * 4 + 3] = endPart;
  bufferedLen++;
}

void WordDelimiterGraphFilter::saveState()
{
  savedTermLength = termAttribute->length();
  savedStartOffset = offsetAttribute->startOffset();
  savedEndOffset = offsetAttribute->endOffset();
  savedState = captureState();

  if (savedTermBuffer.size() < savedTermLength) {
    savedTermBuffer = std::deque<wchar_t>(
        ArrayUtil::oversize(savedTermLength, Character::BYTES));
  }

  System::arraycopy(termAttribute->buffer(), 0, savedTermBuffer, 0,
                    savedTermLength);
}

void WordDelimiterGraphFilter::flushConcatenation(
    shared_ptr<WordDelimiterConcatenation> concat)
{
  if (wordPos == concat->startPos) {
    // we are not generating parts, so we must advance wordPos now
    wordPos++;
  }
  lastConcatCount = concat->subwordCount;
  if (concat->subwordCount != 1 || shouldGenerateParts(concat->type) == false) {
    concat->write();
  }
  concat->clear();
}

bool WordDelimiterGraphFilter::shouldConcatenate(int wordType)
{
  return (has(CATENATE_WORDS) && WordDelimiterIterator::isAlpha(wordType)) ||
         (has(CATENATE_NUMBERS) && WordDelimiterIterator::isDigit(wordType));
}

bool WordDelimiterGraphFilter::shouldGenerateParts(int wordType)
{
  return (has(GENERATE_WORD_PARTS) &&
          WordDelimiterIterator::isAlpha(wordType)) ||
         (has(GENERATE_NUMBER_PARTS) &&
          WordDelimiterIterator::isDigit(wordType));
}

void WordDelimiterGraphFilter::concatenate(
    shared_ptr<WordDelimiterConcatenation> concatenation)
{
  if (concatenation->isEmpty()) {
    concatenation->type = iterator->type();
    concatenation->startPart = iterator->current;
    concatenation->startPos = wordPos;
  }
  concatenation->append(savedTermBuffer, iterator->current,
                        iterator->end - iterator->current);
  concatenation->endPart = iterator->end;
}

bool WordDelimiterGraphFilter::has(int flag) { return (flags & flag) != 0; }

WordDelimiterGraphFilter::WordDelimiterConcatenation::
    WordDelimiterConcatenation(
        shared_ptr<WordDelimiterGraphFilter> outerInstance)
    : outerInstance(outerInstance)
{
}

void WordDelimiterGraphFilter::WordDelimiterConcatenation::append(
    std::deque<wchar_t> &text, int offset, int length)
{
  buffer->append(text, offset, length);
  subwordCount++;
}

void WordDelimiterGraphFilter::WordDelimiterConcatenation::write()
{
  std::deque<wchar_t> termPart(buffer->length());
  buffer->getChars(0, buffer->length(), termPart, 0);
  outerInstance->buffer(termPart, startPos, outerInstance->wordPos, startPart,
                        endPart);
}

bool WordDelimiterGraphFilter::WordDelimiterConcatenation::isEmpty()
{
  return buffer->length() == 0;
}

bool WordDelimiterGraphFilter::WordDelimiterConcatenation::isNotEmpty()
{
  return isEmpty() == false;
}

void WordDelimiterGraphFilter::WordDelimiterConcatenation::clear()
{
  buffer->setLength(0);
  startPart = endPart = type = subwordCount = 0;
}

wstring WordDelimiterGraphFilter::flagsToString(int flags)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  if ((flags & GENERATE_WORD_PARTS) != 0) {
    b->append(L"GENERATE_WORD_PARTS");
  }
  if ((flags & GENERATE_NUMBER_PARTS) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"GENERATE_NUMBER_PARTS");
  }
  if ((flags & CATENATE_WORDS) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"CATENATE_WORDS");
  }
  if ((flags & CATENATE_NUMBERS) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"CATENATE_NUMBERS");
  }
  if ((flags & CATENATE_ALL) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"CATENATE_ALL");
  }
  if ((flags & PRESERVE_ORIGINAL) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"PRESERVE_ORIGINAL");
  }
  if ((flags & SPLIT_ON_CASE_CHANGE) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"SPLIT_ON_CASE_CHANGE");
  }
  if ((flags & SPLIT_ON_NUMERICS) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"SPLIT_ON_NUMERICS");
  }
  if ((flags & STEM_ENGLISH_POSSESSIVE) != 0) {
    if (b->length() > 0) {
      b->append(L" | ");
    }
    b->append(L"STEM_ENGLISH_POSSESSIVE");
  }

  return b->toString();
}

wstring WordDelimiterGraphFilter::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"WordDelimiterGraphFilter(flags=");
  b->append(flagsToString(flags));
  b->append(L')');
  return b->toString();
}
} // namespace org::apache::lucene::analysis::miscellaneous