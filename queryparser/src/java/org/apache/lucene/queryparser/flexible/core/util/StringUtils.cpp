using namespace std;

#include "StringUtils.h"

namespace org::apache::lucene::queryparser::flexible::core::util
{

wstring StringUtils::toString(any obj)
{

  if (obj != nullptr) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return obj.toString();

  } else {
    return L"";
  }
}
} // namespace org::apache::lucene::queryparser::flexible::core::util