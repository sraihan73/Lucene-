using namespace std;

#include "FutureObjects.h"

namespace org::apache::lucene::util
{

FutureObjects::FutureObjects() {} // no instance

int FutureObjects::checkIndex(int index, int length)
{
  if (index < 0 || index >= length) {
    throw out_of_range(L"Index " + to_wstring(index) +
                       L" out-of-bounds for length " + to_wstring(length));
  }
  return index;
}

int FutureObjects::checkFromToIndex(int fromIndex, int toIndex, int length)
{
  if (fromIndex < 0 || fromIndex > toIndex || toIndex > length) {
    throw out_of_range(L"Range [" + to_wstring(fromIndex) + L", " +
                       to_wstring(toIndex) + L") out-of-bounds for length " +
                       to_wstring(length));
  }
  return fromIndex;
}

int FutureObjects::checkFromIndexSize(int fromIndex, int size, int length)
{
  int end = fromIndex + size;
  if (fromIndex < 0 || fromIndex > end || end > length) {
    throw out_of_range(L"Range [" + to_wstring(fromIndex) + L", " +
                       to_wstring(fromIndex) + L" + " + to_wstring(size) +
                       L") out-of-bounds for length " + to_wstring(length));
  }
  return fromIndex;
}
} // namespace org::apache::lucene::util