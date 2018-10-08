using namespace std;

#include "AbstractGroupingTestCase.h"

namespace org::apache::lucene::search::grouping
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

wstring AbstractGroupingTestCase::generateRandomNonEmptyString()
{
  wstring randomValue;
  do {
    // B/c of DV based impl we can't see the difference between an empty string
    // and a null value. For that reason we don't generate empty string groups.
    randomValue = TestUtil::randomRealisticUnicodeString(random());
    // randomValue = _TestUtil.randomSimpleString(random());
  } while (L"" == randomValue);
  return randomValue;
}
} // namespace org::apache::lucene::search::grouping