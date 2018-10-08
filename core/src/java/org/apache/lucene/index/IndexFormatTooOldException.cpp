using namespace std;

#include "IndexFormatTooOldException.h"

namespace org::apache::lucene::index
{
using DataInput = org::apache::lucene::store::DataInput;

IndexFormatTooOldException::IndexFormatTooOldException(
    const wstring &resourceDescription, const wstring &reason)
    : java::io::IOException(L"Format version is not supported (resource " +
                            resourceDescription + L"): " + reason +
                            L". This version of Lucene only supports indexes "
                            L"created with release 6.0 and later."),
      resourceDescription(resourceDescription), reason(reason),
      version(nullopt), minVersion(nullopt), maxVersion(nullopt)
{
}

IndexFormatTooOldException::IndexFormatTooOldException(
    shared_ptr<DataInput> in_, const wstring &reason)
    : IndexFormatTooOldException(Objects->toString(in_), reason)
{
}

IndexFormatTooOldException::IndexFormatTooOldException(
    const wstring &resourceDescription, int version, int minVersion,
    int maxVersion)
    : java::io::IOException(L"Format version is not supported (resource " +
                            resourceDescription + L"): " + version +
                            L" (needs to be between " + minVersion + L" and " +
                            maxVersion +
                            L"). This version of Lucene only supports indexes "
                            L"created with release 6.0 and later."),
      resourceDescription(resourceDescription), reason(L""), version(version),
      minVersion(minVersion), maxVersion(maxVersion)
{
}

IndexFormatTooOldException::IndexFormatTooOldException(
    shared_ptr<DataInput> in_, int version, int minVersion, int maxVersion)
    : IndexFormatTooOldException(Objects->toString(in_), version, minVersion,
                                 maxVersion)
{
}

wstring IndexFormatTooOldException::getResourceDescription()
{
  return resourceDescription;
}

wstring IndexFormatTooOldException::getReason() { return reason; }

optional<int> IndexFormatTooOldException::getVersion() { return version; }

optional<int> IndexFormatTooOldException::getMaxVersion() { return maxVersion; }

optional<int> IndexFormatTooOldException::getMinVersion() { return minVersion; }
} // namespace org::apache::lucene::index