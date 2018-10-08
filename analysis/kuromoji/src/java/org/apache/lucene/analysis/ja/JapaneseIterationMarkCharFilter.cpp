using namespace std;

#include "JapaneseIterationMarkCharFilter.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/RollingCharBuffer.h"

namespace org::apache::lucene::analysis::ja
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
using RollingCharBuffer =
    org::apache::lucene::analysis::util::RollingCharBuffer;
std::deque<wchar_t> JapaneseIterationMarkCharFilter::h2d(50);
std::deque<wchar_t> JapaneseIterationMarkCharFilter::k2d(50);

JapaneseIterationMarkCharFilter::StaticConstructor::StaticConstructor()
{
  // Hiragana dakuten map_obj
  h2d[0] = L'\u304c';  // か => が
  h2d[1] = L'\u304c';  // が => が
  h2d[2] = L'\u304e';  // き => ぎ
  h2d[3] = L'\u304e';  // ぎ => ぎ
  h2d[4] = L'\u3050';  // く => ぐ
  h2d[5] = L'\u3050';  // ぐ => ぐ
  h2d[6] = L'\u3052';  // け => げ
  h2d[7] = L'\u3052';  // げ => げ
  h2d[8] = L'\u3054';  // こ => ご
  h2d[9] = L'\u3054';  // ご => ご
  h2d[10] = L'\u3056'; // さ => ざ
  h2d[11] = L'\u3056'; // ざ => ざ
  h2d[12] = L'\u3058'; // し => じ
  h2d[13] = L'\u3058'; // じ => じ
  h2d[14] = L'\u305a'; // す => ず
  h2d[15] = L'\u305a'; // ず => ず
  h2d[16] = L'\u305c'; // せ => ぜ
  h2d[17] = L'\u305c'; // ぜ => ぜ
  h2d[18] = L'\u305e'; // そ => ぞ
  h2d[19] = L'\u305e'; // ぞ => ぞ
  h2d[20] = L'\u3060'; // た => だ
  h2d[21] = L'\u3060'; // だ => だ
  h2d[22] = L'\u3062'; // ち => ぢ
  h2d[23] = L'\u3062'; // ぢ => ぢ
  h2d[24] = L'\u3063';
  h2d[25] = L'\u3065'; // つ => づ
  h2d[26] = L'\u3065'; // づ => づ
  h2d[27] = L'\u3067'; // て => で
  h2d[28] = L'\u3067'; // で => で
  h2d[29] = L'\u3069'; // と => ど
  h2d[30] = L'\u3069'; // ど => ど
  h2d[31] = L'\u306a';
  h2d[32] = L'\u306b';
  h2d[33] = L'\u306c';
  h2d[34] = L'\u306d';
  h2d[35] = L'\u306e';
  h2d[36] = L'\u3070'; // は => ば
  h2d[37] = L'\u3070'; // ば => ば
  h2d[38] = L'\u3071';
  h2d[39] = L'\u3073'; // ひ => び
  h2d[40] = L'\u3073'; // び => び
  h2d[41] = L'\u3074';
  h2d[42] = L'\u3076'; // ふ => ぶ
  h2d[43] = L'\u3076'; // ぶ => ぶ
  h2d[44] = L'\u3077';
  h2d[45] = L'\u3079'; // へ => べ
  h2d[46] = L'\u3079'; // べ => べ
  h2d[47] = L'\u307a';
  h2d[48] = L'\u307c'; // ほ => ぼ
  h2d[49] = L'\u307c'; // ぼ => ぼ

  // Make katakana dakuten map_obj from hiragana map_obj
  wchar_t codePointDifference = L'\u30ab' - L'\u304b'; // カ - か
  assert(h2d.size() == k2d.size());
  for (int i = 0; i < k2d.size(); i++) {
    k2d[i] = static_cast<wchar_t>(h2d[i] + codePointDifference);
  }
}

JapaneseIterationMarkCharFilter::StaticConstructor
    JapaneseIterationMarkCharFilter::staticConstructor;

JapaneseIterationMarkCharFilter::JapaneseIterationMarkCharFilter(
    shared_ptr<Reader> input)
    : JapaneseIterationMarkCharFilter(input, NORMALIZE_KANJI_DEFAULT,
                                      NORMALIZE_KANA_DEFAULT)
{
}

JapaneseIterationMarkCharFilter::JapaneseIterationMarkCharFilter(
    shared_ptr<Reader> input, bool normalizeKanji, bool normalizeKana)
    : org::apache::lucene::analysis::CharFilter(input)
{
  this->normalizeKanji = normalizeKanji;
  this->normalizeKana = normalizeKana;
  buffer->reset(input);
}

int JapaneseIterationMarkCharFilter::read(std::deque<wchar_t> &buffer,
                                          int offset,
                                          int length) 
{
  int read = 0;

  for (int i = offset; i < offset + length; i++) {
    int c = this->read();
    if (c == -1) {
      break;
    }
    buffer[i] = static_cast<wchar_t>(c);
    read++;
  }

  return read == 0 ? -1 : read;
}

int JapaneseIterationMarkCharFilter::read() 
{
  int ic = buffer->get(bufferPosition);

  // End of input
  if (ic == -1) {
    buffer->freeBefore(bufferPosition);
    return ic;
  }

  wchar_t c = static_cast<wchar_t>(ic);

  // Skip surrogate pair characters
  if (Character::isHighSurrogate(c) || Character::isLowSurrogate(c)) {
    iterationMarkSpanEndPosition = bufferPosition + 1;
  }

  // Free rolling buffer on full stop
  if (c == FULL_STOP_PUNCTUATION) {
    buffer->freeBefore(bufferPosition);
    iterationMarkSpanEndPosition = bufferPosition + 1;
  }

  // Normalize iteration mark
  if (isIterationMark(c)) {
    c = normalizeIterationMark(c);
  }

  bufferPosition++;
  return c;
}

wchar_t JapaneseIterationMarkCharFilter::normalizeIterationMark(
    wchar_t c) 
{

  // Case 1: Inside an iteration mark span
  if (bufferPosition < iterationMarkSpanEndPosition) {
    return normalize(sourceCharacter(bufferPosition, iterationMarksSpanSize),
                     c);
  }

  // Case 2: New iteration mark spans starts where the previous one ended, which
  // is illegal
  if (bufferPosition == iterationMarkSpanEndPosition) {
    // Emit the illegal iteration mark and increase end position to indicate
    // that we can't start a new span on the next position either
    iterationMarkSpanEndPosition++;
    return c;
  }

  // Case 3: New iteration mark span
  iterationMarksSpanSize = nextIterationMarkSpanSize();
  iterationMarkSpanEndPosition = bufferPosition + iterationMarksSpanSize;
  return normalize(sourceCharacter(bufferPosition, iterationMarksSpanSize), c);
}

int JapaneseIterationMarkCharFilter::nextIterationMarkSpanSize() throw(
    IOException)
{
  int spanSize = 0;
  for (int i = bufferPosition;
       buffer->get(i) != -1 &&
       isIterationMark(static_cast<wchar_t>(buffer->get(i)));
       i++) {
    spanSize++;
  }
  // Restrict span size so that we don't go past the previous end position
  if (bufferPosition - spanSize < iterationMarkSpanEndPosition) {
    spanSize = bufferPosition - iterationMarkSpanEndPosition;
  }
  return spanSize;
}

wchar_t JapaneseIterationMarkCharFilter::sourceCharacter(
    int position, int spanSize) 
{
  return static_cast<wchar_t>(buffer->get(position - spanSize));
}

wchar_t JapaneseIterationMarkCharFilter::normalize(wchar_t c, wchar_t m)
{
  if (isHiraganaIterationMark(m)) {
    return normalizedHiragana(c, m);
  }

  if (isKatakanaIterationMark(m)) {
    return normalizedKatakana(c, m);
  }

  return c; // If m is not kana and we are to normalize it, we assume it is
            // kanji and simply return it
}

wchar_t JapaneseIterationMarkCharFilter::normalizedHiragana(wchar_t c,
                                                            wchar_t m)
{
  switch (m) {
  case HIRAGANA_ITERATION_MARK:
    return isHiraganaDakuten(c) ? static_cast<wchar_t>(c - 1) : c;
  case HIRAGANA_VOICED_ITERATION_MARK:
    return lookupHiraganaDakuten(c);
  default:
    return c;
  }
}

wchar_t JapaneseIterationMarkCharFilter::normalizedKatakana(wchar_t c,
                                                            wchar_t m)
{
  switch (m) {
  case KATAKANA_ITERATION_MARK:
    return isKatakanaDakuten(c) ? static_cast<wchar_t>(c - 1) : c;
  case KATAKANA_VOICED_ITERATION_MARK:
    return lookupKatakanaDakuten(c);
  default:
    return c;
  }
}

bool JapaneseIterationMarkCharFilter::isIterationMark(wchar_t c)
{
  return isKanjiIterationMark(c) || isHiraganaIterationMark(c) ||
         isKatakanaIterationMark(c);
}

bool JapaneseIterationMarkCharFilter::isHiraganaIterationMark(wchar_t c)
{
  if (normalizeKana) {
    return c == HIRAGANA_ITERATION_MARK || c == HIRAGANA_VOICED_ITERATION_MARK;
  } else {
    return false;
  }
}

bool JapaneseIterationMarkCharFilter::isKatakanaIterationMark(wchar_t c)
{
  if (normalizeKana) {
    return c == KATAKANA_ITERATION_MARK || c == KATAKANA_VOICED_ITERATION_MARK;
  } else {
    return false;
  }
}

bool JapaneseIterationMarkCharFilter::isKanjiIterationMark(wchar_t c)
{
  if (normalizeKanji) {
    return c == KANJI_ITERATION_MARK;
  } else {
    return false;
  }
}

wchar_t JapaneseIterationMarkCharFilter::lookupHiraganaDakuten(wchar_t c)
{
  return lookup(c, h2d, L'\u304b'); // Code point is for か
}

wchar_t JapaneseIterationMarkCharFilter::lookupKatakanaDakuten(wchar_t c)
{
  return lookup(c, k2d, L'\u30ab'); // Code point is for カ
}

bool JapaneseIterationMarkCharFilter::isHiraganaDakuten(wchar_t c)
{
  return inside(c, h2d, L'\u304b') && c == lookupHiraganaDakuten(c);
}

bool JapaneseIterationMarkCharFilter::isKatakanaDakuten(wchar_t c)
{
  return inside(c, k2d, L'\u30ab') && c == lookupKatakanaDakuten(c);
}

wchar_t JapaneseIterationMarkCharFilter::lookup(wchar_t c,
                                                std::deque<wchar_t> &map_obj,
                                                wchar_t offset)
{
  if (!inside(c, map_obj, offset)) {
    return c;
  } else {
    return map_obj[c - offset];
  }
}

bool JapaneseIterationMarkCharFilter::inside(wchar_t c,
                                             std::deque<wchar_t> &map_obj,
                                             wchar_t offset)
{
  return c >= offset && c < offset + map_obj.size();
}

int JapaneseIterationMarkCharFilter::correct(int currentOff)
{
  return currentOff; // this filter doesn't change the length of strings
}
} // namespace org::apache::lucene::analysis::ja