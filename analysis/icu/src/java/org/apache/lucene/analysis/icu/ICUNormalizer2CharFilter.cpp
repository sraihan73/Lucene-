using namespace std;

#include "ICUNormalizer2CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharacterUtils.h"

namespace org::apache::lucene::analysis::icu
{
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using BaseCharFilter =
    org::apache::lucene::analysis::charfilter::BaseCharFilter;
using com::ibm::icu::text::Normalizer2;

ICUNormalizer2CharFilter::ICUNormalizer2CharFilter(shared_ptr<Reader> in_)
    : ICUNormalizer2CharFilter(
          in_, Normalizer2::getInstance(nullptr, L"nfkc_cf",
                                        Normalizer2::Mode::COMPOSE))
{
}

ICUNormalizer2CharFilter::ICUNormalizer2CharFilter(
    shared_ptr<Reader> in_, shared_ptr<Normalizer2> normalizer)
    : ICUNormalizer2CharFilter(in_, normalizer, 128)
{
}

ICUNormalizer2CharFilter::ICUNormalizer2CharFilter(
    shared_ptr<Reader> in_, shared_ptr<Normalizer2> normalizer, int bufferSize)
    : org::apache::lucene::analysis::charfilter::BaseCharFilter(in_),
      normalizer(Objects::requireNonNull(normalizer)),
      tmpBuffer(CharacterUtils::newCharacterBuffer(bufferSize))
{
}

int ICUNormalizer2CharFilter::read(std::deque<wchar_t> &cbuf, int off,
                                   int len) 
{
  if (off < 0) {
    throw invalid_argument(L"off < 0");
  }
  if (off >= cbuf.size()) {
    throw invalid_argument(L"off >= cbuf.length");
  }
  if (len <= 0) {
    throw invalid_argument(L"len <= 0");
  }

  while (!inputFinished || inputBuffer->length() > 0 ||
         resultBuffer->length() > 0) {
    int retLen;

    if (resultBuffer->length() > 0) {
      retLen = outputFromResultBuffer(cbuf, off, len);
      if (retLen > 0) {
        return retLen;
      }
    }

    int resLen = readAndNormalizeFromInput();
    if (resLen > 0) {
      retLen = outputFromResultBuffer(cbuf, off, len);
      if (retLen > 0) {
        return retLen;
      }
    }

    readInputToBuffer();
  }

  return -1;
}

void ICUNormalizer2CharFilter::readInputToBuffer() 
{
  while (true) {
    // CharacterUtils.fill is supplementary char aware
    constexpr bool hasRemainingChars = CharacterUtils::fill(tmpBuffer, input);

    assert(tmpBuffer->getOffset() == 0);
    inputBuffer->append(tmpBuffer->getBuffer(), 0, tmpBuffer->getLength());

    if (hasRemainingChars == false) {
      inputFinished = true;
      break;
    }

    constexpr int lastCodePoint = Character::codePointBefore(
        tmpBuffer->getBuffer(), tmpBuffer->getLength(), 0);
    if (normalizer->isInert(lastCodePoint)) {
      // we require an inert char so that we can normalize content before and
      // after this character independently
      break;
    }
  }

  // if checkedInputBoundary was at the end of a buffer, we need to check that
  // char again
  checkedInputBoundary = max(checkedInputBoundary - 1, 0);
}

int ICUNormalizer2CharFilter::readAndNormalizeFromInput()
{
  if (inputBuffer->length() <= 0) {
    afterQuickCheckYes = false;
    return 0;
  }
  if (!afterQuickCheckYes) {
    int resLen = readFromInputWhileSpanQuickCheckYes();
    afterQuickCheckYes = true;
    if (resLen > 0) {
      return resLen;
    }
  }
  int resLen = readFromIoNormalizeUptoBoundary();
  if (resLen > 0) {
    afterQuickCheckYes = false;
  }
  return resLen;
}

int ICUNormalizer2CharFilter::readFromInputWhileSpanQuickCheckYes()
{
  int end = normalizer->spanQuickCheckYes(inputBuffer);
  if (end > 0) {
    resultBuffer->append(inputBuffer->substr(0, end));
    inputBuffer->remove(0, end);
    checkedInputBoundary = max(checkedInputBoundary - end, 0);
    charCount += end;
  }
  return end;
}

int ICUNormalizer2CharFilter::readFromIoNormalizeUptoBoundary()
{
  // if there's no buffer to normalize, return 0
  if (inputBuffer->length() <= 0) {
    return 0;
  }

  bool foundBoundary = false;
  constexpr int bufLen = inputBuffer->length();

  while (checkedInputBoundary <= bufLen - 1) {
    int charLen =
        Character::charCount(inputBuffer->codePointAt(checkedInputBoundary));
    checkedInputBoundary += charLen;
    if (checkedInputBoundary < bufLen &&
        normalizer->hasBoundaryBefore(
            inputBuffer->codePointAt(checkedInputBoundary))) {
      foundBoundary = true;
      break;
    }
  }
  if (!foundBoundary && checkedInputBoundary >= bufLen && inputFinished) {
    foundBoundary = true;
    checkedInputBoundary = bufLen;
  }

  if (!foundBoundary) {
    return 0;
  }

  return normalizeInputUpto(checkedInputBoundary);
}

int ICUNormalizer2CharFilter::normalizeInputUpto(int const length)
{
  constexpr int destOrigLen = resultBuffer->length();
  normalizer->normalizeSecondAndAppend(resultBuffer,
                                       inputBuffer->substr(0, length));
  inputBuffer->remove(0, length);
  checkedInputBoundary = max(checkedInputBoundary - length, 0);
  constexpr int resultLength = resultBuffer->length() - destOrigLen;
  recordOffsetDiff(length, resultLength);
  return resultLength;
}

void ICUNormalizer2CharFilter::recordOffsetDiff(int inputLength,
                                                int outputLength)
{
  if (inputLength == outputLength) {
    charCount += outputLength;
    return;
  }
  constexpr int diff = inputLength - outputLength;
  constexpr int cumuDiff = getLastCumulativeDiff();
  if (diff < 0) {
    for (int i = 1; i <= -diff; ++i) {
      addOffCorrectMap(charCount + i, cumuDiff - i);
    }
  } else {
    addOffCorrectMap(charCount + outputLength, cumuDiff + diff);
  }
  charCount += outputLength;
}

int ICUNormalizer2CharFilter::outputFromResultBuffer(std::deque<wchar_t> &cbuf,
                                                     int begin, int len)
{
  len = min(resultBuffer->length(), len);
  resultBuffer->getChars(0, len, cbuf, begin);
  if (len > 0) {
    resultBuffer->remove(0, len);
  }
  return len;
}
} // namespace org::apache::lucene::analysis::icu