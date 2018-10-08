using namespace std;

#include "TestRamUsageTesterOnWildAnimals.h"

namespace org::apache::lucene::util
{
using org::junit::Assert;

void TestRamUsageTesterOnWildAnimals::testOverflowMaxChainLength()
{
  int UPPERLIMIT = 100000;
  int lower = 0;
  int upper = UPPERLIMIT;

  while (lower + 1 < upper) {
    int mid = (lower + upper) / 2;
    try {
      shared_ptr<ListElement> first = make_shared<ListElement>();
      shared_ptr<ListElement> last = first;
      for (int i = 0; i < mid; i++) {
        last = (last->next = make_shared<ListElement>());
      }
      RamUsageTester::sizeOf(first); // cause SOE or pass.
      lower = mid;
    } catch (const StackOverflowError &e) {
      upper = mid;
    }
  }

  if (lower + 1 < UPPERLIMIT) {
    Assert::fail(L"Max object chain length till stack overflow: " +
                 to_wstring(lower));
  }
}
} // namespace org::apache::lucene::util