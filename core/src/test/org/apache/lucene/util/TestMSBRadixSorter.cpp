using namespace std;

#include "TestMSBRadixSorter.h"

namespace org::apache::lucene::util
{

void TestMSBRadixSorter::test(std::deque<std::shared_ptr<BytesRef>> &refs,
                              int len)
{
  std::deque<std::shared_ptr<BytesRef>> expected = Arrays::copyOf(refs, len);
  Arrays::sort(expected);

  int maxLength = 0;
  for (int i = 0; i < len; ++i) {
    shared_ptr<BytesRef> ref = refs[i];
    maxLength = max(maxLength, ref->length);
  }
  switch (random()->nextInt(3)) {
  case 0:
    maxLength += TestUtil::nextInt(random(), 1, 5);
    break;
  case 1:
    maxLength = numeric_limits<int>::max();
    break;
  default:
    // leave unchanged
    break;
  }

  constexpr int finalMaxLength = maxLength;
  make_shared<MSBRadixSorterAnonymousInnerClass>(shared_from_this(), maxLength,
                                                 refs, finalMaxLength)
      .sort(0, len);
  std::deque<std::shared_ptr<BytesRef>> actual = Arrays::copyOf(refs, len);
  assertArrayEquals(expected, actual);
}

TestMSBRadixSorter::MSBRadixSorterAnonymousInnerClass::
    MSBRadixSorterAnonymousInnerClass(
        shared_ptr<TestMSBRadixSorter> outerInstance, int maxLength,
        deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> &refs,
        int finalMaxLength)
    : MSBRadixSorter(maxLength)
{
  this->outerInstance = outerInstance;
  this->refs = refs;
  this->finalMaxLength = finalMaxLength;
}

int TestMSBRadixSorter::MSBRadixSorterAnonymousInnerClass::byteAt(int i, int k)
{
  assertTrue(k < finalMaxLength);
  shared_ptr<BytesRef> ref = refs[i];
  if (ref->length <= k) {
    return -1;
  }
  return ref->bytes[ref->offset + k] & 0xff;
}

void TestMSBRadixSorter::MSBRadixSorterAnonymousInnerClass::swap(int i, int j)
{
  shared_ptr<BytesRef> tmp = refs[i];
  refs[i] = refs[j];
  refs[j] = tmp;
}

void TestMSBRadixSorter::testEmpty()
{
  test(std::deque<std::shared_ptr<BytesRef>>(random()->nextInt(5)), 0);
}

void TestMSBRadixSorter::testOneValue()
{
  shared_ptr<BytesRef> bytes =
      make_shared<BytesRef>(TestUtil::randomSimpleString(random()));
  test(std::deque<std::shared_ptr<BytesRef>>{bytes}, 1);
}

void TestMSBRadixSorter::testTwoValues()
{
  shared_ptr<BytesRef> bytes1 =
      make_shared<BytesRef>(TestUtil::randomSimpleString(random()));
  shared_ptr<BytesRef> bytes2 =
      make_shared<BytesRef>(TestUtil::randomSimpleString(random()));
  test(std::deque<std::shared_ptr<BytesRef>>{bytes1, bytes2}, 2);
}

void TestMSBRadixSorter::testRandom(int commonPrefixLen, int maxLen)
{
  std::deque<char> commonPrefix(commonPrefixLen);
  random()->nextBytes(commonPrefix);
  constexpr int len = random()->nextInt(100000);
  std::deque<std::shared_ptr<BytesRef>> bytes(len + random()->nextInt(50));
  for (int i = 0; i < len; ++i) {
    std::deque<char> b(commonPrefixLen + random()->nextInt(maxLen));
    random()->nextBytes(b);
    System::arraycopy(commonPrefix, 0, b, 0, commonPrefixLen);
    bytes[i] = make_shared<BytesRef>(b);
  }
  test(bytes, len);
}

void TestMSBRadixSorter::testRandom()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(0, 10);
  }
}

void TestMSBRadixSorter::testRandomWithLotsOfDuplicates()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(0, 2);
  }
}

void TestMSBRadixSorter::testRandomWithSharedPrefix()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(TestUtil::nextInt(random(), 1, 30), 10);
  }
}

void TestMSBRadixSorter::testRandomWithSharedPrefixAndLotsOfDuplicates()
{
  for (int iter = 0; iter < 10; ++iter) {
    testRandom(TestUtil::nextInt(random(), 1, 30), 2);
  }
}

void TestMSBRadixSorter::testRandom2()
{
  // how large our alphabet is
  int letterCount = TestUtil::nextInt(random(), 2, 10);

  // how many substring fragments to use
  int substringCount = TestUtil::nextInt(random(), 2, 10);
  shared_ptr<Set<std::shared_ptr<BytesRef>>> substringsSet =
      unordered_set<std::shared_ptr<BytesRef>>();

  // how many strings to make
  int stringCount = atLeast(10000);

  // System.out.println("letterCount=" + letterCount + " substringCount=" +
  // substringCount + " stringCount=" + stringCount);
  while (substringsSet->size() < substringCount) {
    int length = TestUtil::nextInt(random(), 2, 10);
    std::deque<char> bytes(length);
    for (int i = 0; i < length; i++) {
      bytes[i] = static_cast<char>(random()->nextInt(letterCount));
    }
    shared_ptr<BytesRef> br = make_shared<BytesRef>(bytes);
    substringsSet->add(br);
    // System.out.println("add substring count=" + substringsSet.size() + ": " +
    // br);
  }

  std::deque<std::shared_ptr<BytesRef>> substrings = substringsSet->toArray(
      std::deque<std::shared_ptr<BytesRef>>(substringsSet->size()));
  std::deque<double> chance(substrings.size());
  double sum = 0.0;
  for (int i = 0; i < substrings.size(); i++) {
    chance[i] = random()->nextDouble();
    sum += chance[i];
  }

  // give each substring a random chance of occurring:
  double accum = 0.0;
  for (int i = 0; i < substrings.size(); i++) {
    accum += chance[i] / sum;
    chance[i] = accum;
  }

  shared_ptr<Set<std::shared_ptr<BytesRef>>> stringsSet =
      unordered_set<std::shared_ptr<BytesRef>>();
  int iters = 0;
  while (stringsSet->size() < stringCount && iters < stringCount * 5) {
    int count = TestUtil::nextInt(random(), 1, 5);
    shared_ptr<BytesRefBuilder> b = make_shared<BytesRefBuilder>();
    for (int i = 0; i < count; i++) {
      double v = random()->nextDouble();
      accum = 0.0;
      for (int j = 0; j < substrings.size(); j++) {
        accum += chance[j];
        if (accum >= v) {
          b->append(substrings[j]);
          break;
        }
      }
    }
    shared_ptr<BytesRef> br = b->toBytesRef();
    stringsSet->add(br);
    // System.out.println("add string count=" + stringsSet.size() + ": " + br);
    iters++;
  }

  test(stringsSet->toArray(
           std::deque<std::shared_ptr<BytesRef>>(stringsSet->size())),
       stringsSet->size());
}
} // namespace org::apache::lucene::util