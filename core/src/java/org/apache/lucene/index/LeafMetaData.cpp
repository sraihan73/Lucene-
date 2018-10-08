using namespace std;

#include "LeafMetaData.h"

namespace org::apache::lucene::index
{
using Sort = org::apache::lucene::search::Sort;
using Version = org::apache::lucene::util::Version;

LeafMetaData::LeafMetaData(int createdVersionMajor,
                           shared_ptr<Version> minVersion,
                           shared_ptr<Sort> sort)
    : createdVersionMajor(createdVersionMajor), minVersion(minVersion),
      sort(sort)
{
  if (createdVersionMajor > Version::LATEST->major) {
    throw invalid_argument(L"createdVersionMajor is in the future: " +
                           to_wstring(createdVersionMajor));
  }
  if (createdVersionMajor < 6) {
    throw invalid_argument(L"createdVersionMajor must be >= 6, got: " +
                           to_wstring(createdVersionMajor));
  }
  if (minVersion != nullptr &&
      minVersion->onOrAfter(Version::LUCENE_7_0_0) == false) {
    throw invalid_argument(L"minVersion must be >= 7.0.0: " + minVersion);
  }
  if (createdVersionMajor >= 7 && minVersion == nullptr) {
    throw invalid_argument(
        L"minVersion must be set when createdVersionMajor is >= 7");
  }
}

int LeafMetaData::getCreatedVersionMajor() { return createdVersionMajor; }

shared_ptr<Version> LeafMetaData::getMinVersion() { return minVersion; }

shared_ptr<Sort> LeafMetaData::getSort() { return sort; }
} // namespace org::apache::lucene::index