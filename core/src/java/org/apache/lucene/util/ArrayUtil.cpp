using namespace std;

#include "ArrayUtil.h"

namespace org::apache::lucene::util
{

ArrayUtil::ArrayUtil() {} // no instance

int ArrayUtil::parseInt(std::deque<wchar_t> &chars, int offset,
                        int len) 
{
  return parseInt(chars, offset, len, 10);
}

int ArrayUtil::parseInt(std::deque<wchar_t> &chars, int offset, int len,
                        int radix) 
{
  if (chars.empty() || radix < Character::MIN_RADIX ||
      radix > Character::MAX_RADIX) {
    throw make_shared<NumberFormatException>();
  }
  int i = 0;
  if (len == 0) {
    throw make_shared<NumberFormatException>(L"chars length is 0");
  }
  bool negative = chars[offset + i] == L'-';
  if (negative && ++i == len) {
    throw make_shared<NumberFormatException>(L"can't convert to an int");
  }
  if (negative == true) {
    offset++;
    len--;
  }
  return parse(chars, offset, len, radix, negative);
}

int ArrayUtil::parse(std::deque<wchar_t> &chars, int offset, int len,
                     int radix, bool negative) 
{
  int max = numeric_limits<int>::min() / radix;
  int result = 0;
  for (int i = 0; i < len; i++) {
    int digit = Character::digit(chars[i + offset], radix);
    if (digit == -1) {
      throw make_shared<NumberFormatException>(L"Unable to parse");
    }
    if (max > result) {
      throw make_shared<NumberFormatException>(L"Unable to parse");
    }
    int next = result * radix - digit;
    if (next > result) {
      throw make_shared<NumberFormatException>(L"Unable to parse");
    }
    result = next;
  }
  /*while (offset < len) {

  }*/
  if (!negative) {
    result = -result;
    if (result < 0) {
      throw make_shared<NumberFormatException>(L"Unable to parse");
    }
  }
  return result;
}

int ArrayUtil::oversize(int minTargetSize, int bytesPerElement)
{

  if (minTargetSize < 0) {
    // catch usage that accidentally overflows int
    throw invalid_argument(L"invalid array size " + to_wstring(minTargetSize));
  }

  if (minTargetSize == 0) {
    // wait until at least one element is requested
    return 0;
  }

  if (minTargetSize > MAX_ARRAY_LENGTH) {
    throw invalid_argument(L"requested array size " +
                           to_wstring(minTargetSize) +
                           L" exceeds maximum array in java (" +
                           to_wstring(MAX_ARRAY_LENGTH) + L")");
  }

  // asymptotic exponential growth by 1/8th, favors
  // spending a bit more CPU to not tie up too much wasted
  // RAM:
  int extra = minTargetSize >> 3;

  if (extra < 3) {
    // for very small arrays, where constant overhead of
    // realloc is presumably relatively high, we grow
    // faster
    extra = 3;
  }

  int newSize = minTargetSize + extra;

  // add 7 to allow for worst case byte alignment addition below:
  if (newSize + 7 < 0 || newSize + 7 > MAX_ARRAY_LENGTH) {
    // int overflowed, or we exceeded the maximum array length
    return MAX_ARRAY_LENGTH;
  }

  if (Constants::JRE_IS_64BIT) {
    // round up to 8 byte alignment in 64bit env
    switch (bytesPerElement) {
    case 4:
      // round up to multiple of 2
      return (newSize + 1) & 0x7ffffffe;
    case 2:
      // round up to multiple of 4
      return (newSize + 3) & 0x7ffffffc;
    case 1:
      // round up to multiple of 8
      return (newSize + 7) & 0x7ffffff8;
    case 8:
      // no rounding
    default:
      // odd (invalid?) size
      return newSize;
    }
  } else {
    // round up to 4 byte alignment in 64bit env
    switch (bytesPerElement) {
    case 2:
      // round up to multiple of 2
      return (newSize + 1) & 0x7ffffffe;
    case 1:
      // round up to multiple of 4
      return (newSize + 3) & 0x7ffffffc;
    case 4:
    case 8:
      // no rounding
    default:
      // odd (invalid?) size
      return newSize;
    }
  }
}

template <typename T>
std::deque<T> ArrayUtil::grow(std::deque<T> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(
        array_, oversize(minSize, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
  } else {
    return array_;
  }
}

std::deque<short> ArrayUtil::grow(std::deque<short> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Short::BYTES));
  } else {
    return array_;
  }
}

std::deque<short> ArrayUtil::grow(std::deque<short> &array_)
{
  return grow(array_, 1 + array_.size());
}

std::deque<float> ArrayUtil::grow(std::deque<float> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Float::BYTES));
  } else {
    return array_;
  }
}

std::deque<float> ArrayUtil::grow(std::deque<float> &array_)
{
  return grow(array_, 1 + array_.size());
}

std::deque<double> ArrayUtil::grow(std::deque<double> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Double::BYTES));
  } else {
    return array_;
  }
}

std::deque<double> ArrayUtil::grow(std::deque<double> &array_)
{
  return grow(array_, 1 + array_.size());
}

std::deque<int> ArrayUtil::grow(std::deque<int> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Integer::BYTES));
  } else {
    return array_;
  }
}

std::deque<int> ArrayUtil::grow(std::deque<int> &array_)
{
  return grow(array_, 1 + array_.size());
}

std::deque<int64_t> ArrayUtil::grow(std::deque<int64_t> &array_,
                                       int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Long::BYTES));
  } else {
    return array_;
  }
}

std::deque<int64_t> ArrayUtil::grow(std::deque<int64_t> &array_)
{
  return grow(array_, 1 + array_.size());
}

std::deque<char> ArrayUtil::grow(std::deque<char> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Byte::BYTES));
  } else {
    return array_;
  }
}

std::deque<char> ArrayUtil::grow(std::deque<char> &array_)
{
  return grow(array_, 1 + array_.size());
}

std::deque<wchar_t> ArrayUtil::grow(std::deque<wchar_t> &array_, int minSize)
{
  assert((minSize >= 0, L"size must be positive (got " + to_wstring(minSize) +
                            L"): likely integer overflow?"));
  if (array_.size() < minSize) {
    return Arrays::copyOf(array_, oversize(minSize, Character::BYTES));
  } else {
    return array_;
  }
}

std::deque<wchar_t> ArrayUtil::grow(std::deque<wchar_t> &array_)
{
  return grow(array_, 1 + array_.size());
}

int ArrayUtil::hashCode(std::deque<wchar_t> &array_, int start, int end)
{
  int code = 0;
  for (int i = end - 1; i >= start; i--) {
    code = code * 31 + array_[i];
  }
  return code;
}

template <typename T>
void ArrayUtil::swap(std::deque<T> &arr, int i, int j)
{
  constexpr T tmp = arr[i];
  arr[i] = arr[j];
  arr[j] = tmp;
}

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void introSort(T[] a, int fromIndex, int
// toIndex, java.util.Comparator<? super T> comp)
void ArrayUtil::introSort(std::deque<T> &a, int fromIndex, int toIndex,
                          shared_ptr<Comparator<T1>> comp)
{
  if (toIndex - fromIndex <= 1) {
    return;
  }
  (make_shared<ArrayIntroSorter<>>(a, comp))->sort(fromIndex, toIndex);
}

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void introSort(T[] a, java.util.Comparator<?
// super T> comp)
void ArrayUtil::introSort(std::deque<T> &a, shared_ptr<Comparator<T1>> comp)
{
  introSort(a, 0, a.size(), comp);
}

template <typename T>
void ArrayUtil::introSort(std::deque<T> &a, int fromIndex, int toIndex)
{
        static_assert(is_base_of<Comparable<? super T>, T>::value, L"T must inherit from Comparable<? super T>");

        if (toIndex - fromIndex <= 1) {
          return;
        }
        introSort(a, fromIndex, toIndex, Comparator::naturalOrder());
}

template <typename T>
void ArrayUtil::introSort(std::deque<T> &a)
{
        static_assert(is_base_of<Comparable<? super T>, T>::value, L"T must inherit from Comparable<? super T>");

        introSort(a, 0, a.size());
}

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void timSort(T[] a, int fromIndex, int
// toIndex, java.util.Comparator<? super T> comp)
void ArrayUtil::timSort(std::deque<T> &a, int fromIndex, int toIndex,
                        shared_ptr<Comparator<T1>> comp)
{
  if (toIndex - fromIndex <= 1) {
    return;
  }
  (make_shared<ArrayTimSorter<>>(a, comp, a.size() / 64))
      ->sort(fromIndex, toIndex);
}

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void timSort(T[] a, java.util.Comparator<?
// super T> comp)
void ArrayUtil::timSort(std::deque<T> &a, shared_ptr<Comparator<T1>> comp)
{
  timSort(a, 0, a.size(), comp);
}

template <typename T>
void ArrayUtil::timSort(std::deque<T> &a, int fromIndex, int toIndex)
{
        static_assert(is_base_of<Comparable<? super T>, T>::value, L"T must inherit from Comparable<? super T>");

        if (toIndex - fromIndex <= 1) {
          return;
        }
        timSort(a, fromIndex, toIndex, Comparator::naturalOrder());
}

template <typename T>
void ArrayUtil::timSort(std::deque<T> &a)
{
        static_assert(is_base_of<Comparable<? super T>, T>::value, L"T must inherit from Comparable<? super T>");

        timSort(a, 0, a.size());
}

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void select(T[] arr, int from, int to, int
// k, java.util.Comparator<? super T> comparator)
void ArrayUtil::select(std::deque<T> &arr, int from, int to, int k,
                       shared_ptr<Comparator<T1>> comparator)
{
  make_shared<IntroSelectorAnonymousInnerClass>(arr, comparator)
      .select(from, to, k);
}

template <typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public IntroSelectorAnonymousInnerClass(std::deque<T> arr,
// java.util.Comparator<? super T> comparator)
ArrayUtil::IntroSelectorAnonymousInnerClass::IntroSelectorAnonymousInnerClass(
    deque<std::shared_ptr<T>> &arr, shared_ptr<Comparator<T1>> comparator)
{
  this->arr = arr;
  this->comparator = comparator;
}

void ArrayUtil::IntroSelectorAnonymousInnerClass::swap(int i, int j)
{
  ArrayUtil::swap(arr, i, j);
}

void ArrayUtil::IntroSelectorAnonymousInnerClass::setPivot(int i)
{
  pivot = arr[i];
}

int ArrayUtil::IntroSelectorAnonymousInnerClass::comparePivot(int j)
{
  return comparator->compare(pivot, arr[j]);
}
} // namespace org::apache::lucene::util