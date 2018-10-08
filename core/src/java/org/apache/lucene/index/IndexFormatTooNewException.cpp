using namespace std;

#include "IndexFormatTooNewException.h"

namespace org::apache::lucene::index
{
using DataInput = org::apache::lucene::store::DataInput;

IndexFormatTooNewException::IndexFormatTooNewException(
    const wstring &resourceDescription, int version, int minVersion,
    int maxVersion)
    : java::io::IOException(L"Format version is not supported (resource " +
                            resourceDescription + L"): " + version +
                            L" (needs to be between " + minVersion + L" and " +
                            maxVersion + L")"),
      resourceDescription(resourceDescription), version(version),
      minVersion(minVersion), maxVersion(maxVersion)
{
}

IndexFormatTooNewException::IndexFormatTooNewException(
    shared_ptr<DataInput> in_, int version, int minVersion, int maxVersion)
    : IndexFormatTooNewException(Objects->toString(in_), version, minVersion,
                                 maxVersion)
{
}

wstring IndexFormatTooNewException::getResourceDescription()
{
  return resourceDescription;
}

int IndexFormatTooNewException::getVersion() { return version; }

int IndexFormatTooNewException::getMaxVersion() { return maxVersion; }

int IndexFormatTooNewException::getMinVersion() { return minVersion; }
} // namespace org::apache::lucene::index