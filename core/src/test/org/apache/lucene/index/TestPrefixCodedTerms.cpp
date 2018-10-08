using namespace std;

#include "TestPrefixCodedTerms.h"

namespace org::apache::lucene::index
{
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPrefixCodedTerms::testEmpty()
{
  shared_ptr<PrefixCodedTerms::Builder> b =
      make_shared<PrefixCodedTerms::Builder>();
  shared_ptr<PrefixCodedTerms> pb = b->finish();
  shared_ptr<TermIterator> iter = pb->begin();
  assertNull(iter->next());
}

void TestPrefixCodedTerms::testOne()
{
  shared_ptr<Term> term = make_shared<Term>(L"foo", L"bogus");
  shared_ptr<PrefixCodedTerms::Builder> b =
      make_shared<PrefixCodedTerms::Builder>();
  b->add(term);
  shared_ptr<PrefixCodedTerms> pb = b->finish();
  shared_ptr<TermIterator> iter = pb->begin();
  assertNotNull(iter->next());
  TestUtil::assertEquals(L"foo", iter->field());
  TestUtil::assertEquals(L"bogus", iter->bytes->utf8ToString());
  assertNull(iter->next());
}

void TestPrefixCodedTerms::testRandom()
{
  shared_ptr<Set<std::shared_ptr<Term>>> terms = set<std::shared_ptr<Term>>();
  int nterms = atLeast(10000);
  for (int i = 0; i < nterms; i++) {
    shared_ptr<Term> term =
        make_shared<Term>(TestUtil::randomUnicodeString(random(), 2),
                          TestUtil::randomUnicodeString(random()));
    terms->add(term);
  }

  shared_ptr<PrefixCodedTerms::Builder> b =
      make_shared<PrefixCodedTerms::Builder>();
  for (auto ref : terms) {
    b->add(ref);
  }
  shared_ptr<PrefixCodedTerms> pb = b->finish();

  shared_ptr<TermIterator> iter = pb->begin();
  Set<std::shared_ptr<Term>>::const_iterator expected = terms->begin();
  TestUtil::assertEquals(terms->size(), pb->size());
  // System.out.println("TEST: now iter");
  while (iter->next() != nullptr) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    assertTrue(expected->hasNext());
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    TestUtil::assertEquals(expected->next(),
                           make_shared<Term>(iter->field(), iter->bytes));
  }

  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(expected->hasNext());
}
} // namespace org::apache::lucene::index