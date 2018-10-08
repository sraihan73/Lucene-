using namespace std;

#include "IndexOutput.h"

namespace org::apache::lucene::store
{

IndexOutput::IndexOutput(const wstring &resourceDescription,
                         const wstring &name)
    : resourceDescription(resourceDescription), name(name)
{
  if (resourceDescription == L"") {
    throw invalid_argument(L"resourceDescription must not be null");
  }
}

wstring IndexOutput::getName() { return name; }

wstring IndexOutput::toString() { return resourceDescription; }
} // namespace org::apache::lucene::store