using namespace std;

#include "FacetResult.h"

namespace org::apache::lucene::facet
{

FacetResult::FacetResult(
    const wstring &dim, std::deque<wstring> &path, shared_ptr<Number> value,
    std::deque<std::shared_ptr<LabelAndValue>> &labelValues, int childCount)
    : dim(dim), path(path), value(value), childCount(childCount),
      labelValues(labelValues)
{
}

wstring FacetResult::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"dim=");
  sb->append(dim);
  sb->append(L" path=");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  sb->append(Arrays->toString(path));
  sb->append(L" value=");
  sb->append(value);
  sb->append(L" childCount=");
  sb->append(childCount);
  sb->append(L'\n');
  for (auto labelValue : labelValues) {
    sb->append(L"  " + labelValue + L"\n");
  }
  return sb->toString();
}

bool FacetResult::equals(any _other)
{
  if ((std::dynamic_pointer_cast<FacetResult>(_other) != nullptr) == false) {
    return false;
  }
  shared_ptr<FacetResult> other =
      any_cast<std::shared_ptr<FacetResult>>(_other);
  return value->equals(other->value) && childCount == other->childCount &&
         Arrays::equals(labelValues, other->labelValues);
}

int FacetResult::hashCode()
{
  int hashCode = value->hashCode() + 31 * childCount;
  for (auto labelValue : labelValues) {
    hashCode = labelValue->hashCode() + 31 * hashCode;
  }
  return hashCode;
}
} // namespace org::apache::lucene::facet