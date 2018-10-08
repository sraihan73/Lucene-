using namespace std;

#include "DefaultEncoder.h"

namespace org::apache::lucene::search::highlight
{

DefaultEncoder::DefaultEncoder() {}

wstring DefaultEncoder::encodeText(const wstring &originalText)
{
  return originalText;
}
} // namespace org::apache::lucene::search::highlight