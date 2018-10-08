using namespace std;

#include "FlushInfo.h"

namespace org::apache::lucene::store
{

FlushInfo::FlushInfo(int numDocs, int64_t estimatedSegmentSize)
    : numDocs(numDocs), estimatedSegmentSize(estimatedSegmentSize)
{
}

int FlushInfo::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result =
      prime * result +
      static_cast<int>(
          estimatedSegmentSize ^
          (static_cast<int64_t>(
              static_cast<uint64_t>(estimatedSegmentSize) >> 32)));
  result = prime * result + numDocs;
  return result;
}

bool FlushInfo::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<FlushInfo> other = any_cast<std::shared_ptr<FlushInfo>>(obj);
  if (estimatedSegmentSize != other->estimatedSegmentSize) {
    return false;
  }
  if (numDocs != other->numDocs) {
    return false;
  }
  return true;
}

wstring FlushInfo::toString()
{
  return L"FlushInfo [numDocs=" + to_wstring(numDocs) +
         L", estimatedSegmentSize=" + to_wstring(estimatedSegmentSize) + L"]";
}
} // namespace org::apache::lucene::store