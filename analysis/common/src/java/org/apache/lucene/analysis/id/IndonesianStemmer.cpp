using namespace std;

#include "IndonesianStemmer.h"

namespace org::apache::lucene::analysis::id
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int IndonesianStemmer::stem(std::deque<wchar_t> &text, int length,
                            bool stemDerivational)
{
  flags = 0;
  numSyllables = 0;
  for (int i = 0; i < length; i++) {
    if (isVowel(text[i])) {
      numSyllables++;
    }
  }

  if (numSyllables > 2) {
    length = removeParticle(text, length);
  }
  if (numSyllables > 2) {
    length = removePossessivePronoun(text, length);
  }

  if (stemDerivational) {
    length = this->stemDerivational(text, length);
  }
  return length;
}

int IndonesianStemmer::stemDerivational(std::deque<wchar_t> &text, int length)
{
  int oldLength = length;
  if (numSyllables > 2) {
    length = removeFirstOrderPrefix(text, length);
  }
  if (oldLength != length) { // a rule is fired
    oldLength = length;
    if (numSyllables > 2) {
      length = removeSuffix(text, length);
    }
    if (oldLength != length) // a rule is fired
    {
      if (numSyllables > 2) {
        length = removeSecondOrderPrefix(text, length);
      }
    }
  } else { // fail
    if (numSyllables > 2) {
      length = removeSecondOrderPrefix(text, length);
    }
    if (numSyllables > 2) {
      length = removeSuffix(text, length);
    }
  }
  return length;
}

bool IndonesianStemmer::isVowel(wchar_t ch)
{
  switch (ch) {
  case L'a':
  case L'e':
  case L'i':
  case L'o':
  case L'u':
    return true;
  default:
    return false;
  }
}

int IndonesianStemmer::removeParticle(std::deque<wchar_t> &text, int length)
{
  if (StemmerUtil::endsWith(text, length, L"kah") ||
      StemmerUtil::endsWith(text, length, L"lah") ||
      StemmerUtil::endsWith(text, length, L"pun")) {
    numSyllables--;
    return length - 3;
  }

  return length;
}

int IndonesianStemmer::removePossessivePronoun(std::deque<wchar_t> &text,
                                               int length)
{
  if (StemmerUtil::endsWith(text, length, L"ku") ||
      StemmerUtil::endsWith(text, length, L"mu")) {
    numSyllables--;
    return length - 2;
  }

  if (StemmerUtil::endsWith(text, length, L"nya")) {
    numSyllables--;
    return length - 3;
  }

  return length;
}

int IndonesianStemmer::removeFirstOrderPrefix(std::deque<wchar_t> &text,
                                              int length)
{
  if (StemmerUtil::startsWith(text, length, L"meng")) {
    flags |= REMOVED_MENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 4);
  }

  if (StemmerUtil::startsWith(text, length, L"meny") && length > 4 &&
      isVowel(text[4])) {
    flags |= REMOVED_MENG;
    text[3] = L's';
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"men")) {
    flags |= REMOVED_MENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"mem")) {
    flags |= REMOVED_MENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"me")) {
    flags |= REMOVED_MENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 2);
  }

  if (StemmerUtil::startsWith(text, length, L"peng")) {
    flags |= REMOVED_PENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 4);
  }

  if (StemmerUtil::startsWith(text, length, L"peny") && length > 4 &&
      isVowel(text[4])) {
    flags |= REMOVED_PENG;
    text[3] = L's';
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"peny")) {
    flags |= REMOVED_PENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 4);
  }

  if (StemmerUtil::startsWith(text, length, L"pen") && length > 3 &&
      isVowel(text[3])) {
    flags |= REMOVED_PENG;
    text[2] = L't';
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 2);
  }

  if (StemmerUtil::startsWith(text, length, L"pen")) {
    flags |= REMOVED_PENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"pem")) {
    flags |= REMOVED_PENG;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"di")) {
    flags |= REMOVED_DI;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 2);
  }

  if (StemmerUtil::startsWith(text, length, L"ter")) {
    flags |= REMOVED_TER;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"ke")) {
    flags |= REMOVED_KE;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 2);
  }

  return length;
}

int IndonesianStemmer::removeSecondOrderPrefix(std::deque<wchar_t> &text,
                                               int length)
{
  if (StemmerUtil::startsWith(text, length, L"ber")) {
    flags |= REMOVED_BER;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (length == 7 && StemmerUtil::startsWith(text, length, L"belajar")) {
    flags |= REMOVED_BER;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"be") && length > 4 &&
      !isVowel(text[2]) && text[3] == L'e' && text[4] == L'r') {
    flags |= REMOVED_BER;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 2);
  }

  if (StemmerUtil::startsWith(text, length, L"per")) {
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (length == 7 && StemmerUtil::startsWith(text, length, L"pelajar")) {
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 3);
  }

  if (StemmerUtil::startsWith(text, length, L"pe")) {
    flags |= REMOVED_PE;
    numSyllables--;
    return StemmerUtil::deleteN(text, 0, length, 2);
  }

  return length;
}

int IndonesianStemmer::removeSuffix(std::deque<wchar_t> &text, int length)
{
  if (StemmerUtil::endsWith(text, length, L"kan") &&
      (flags & REMOVED_KE) == 0 && (flags & REMOVED_PENG) == 0 &&
      (flags & REMOVED_PE) == 0) {
    numSyllables--;
    return length - 3;
  }

  if (StemmerUtil::endsWith(text, length, L"an") && (flags & REMOVED_DI) == 0 &&
      (flags & REMOVED_MENG) == 0 && (flags & REMOVED_TER) == 0) {
    numSyllables--;
    return length - 2;
  }

  if (StemmerUtil::endsWith(text, length, L"i") &&
      !StemmerUtil::endsWith(text, length, L"si") &&
      (flags & REMOVED_BER) == 0 && (flags & REMOVED_KE) == 0 &&
      (flags & REMOVED_PENG) == 0) {
    numSyllables--;
    return length - 1;
  }
  return length;
}
} // namespace org::apache::lucene::analysis::id