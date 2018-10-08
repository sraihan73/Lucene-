using namespace std;

#include "TestTerm.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestTerm::testEquals()
{
  shared_ptr<Term> *const base = make_shared<Term>(L"same", L"same");
  shared_ptr<Term> *const same = make_shared<Term>(L"same", L"same");
  shared_ptr<Term> *const differentField =
      make_shared<Term>(L"different", L"same");
  shared_ptr<Term> *const differentText =
      make_shared<Term>(L"same", L"different");
  const wstring differentType = L"AString";
  assertEquals(base, base);
  assertEquals(base, same);
  assertFalse(base->equals(differentField));
  assertFalse(base->equals(differentText));
  assertFalse(base->equals(differentType));
}
} // namespace org::apache::lucene::index