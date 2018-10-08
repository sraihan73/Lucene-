using namespace std;

#include "RadixSelector.h"

namespace org::apache::lucene::util
{

RadixSelector::RadixSelector(int maxLength)
    : commonPrefix(std::deque<int>(min(24, maxLength))), maxLength(maxLength)
{
}

shared_ptr<Selector> RadixSelector::getFallbackSelector(int d)
{
  return make_shared<IntroSelectorAnonymousInnerClass>(shared_from_this(), d);
}

RadixSelector::IntroSelectorAnonymousInnerClass::
    IntroSelectorAnonymousInnerClass(shared_ptr<RadixSelector> outerInstance,
                                     int d)
{
  this->outerInstance = outerInstance;
  this->d = d;
  pivot = make_shared<BytesRefBuilder>();
}

void RadixSelector::IntroSelectorAnonymousInnerClass::swap(int i, int j)
{
  outerInstance->swap(i, j);
}

int RadixSelector::IntroSelectorAnonymousInnerClass::compare(int i, int j)
{
  for (int o = d; o < outerInstance->maxLength; ++o) {
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

void RadixSelector::IntroSelectorAnonymousInnerClass::setPivot(int i)
{
  pivot->setLength(0);
  for (int o = d; o < outerInstance->maxLength; ++o) {
    constexpr int b = outerInstance->byteAt(i, o);
    if (b == -1) {
      break;
    }
    pivot->append(static_cast<char>(b));
  }
}

int RadixSelector::IntroSelectorAnonymousInnerClass::comparePivot(int j)
{
  for (int o = 0; o < pivot->length(); ++o) {
    constexpr int b1 = pivot::byteAt(o) & 0xff;
    constexpr int b2 = outerInstance->byteAt(j, d + o);
    if (b1 != b2) {
      return b1 - b2;
    }
  }
  if (d + pivot->length() == outerInstance->maxLength) {
    return 0;
  }
  return -1 - outerInstance->byteAt(j, d + pivot->length());
}

void RadixSelector::select(int from, int to, int k)
{
  checkArgs(from, to, k);
  select(from, to, k, 0, 0);
}

void RadixSelector::select(int from, int to, int k, int d, int l)
{
  if (to - from <= LENGTH_THRESHOLD || d >= LEVEL_THRESHOLD) {
    getFallbackSelector(d)->select(from, to, k);
  } else {
    radixSelect(from, to, k, d, l);
  }
}

void RadixSelector::radixSelect(int from, int to, int k, int d, int l)
{
  const std::deque<int> histogram = this->histogram;
  Arrays::fill(histogram, 0);

  constexpr int commonPrefixLength =
      computeCommonPrefixLengthAndBuildHistogram(from, to, d, histogram);
  if (commonPrefixLength > 0) {
    // if there are no more chars to compare or if all entries fell into the
    // first bucket (which means strings are shorter than d) then we are done
    // otherwise recurse
    if (d + commonPrefixLength < maxLength && histogram[0] < to - from) {
      radixSelect(from, to, k, d + commonPrefixLength, l);
    }
    return;
  }
  assert((assertHistogram(commonPrefixLength, histogram)));

  int bucketFrom = from;
  for (int bucket = 0; bucket < HISTOGRAM_SIZE; ++bucket) {
    constexpr int bucketTo = bucketFrom + histogram[bucket];

    if (bucketTo > k) {
      partition(from, to, bucket, bucketFrom, bucketTo, d);

      if (bucket != 0 && d + 1 < maxLength) {
        // all elements in bucket 0 are equal so we only need to recurse if
        // bucket != 0
        select(bucketFrom, bucketTo, k, d + 1, l + 1);
      }
      return;
    }
    bucketFrom = bucketTo;
  }
  throw make_shared<AssertionError>(L"Unreachable code");
}

bool RadixSelector::assertHistogram(int commonPrefixLength,
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
    assert(commonPrefixLength == 0);
  }
  return true;
}

int RadixSelector::getBucket(int i, int k) { return byteAt(i, k) + 1; }

int RadixSelector::computeCommonPrefixLengthAndBuildHistogram(
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

void RadixSelector::buildHistogram(int from, int to, int k,
                                   std::deque<int> &histogram)
{
  for (int i = from; i < to; ++i) {
    histogram[getBucket(i, k)]++;
  }
}

void RadixSelector::partition(int from, int to, int bucket, int bucketFrom,
                              int bucketTo, int d)
{
  int left = from;
  int right = to - 1;

  int slot = bucketFrom;

  for (;;) {
    int leftBucket = getBucket(left, d);
    int rightBucket = getBucket(right, d);

    while (leftBucket <= bucket && left < bucketFrom) {
      if (leftBucket == bucket) {
        swap(left, slot++);
      } else {
        ++left;
      }
      leftBucket = getBucket(left, d);
    }

    while (rightBucket >= bucket && right >= bucketTo) {
      if (rightBucket == bucket) {
        swap(right, slot++);
      } else {
        --right;
      }
      rightBucket = getBucket(right, d);
    }

    if (left < bucketFrom && right >= bucketTo) {
      swap(left++, right--);
    } else {
      assert(left == bucketFrom);
      assert(right == bucketTo - 1);
      break;
    }
  }
}
} // namespace org::apache::lucene::util