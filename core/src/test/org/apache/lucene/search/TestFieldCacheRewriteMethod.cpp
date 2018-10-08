using namespace std;

#include "TestFieldCacheRewriteMethod.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using RegExp = org::apache::lucene::util::automaton::RegExp;

void TestFieldCacheRewriteMethod::assertSame(const wstring &regexp) throw(
    IOException)
{
  shared_ptr<RegexpQuery> fieldCache = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, regexp), RegExp::NONE);
  fieldCache->setRewriteMethod(make_shared<DocValuesRewriteMethod>());

  shared_ptr<RegexpQuery> filter = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, regexp), RegExp::NONE);
  filter->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);

  shared_ptr<TopDocs> fieldCacheDocs = searcher1->search(fieldCache, 25);
  shared_ptr<TopDocs> filterDocs = searcher2->search(filter, 25);

  CheckHits::checkEqual(fieldCache, fieldCacheDocs->scoreDocs,
                        filterDocs->scoreDocs);
}

void TestFieldCacheRewriteMethod::testEquals() 
{
  shared_ptr<RegexpQuery> a1 = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, L"[aA]"), RegExp::NONE);
  shared_ptr<RegexpQuery> a2 = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, L"[aA]"), RegExp::NONE);
  shared_ptr<RegexpQuery> b = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, L"[bB]"), RegExp::NONE);
  assertEquals(a1, a2);
  assertFalse(a1->equals(b));

  a1->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  a2->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  b->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  assertEquals(a1, a2);
  assertFalse(a1->equals(b));
  QueryUtils::check(a1);
}
} // namespace org::apache::lucene::search