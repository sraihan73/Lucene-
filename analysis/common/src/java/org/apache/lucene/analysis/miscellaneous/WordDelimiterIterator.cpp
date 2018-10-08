using namespace std;

#include "WordDelimiterIterator.h"

namespace org::apache::lucene::analysis::miscellaneous
{

std::deque<char> const WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE;

WordDelimiterIterator::StaticConstructor::StaticConstructor()
{
  std::deque<char> tab(256);
  for (int i = 0; i < 256; i++) {
    char code = 0;
    if (islower(i)) {
      code |= LOWER;
    } else if (isupper(i)) {
      code |= UPPER;
    } else if (isdigit(i)) {
      code |= DIGIT;
    }
    if (code == 0) {
      code = SUBWORD_DELIM;
    }
    tab[i] = code;
  }
  DEFAULT_WORD_DELIM_TABLE = tab;
}

WordDelimiterIterator::StaticConstructor
    WordDelimiterIterator::staticConstructor;

WordDelimiterIterator::WordDelimiterIterator(std::deque<char> &charTypeTable,
                                             bool splitOnCaseChange,
                                             bool splitOnNumerics,
                                             bool stemEnglishPossessive)
    : splitOnCaseChange(splitOnCaseChange), splitOnNumerics(splitOnNumerics),
      stemEnglishPossessive(stemEnglishPossessive), charTypeTable(charTypeTable)
{
}

int WordDelimiterIterator::next()
{
  current = end;
  if (current == DONE) {
    return DONE;
  }

  if (skipPossessive) {
    current += 2;
    skipPossessive = false;
  }

  int lastType = 0;

  while (current < endBounds &&
         (isSubwordDelim(lastType = charType(text[current])))) {
    current++;
  }

  if (current >= endBounds) {
    return end = DONE;
  }

  for (end = current + 1; end < endBounds; end++) {
    int type = charType(text[end]);
    if (isBreak(lastType, type)) {
      break;
    }
    lastType = type;
  }

  if (end < endBounds - 1 && endsWithPossessive(end + 2)) {
    skipPossessive = true;
  }

  return end;
}

int WordDelimiterIterator::type()
{
  if (end == DONE) {
    return 0;
  }

  int type = charType(text[current]);
  switch (type) {
  // return ALPHA word type for both lower and upper
  case LOWER:
  case UPPER:
    return ALPHA;
  default:
    return type;
  }
}

void WordDelimiterIterator::setText(std::deque<wchar_t> &text, int length)
{
  this->text = text;
  this->length = this->endBounds = length;
  current = startBounds = end = 0;
  skipPossessive = hasFinalPossessive = false;
  setBounds();
}

bool WordDelimiterIterator::isBreak(int lastType, int type)
{
  if ((type & lastType) != 0) {
    return false;
  }

  if (!splitOnCaseChange && isAlpha(lastType) && isAlpha(type)) {
    // ALPHA->ALPHA: always ignore if case isn't considered.
    return false;
  } else if (isUpper(lastType) && isAlpha(type)) {
    // UPPER->letter: Don't split
    return false;
  } else if (!splitOnNumerics && ((isAlpha(lastType) && isDigit(type)) ||
                                  (isDigit(lastType) && isAlpha(type)))) {
    // ALPHA->NUMERIC, NUMERIC->ALPHA :Don't split
    return false;
  }

  return true;
}

bool WordDelimiterIterator::isSingleWord()
{
  if (hasFinalPossessive) {
    return current == startBounds && end == endBounds - 2;
  } else {
    return current == startBounds && end == endBounds;
  }
}

void WordDelimiterIterator::setBounds()
{
  while (startBounds < length &&
         (isSubwordDelim(charType(text[startBounds])))) {
    startBounds++;
  }

  while (endBounds > startBounds &&
         (isSubwordDelim(charType(text[endBounds - 1])))) {
    endBounds--;
  }
  if (endsWithPossessive(endBounds)) {
    hasFinalPossessive = true;
  }
  current = startBounds;
}

bool WordDelimiterIterator::endsWithPossessive(int pos)
{
  return (stemEnglishPossessive && pos > 2 && text[pos - 2] == L'\'' &&
          (text[pos - 1] == L's' || text[pos - 1] == L'S') &&
          isAlpha(charType(text[pos - 3])) &&
          (pos == endBounds || isSubwordDelim(charType(text[pos]))));
}

int WordDelimiterIterator::charType(int ch)
{
  if (ch < charTypeTable.size()) {
    return charTypeTable[ch];
  }
  return getType(ch);
}

char WordDelimiterIterator::getType(int ch)
{
  switch (Character::getType(ch)) {
  case Character::UPPERCASE_LETTER:
    return UPPER;
  case Character::LOWERCASE_LETTER:
    return LOWER;

  case Character::TITLECASE_LETTER:
  case Character::MODIFIER_LETTER:
  case Character::OTHER_LETTER:
  case Character::NON_SPACING_MARK:
  case Character::ENCLOSING_MARK: // depends what it encloses?
  case Character::COMBINING_SPACING_MARK:
    return ALPHA;

  case Character::DECIMAL_DIGIT_NUMBER:
  case Character::LETTER_NUMBER:
  case Character::OTHER_NUMBER:
    return DIGIT;

    // case Character.SPACE_SEPARATOR:
    // case Character.LINE_SEPARATOR:
    // case Character.PARAGRAPH_SEPARATOR:
    // case Character.CONTROL:
    // case Character.FORMAT:
    // case Character.PRIVATE_USE:

  case Character::SURROGATE: // prevent splitting
    return ALPHA | DIGIT;

    // case Character.DASH_PUNCTUATION:
    // case Character.START_PUNCTUATION:
    // case Character.END_PUNCTUATION:
    // case Character.CONNECTOR_PUNCTUATION:
    // case Character.OTHER_PUNCTUATION:
    // case Character.MATH_SYMBOL:
    // case Character.CURRENCY_SYMBOL:
    // case Character.MODIFIER_SYMBOL:
    // case Character.OTHER_SYMBOL:
    // case Character.INITIAL_QUOTE_PUNCTUATION:
    // case Character.FINAL_QUOTE_PUNCTUATION:

  default:
    return SUBWORD_DELIM;
  }
}

bool WordDelimiterIterator::isAlpha(int type) { return (type & ALPHA) != 0; }

bool WordDelimiterIterator::isDigit(int type) { return (type & DIGIT) != 0; }

bool WordDelimiterIterator::isSubwordDelim(int type)
{
  return (type & SUBWORD_DELIM) != 0;
}

bool WordDelimiterIterator::isUpper(int type) { return (type & UPPER) != 0; }
} // namespace org::apache::lucene::analysis::miscellaneous