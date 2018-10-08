using namespace std;

#include "TestPosition.h"

namespace org::apache::lucene::analysis
{
using org::junit::Ignore;

wstring TestPosition::getFact() { return fact; }

void TestPosition::setFact(const wstring &fact) { this->fact = fact; }
} // namespace org::apache::lucene::analysis