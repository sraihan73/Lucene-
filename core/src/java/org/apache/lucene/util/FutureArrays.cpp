using namespace std;

#include "FutureArrays.h"

namespace org::apache::lucene::util
{

FutureArrays::FutureArrays() {} // no instance

void FutureArrays::checkFromToIndex(int fromIndex, int toIndex, int length)
{
  if (fromIndex > toIndex) {
    throw invalid_argument(L"fromIndex " + to_wstring(fromIndex) +
                           L" > toIndex " + to_wstring(toIndex));
  }
  if (fromIndex < 0 || toIndex > length) {
    throw out_of_range(L"Range [" + to_wstring(fromIndex) + L", " +
                       to_wstring(toIndex) + L") out-of-bounds for length " +
                       to_wstring(length));
  }
}

int FutureArrays::mismatch(std::deque<char> &a, int aFromIndex, int aToIndex,
                           std::deque<char> &b, int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  int len = min(aLen, bLen);
  for (int i = 0; i < len; i++) {
    if (a[i + aFromIndex] != b[i + bFromIndex]) {
      return i;
    }
  }
  return aLen == bLen ? -1 : len;
}

int FutureArrays::compareUnsigned(std::deque<char> &a, int aFromIndex,
                                  int aToIndex, std::deque<char> &b,
                                  int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  int len = min(aLen, bLen);
  for (int i = 0; i < len; i++) {
    int aByte = a[i + aFromIndex] & 0xFF;
    int bByte = b[i + bFromIndex] & 0xFF;
    int diff = aByte - bByte;
    if (diff != 0) {
      return diff;
    }
  }

  // One is a prefix of the other, or, they are equal:
  return aLen - bLen;
}

bool FutureArrays::equals(std::deque<char> &a, int aFromIndex, int aToIndex,
                          std::deque<char> &b, int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  // lengths differ: cannot be equal
  if (aLen != bLen) {
    return false;
  }
  for (int i = 0; i < aLen; i++) {
    if (a[i + aFromIndex] != b[i + bFromIndex]) {
      return false;
    }
  }
  return true;
}

int FutureArrays::mismatch(std::deque<wchar_t> &a, int aFromIndex,
                           int aToIndex, std::deque<wchar_t> &b,
                           int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  int len = min(aLen, bLen);
  for (int i = 0; i < len; i++) {
    if (a[i + aFromIndex] != b[i + bFromIndex]) {
      return i;
    }
  }
  return aLen == bLen ? -1 : len;
}

int FutureArrays::compare(std::deque<wchar_t> &a, int aFromIndex, int aToIndex,
                          std::deque<wchar_t> &b, int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  int len = min(aLen, bLen);
  for (int i = 0; i < len; i++) {
    int aInt = a[i + aFromIndex];
    int bInt = b[i + bFromIndex];
    if (aInt > bInt) {
      return 1;
    } else if (aInt < bInt) {
      return -1;
    }
  }

  // One is a prefix of the other, or, they are equal:
  return aLen - bLen;
}

bool FutureArrays::equals(std::deque<wchar_t> &a, int aFromIndex, int aToIndex,
                          std::deque<wchar_t> &b, int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  // lengths differ: cannot be equal
  if (aLen != bLen) {
    return false;
  }
  for (int i = 0; i < aLen; i++) {
    if (a[i + aFromIndex] != b[i + bFromIndex]) {
      return false;
    }
  }
  return true;
}

int FutureArrays::compare(std::deque<int> &a, int aFromIndex, int aToIndex,
                          std::deque<int> &b, int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  int len = min(aLen, bLen);
  for (int i = 0; i < len; i++) {
    int aInt = a[i + aFromIndex];
    int bInt = b[i + bFromIndex];
    if (aInt > bInt) {
      return 1;
    } else if (aInt < bInt) {
      return -1;
    }
  }

  // One is a prefix of the other, or, they are equal:
  return aLen - bLen;
}

bool FutureArrays::equals(std::deque<int> &a, int aFromIndex, int aToIndex,
                          std::deque<int> &b, int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  // lengths differ: cannot be equal
  if (aLen != bLen) {
    return false;
  }
  for (int i = 0; i < aLen; i++) {
    if (a[i + aFromIndex] != b[i + bFromIndex]) {
      return false;
    }
  }
  return true;
}

int FutureArrays::compare(std::deque<int64_t> &a, int aFromIndex,
                          int aToIndex, std::deque<int64_t> &b,
                          int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  int len = min(aLen, bLen);
  for (int i = 0; i < len; i++) {
    int64_t aInt = a[i + aFromIndex];
    int64_t bInt = b[i + bFromIndex];
    if (aInt > bInt) {
      return 1;
    } else if (aInt < bInt) {
      return -1;
    }
  }

  // One is a prefix of the other, or, they are equal:
  return aLen - bLen;
}

bool FutureArrays::equals(std::deque<int64_t> &a, int aFromIndex,
                          int aToIndex, std::deque<int64_t> &b,
                          int bFromIndex, int bToIndex)
{
  checkFromToIndex(aFromIndex, aToIndex, a.size());
  checkFromToIndex(bFromIndex, bToIndex, b.size());
  int aLen = aToIndex - aFromIndex;
  int bLen = bToIndex - bFromIndex;
  // lengths differ: cannot be equal
  if (aLen != bLen) {
    return false;
  }
  for (int i = 0; i < aLen; i++) {
    if (a[i + aFromIndex] != b[i + bFromIndex]) {
      return false;
    }
  }
  return true;
}
} // namespace org::apache::lucene::util