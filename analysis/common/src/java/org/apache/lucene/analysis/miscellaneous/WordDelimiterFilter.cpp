using namespace std;

#include "WordDelimiterFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
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
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;

WordDelimiterFilter::WordDelimiterFilter(shared_ptr<TokenStream> in_,
                                         std::deque<char> &charTypeTable,
                                         int configurationFlags,
                                         shared_ptr<CharArraySet> protWords)
    : org::apache::lucene::analysis::TokenFilter(in_), protWords(protWords),
      flags(configurationFlags),
      iterator(make_shared<WordDelimiterIterator>(
          charTypeTable, has(SPLIT_ON_CASE_CHANGE), has(SPLIT_ON_NUMERICS),
          has(STEM_ENGLISH_POSSESSIVE)))
{
}

WordDelimiterFilter::WordDelimiterFilter(shared_ptr<TokenStream> in_,
                                         int configurationFlags,
                                         shared_ptr<CharArraySet> protWords)
    : WordDelimiterFilter(in_, WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE,
                          configurationFlags, protWords)
{
}

bool WordDelimiterFilter::incrementToken() 
{
  while (true) {
    if (!hasSavedState) {
      // process a new input word
      if (!input->incrementToken()) {
        return false;
      }
      if (has(IGNORE_KEYWORDS) && keywordAttribute->isKeyword()) {
        return true;
      }
      int termLength = termAttribute->length();
      std::deque<wchar_t> termBuffer = termAttribute->buffer();

      accumPosInc += posIncAttribute->getPositionIncrement();

      iterator->setText(termBuffer, termLength);
      iterator->next();

      // word of no delimiters, or protected word: just return it
      if ((iterator->current == 0 && iterator->end == termLength) ||
          (protWords != nullptr &&
           protWords->contains(termBuffer, 0, termLength))) {
        posIncAttribute->setPositionIncrement(accumPosInc);
        accumPosInc = 0;
        first = false;
        return true;
      }

      // word of simply delimiters
      if (iterator->end == WordDelimiterIterator::DONE &&
          !has(PRESERVE_ORIGINAL)) {
        // if the posInc is 1, simply ignore it in the accumulation
        // TODO: proper hole adjustment (FilteringTokenFilter-like) instead of
        // this previous logic!
        if (posIncAttribute->getPositionIncrement() == 1 && !first) {
          accumPosInc--;
        }
        continue;
      }

      saveState();

      hasOutputToken = false;
      hasOutputFollowingOriginal = !has(PRESERVE_ORIGINAL);
      lastConcatCount = 0;

      if (has(PRESERVE_ORIGINAL)) {
        posIncAttribute->setPositionIncrement(accumPosInc);
        accumPosInc = 0;
        first = false;
        return true;
      }
    }

    // at the end of the string, output any concatenations
    if (iterator->end == WordDelimiterIterator::DONE) {
      if (!concat->isEmpty()) {
        if (flushConcatenation(concat)) {
          buffer();
          continue;
        }
      }

      if (!concatAll->isEmpty()) {
        // only if we haven't output this same combo above!
        if (concatAll->subwordCount > lastConcatCount) {
          concatAll->writeAndClear();
          buffer();
          continue;
        }
        concatAll->clear();
      }

      if (bufferedPos < bufferedLen) {
        if (bufferedPos == 0) {
          sorter->sort(0, bufferedLen);
        }
        clearAttributes();
        restoreState(buffered[bufferedPos++]);
        if (first && posIncAttribute->getPositionIncrement() == 0) {
          // can easily happen with strange combinations (e.g. not outputting
          // numbers, but concat-all)
          posIncAttribute->setPositionIncrement(1);
        }
        first = false;
        return true;
      }

      // no saved concatenations, on to the next input word
      bufferedPos = bufferedLen = 0;
      hasSavedState = false;
      continue;
    }

    // word surrounded by delimiters: always output
    if (iterator->isSingleWord()) {
      generatePart(true);
      iterator->next();
      first = false;
      return true;
    }

    int wordType = iterator->type();

    // do we already have queued up incompatible concatenations?
    if (!concat->isEmpty() && (concat->type & wordType) == 0) {
      if (flushConcatenation(concat)) {
        hasOutputToken = false;
        buffer();
        continue;
      }
      hasOutputToken = false;
    }

    // add subwords depending upon options
    if (shouldConcatenate(wordType)) {
      if (concat->isEmpty()) {
        concat->type = wordType;
      }
      concatenate(concat);
    }

    // add all subwords (catenateAll)
    if (has(CATENATE_ALL)) {
      concatenate(concatAll);
    }

    // if we should output the word or number part
    if (shouldGenerateParts(wordType)) {
      generatePart(false);
      buffer();
    }

    iterator->next();
  }
}

void WordDelimiterFilter::reset() 
{
  TokenFilter::reset();
  hasSavedState = false;
  concat->clear();
  concatAll->clear();
  accumPosInc = bufferedPos = bufferedLen = 0;
  first = true;
}

WordDelimiterFilter::OffsetSorter::OffsetSorter(
    shared_ptr<WordDelimiterFilter> outerInstance)
    : outerInstance(outerInstance)
{
}

int WordDelimiterFilter::OffsetSorter::compare(int i, int j)
{
  int cmp =
      Integer::compare(outerInstance->startOff[i], outerInstance->startOff[j]);
  if (cmp == 0) {
    cmp = Integer::compare(outerInstance->posInc[j], outerInstance->posInc[i]);
  }
  return cmp;
}

void WordDelimiterFilter::OffsetSorter::swap(int i, int j)
{
  shared_ptr<AttributeSource::State> tmp = outerInstance->buffered[i];
  outerInstance->buffered[i] = outerInstance->buffered[j];
  outerInstance->buffered[j] = tmp;

  int tmp2 = outerInstance->startOff[i];
  outerInstance->startOff[i] = outerInstance->startOff[j];
  outerInstance->startOff[j] = tmp2;

  tmp2 = outerInstance->posInc[i];
  outerInstance->posInc[i] = outerInstance->posInc[j];
  outerInstance->posInc[j] = tmp2;
}

void WordDelimiterFilter::buffer()
{
  if (bufferedLen == buffered.size()) {
    int newSize = ArrayUtil::oversize(bufferedLen + 1, 8);
    buffered = Arrays::copyOf(buffered, newSize);
    startOff = Arrays::copyOf(startOff, newSize);
    posInc = Arrays::copyOf(posInc, newSize);
  }
  startOff[bufferedLen] = offsetAttribute->startOffset();
  posInc[bufferedLen] = posIncAttribute->getPositionIncrement();
  buffered[bufferedLen] = captureState();
  bufferedLen++;
}

void WordDelimiterFilter::saveState()
{
  // otherwise, we have delimiters, save state
  savedStartOffset = offsetAttribute->startOffset();
  savedEndOffset = offsetAttribute->endOffset();
  // if length by start + end offsets doesn't match the term text then assume
  // this is a synonym and don't adjust the offsets.
  hasIllegalOffsets =
      (savedEndOffset - savedStartOffset != termAttribute->length());
  savedType = typeAttribute->type();

  if (savedBuffer.size() < termAttribute->length()) {
    savedBuffer = std::deque<wchar_t>(
        ArrayUtil::oversize(termAttribute->length(), Character::BYTES));
  }

  System::arraycopy(termAttribute->buffer(), 0, savedBuffer, 0,
                    termAttribute->length());
  iterator->text = savedBuffer;

  hasSavedState = true;
}

bool WordDelimiterFilter::flushConcatenation(
    shared_ptr<WordDelimiterConcatenation> concatenation)
{
  lastConcatCount = concatenation->subwordCount;
  if (concatenation->subwordCount != 1 ||
      !shouldGenerateParts(concatenation->type)) {
    concatenation->writeAndClear();
    return true;
  }
  concatenation->clear();
  return false;
}

bool WordDelimiterFilter::shouldConcatenate(int wordType)
{
  return (has(CATENATE_WORDS) && isAlpha(wordType)) ||
         (has(CATENATE_NUMBERS) && isDigit(wordType));
}

bool WordDelimiterFilter::shouldGenerateParts(int wordType)
{
  return (has(GENERATE_WORD_PARTS) && isAlpha(wordType)) ||
         (has(GENERATE_NUMBER_PARTS) && isDigit(wordType));
}

void WordDelimiterFilter::concatenate(
    shared_ptr<WordDelimiterConcatenation> concatenation)
{
  if (concatenation->isEmpty()) {
    concatenation->startOffset = savedStartOffset + iterator->current;
  }
  concatenation->append(savedBuffer, iterator->current,
                        iterator->end - iterator->current);
  concatenation->endOffset = savedStartOffset + iterator->end;
}

void WordDelimiterFilter::generatePart(bool isSingleWord)
{
  clearAttributes();
  termAttribute->copyBuffer(savedBuffer, iterator->current,
                            iterator->end - iterator->current);
  int startOffset = savedStartOffset + iterator->current;
  int endOffset = savedStartOffset + iterator->end;

  if (hasIllegalOffsets) {
    // historically this filter did this regardless for 'isSingleWord',
    // but we must do a sanity check:
    if (isSingleWord && startOffset <= savedEndOffset) {
      offsetAttribute->setOffset(startOffset, savedEndOffset);
    } else {
      offsetAttribute->setOffset(savedStartOffset, savedEndOffset);
    }
  } else {
    offsetAttribute->setOffset(startOffset, endOffset);
  }
  posIncAttribute->setPositionIncrement(position(false));
  typeAttribute->setType(savedType);
}

int WordDelimiterFilter::position(bool inject)
{
  int posInc = accumPosInc;

  if (hasOutputToken) {
    accumPosInc = 0;
    return inject ? 0 : max(1, posInc);
  }

  hasOutputToken = true;

  if (!hasOutputFollowingOriginal) {
    // the first token following the original is 0 regardless
    hasOutputFollowingOriginal = true;
    return 0;
  }
  // clear the accumulated position increment
  accumPosInc = 0;
  return max(1, posInc);
}

bool WordDelimiterFilter::isAlpha(int type) { return (type & ALPHA) != 0; }

bool WordDelimiterFilter::isDigit(int type) { return (type & DIGIT) != 0; }

bool WordDelimiterFilter::isSubwordDelim(int type)
{
  return (type & SUBWORD_DELIM) != 0;
}

bool WordDelimiterFilter::isUpper(int type) { return (type & UPPER) != 0; }

bool WordDelimiterFilter::has(int flag) { return (flags & flag) != 0; }

WordDelimiterFilter::WordDelimiterConcatenation::WordDelimiterConcatenation(
    shared_ptr<WordDelimiterFilter> outerInstance)
    : outerInstance(outerInstance)
{
}

void WordDelimiterFilter::WordDelimiterConcatenation::append(
    std::deque<wchar_t> &text, int offset, int length)
{
  buffer->append(text, offset, length);
  subwordCount++;
}

void WordDelimiterFilter::WordDelimiterConcatenation::write()
{
  outerInstance->clearAttributes();
  if (outerInstance->termAttribute->length() < buffer->length()) {
    outerInstance->termAttribute->resizeBuffer(buffer->length());
  }
  std::deque<wchar_t> termbuffer = outerInstance->termAttribute->buffer();

  buffer->getChars(0, buffer->length(), termbuffer, 0);
  outerInstance->termAttribute->setLength(buffer->length());

  if (outerInstance->hasIllegalOffsets) {
    outerInstance->offsetAttribute->setOffset(outerInstance->savedStartOffset,
                                              outerInstance->savedEndOffset);
  } else {
    outerInstance->offsetAttribute->setOffset(startOffset, endOffset);
  }
  outerInstance->posIncAttribute->setPositionIncrement(
      outerInstance->position(true));
  outerInstance->typeAttribute->setType(outerInstance->savedType);
  outerInstance->accumPosInc = 0;
}

bool WordDelimiterFilter::WordDelimiterConcatenation::isEmpty()
{
  return buffer->length() == 0;
}

void WordDelimiterFilter::WordDelimiterConcatenation::clear()
{
  buffer->setLength(0);
  startOffset = endOffset = type = subwordCount = 0;
}

void WordDelimiterFilter::WordDelimiterConcatenation::writeAndClear()
{
  write();
  clear();
}
} // namespace org::apache::lucene::analysis::miscellaneous