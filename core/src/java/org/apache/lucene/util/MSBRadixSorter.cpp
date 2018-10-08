using namespace std;

#include "MSBRadixSorter.h"

namespace org::apache::lucene::util
{

MSBRadixSorter::MSBRadixSorter(int maxLength)
    : commonPrefix(std::deque<int>(min(24, maxLength))), maxLength(maxLength)
{
}

shared_ptr<Sorter> MSBRadixSorter::getFallbackSorter(int k)
{
  return make_shared<IntroSorterAnonymousInnerClass>(shared_from_this(), k);
}

MSBRadixSorter::IntroSorterAnonymousInnerClass::IntroSorterAnonymousInnerClass(
    shared_ptr<MSBRadixSorter> outerInstance, int k)
{
  this->outerInstance = outerInstance;
  this->k = k;
  pivot = make_shared<BytesRefBuilder>();
}

void MSBRadixSorter::IntroSorterAnonymousInnerClass::swap(int i, int j)
{
  outerInstance->swap(i, j);
}

int MSBRadixSorter::IntroSorterAnonymousInnerClass::compare(int i, int j)
{
  for (int o = k; o < outerInstance->maxLength; ++o) {
    constexpr int b1 = outerInstance->byteAt(i, o);
    constexpr int b2 = outerInstance->byteAt(j, o);
    if (b1 != b2) {
      return b1 - b2;
    } else if (b1 == -1) {
      break;
    }
  }
  return 0;
}

void MSBRadixSorter::IntroSorterAnonymousInnerClass::setPivot(int i)
{
  pivot->setLength(0);
  for (int o = k; o < outerInstance->maxLength; ++o) {
    constexpr int b = outerInstance->byteAt(i, o);
    if (b == -1) {
      break;
    }
    pivot->append(static_cast<char>(b));
  }
}

int MSBRadixSorter::IntroSorterAnonymousInnerClass::comparePivot(int j)
{
  for (int o = 0; o < pivot->length(); ++o) {
    constexpr int b1 = pivot::byteAt(o) & 0xff;
    constexpr int b2 = outerInstance->byteAt(j, k + o);
    if (b1 != b2) {
      return b1 - b2;
    }
  }
  if (k + pivot->length() == outerInstance->maxLength) {
    return 0;
  }
  return -1 - outerInstance->byteAt(j, k + pivot->length());
}

int MSBRadixSorter::compare(int i, int j)
{
  throw make_shared<UnsupportedOperationException>(
      L"unused: not a comparison-based sort");
}

void MSBRadixSorter::sort(int from, int to)
{
  checkRange(from, to);
  sort(from, to, 0, 0);
}

void MSBRadixSorter::sort(int from, int to, int k, int l)
{
  if (to - from <= LENGTH_THRESHOLD || l >= LEVEL_THRESHOLD) {
    introSort(from, to, k);
  } else {
    radixSort(from, to, k, l);
  }
}

void MSBRadixSorter::introSort(int from, int to, int k)
{
  getFallbackSorter(k)->sort(from, to);
}

void MSBRadixSorter::radixSort(int from, int to, int k, int l)
{
  std::deque<int> histogram = histograms[l];
  if (histogram.empty()) {
    histogram = histograms[l] = std::deque<int>(HISTOGRAM_SIZE);
  } else {
    Arrays::fill(histogram, 0);
  }

  constexpr int commonPrefixLength =
      computeCommonPrefixLengthAndBuildHistogram(from, to, k, histogram);
  if (commonPrefixLength > 0) {
    // if there are no more chars to compare or if all entries fell into the
    // first bucket (which means strings are shorter than k) then we are done
    // otherwise recurse
    if (k + commonPrefixLength < maxLength && histogram[0] < to - from) {
      radixSort(from, to, k + commonPrefixLength, l);
    }
    return;
  }
  assert((assertHistogram(commonPrefixLength, histogram)));

  std::deque<int> startOffsets = histogram;
  std::deque<int> endOffsets = this->endOffsets;
  sumHistogram(histogram, endOffsets);
  reorder(from, to, startOffsets, endOffsets, k);
  endOffsets = startOffsets;

  if (k + 1 < maxLength) {
    // recurse on all but the first bucket since all keys are equals in this
    // bucket (we already compared all bytes)
    for (int prev = endOffsets[0], i = 1; i < HISTOGRAM_SIZE; ++i) {
      int h = endOffsets[i];
      constexpr int bucketLen = h - prev;
      if (bucketLen > 1) {
        sort(from + prev, from + h, k + 1, l + 1);
      }
      prev = h;
    }
  }
}

bool MSBRadixSorter::assertHistogram(int commonPrefixLength,
                                     std::deque<int> &histogram)
{
  int numberOfUniqueBytes = 0;
  for (auto freq : histogram) {
    if (freq > 0) {
      numberOfUniqueBytes++;
    }
  }
  if (numberOfUniqueBytes == 1) {
    assert(commonPrefixLength >= 1);
  } else {
    assert((commonPrefixLength == 0, commonPrefixLength));
  }
  return true;
}

int MSBRadixSorter::getBucket(int i, int k) { return byteAt(i, k) + 1; }

int MSBRadixSorter::computeCommonPrefixLengthAndBuildHistogram(
    int from, int to, int k, std::deque<int> &histogram)
{
  const std::deque<int> commonPrefix = this->commonPrefix;
  int commonPrefixLength = min(commonPrefix.size(), maxLength - k);
  for (int j = 0; j < commonPrefixLength; ++j) {
    constexpr int b = byteAt(from, k + j);
    commonPrefix[j] = b;
    if (b == -1) {
      commonPrefixLength = j + 1;
      break;
    }
  }

  int i;
  for (i = from + 1; i < to; ++i) {
    for (int j = 0; j < commonPrefixLength; ++j) {
      constexpr int b = byteAt(i, k + j);
      if (b != commonPrefix[j]) {
        commonPrefixLength = j;
        if (commonPrefixLength == 0) { // we have no common prefix
          histogram[commonPrefix[0] + 1] = i - from;
          histogram[b + 1] = 1;
          goto outerBreak;
        }
        break;
      }
    }
  outerContinue:;
  }
outerBreak:

  if (i < to) {
    // the loop got broken because there is no common prefix
    assert(commonPrefixLength == 0);
    buildHistogram(i + 1, to, k, histogram);
  } else {
    assert(commonPrefixLength > 0);
    histogram[commonPrefix[0] + 1] = to - from;
  }

  return commonPrefixLength;
}

void MSBRadixSorter::buildHistogram(int from, int to, int k,
                                    std::deque<int> &histogram)
{
  for (int i = from; i < to; ++i) {
    histogram[getBucket(i, k)]++;
  }
}

void MSBRadixSorter::sumHistogram(std::deque<int> &histogram,
                                  std::deque<int> &endOffsets)
{
  int accum = 0;
  for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
    constexpr int count = histogram[i];
    histogram[i] = accum;
    accum += count;
    endOffsets[i] = accum;
  }
}

void MSBRadixSorter::reorder(int from, int to, std::deque<int> &startOffsets,
                             std::deque<int> &endOffsets, int k)
{
  // reorder in place, like the dutch flag problem
  for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
    constexpr int limit = endOffsets[i];
    for (int h1 = startOffsets[i]; h1 < limit; h1 = startOffsets[i]) {
      constexpr int b = getBucket(from + h1, k);
      constexpr int h2 = startOffsets[b]++;
      swap(from + h1, from + h2);
    }
  }
}
} // namespace org::apache::lucene::util