using namespace std;

#include "MergeInfo.h"

namespace org::apache::lucene::store
{

MergeInfo::MergeInfo(int totalMaxDoc, int64_t estimatedMergeBytes,
                     bool isExternal, int mergeMaxNumSegments)
    : totalMaxDoc(totalMaxDoc), estimatedMergeBytes(estimatedMergeBytes),
      isExternal(isExternal), mergeMaxNumSegments(mergeMaxNumSegments)
{
}

int MergeInfo::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result =
      prime * result +
      static_cast<int>(
          estimatedMergeBytes ^
          (static_cast<int64_t>(
              static_cast<uint64_t>(estimatedMergeBytes) >> 32)));
  result = prime * result + (isExternal ? 1231 : 1237);
  result = prime * result + mergeMaxNumSegments;
  result = prime * result + totalMaxDoc;
  return result;
}

bool MergeInfo::equals(any obj)
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
  shared_ptr<MergeInfo> other = any_cast<std::shared_ptr<MergeInfo>>(obj);
  if (estimatedMergeBytes != other->estimatedMergeBytes) {
    return false;
  }
  if (isExternal != other->isExternal) {
    return false;
  }
  if (mergeMaxNumSegments != other->mergeMaxNumSegments) {
    return false;
  }
  if (totalMaxDoc != other->totalMaxDoc) {
    return false;
  }
  return true;
}

wstring MergeInfo::toString()
{
  return L"MergeInfo [totalMaxDoc=" + to_wstring(totalMaxDoc) +
         L", estimatedMergeBytes=" + to_wstring(estimatedMergeBytes) +
         L", isExternal=" + StringHelper::toString(isExternal) +
         L", mergeMaxNumSegments=" + to_wstring(mergeMaxNumSegments) + L"]";
}
} // namespace org::apache::lucene::store