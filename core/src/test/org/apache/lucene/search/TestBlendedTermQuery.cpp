using namespace std;

#include "TestBlendedTermQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBlendedTermQuery::testEquals()
{
  shared_ptr<Term> t1 = make_shared<Term>(L"foo", L"bar");

  shared_ptr<BlendedTermQuery> bt1 =
      (make_shared<BlendedTermQuery::Builder>())->add(t1)->build();
  shared_ptr<BlendedTermQuery> bt2 =
      (make_shared<BlendedTermQuery::Builder>())->add(t1)->build();
  QueryUtils::checkEqual(bt1, bt2);

  bt1 = (make_shared<BlendedTermQuery::Builder>())
            ->setRewriteMethod(BlendedTermQuery::BOOLEAN_REWRITE)
            ->add(t1)
            ->build();
  bt2 = (make_shared<BlendedTermQuery::Builder>())
            ->setRewriteMethod(BlendedTermQuery::DISJUNCTION_MAX_REWRITE)
            ->add(t1)
            ->build();
  QueryUtils::checkUnequal(bt1, bt2);

  shared_ptr<Term> t2 = make_shared<Term>(L"foo", L"baz");

  bt1 = (make_shared<BlendedTermQuery::Builder>())->add(t1)->add(t2)->build();
  bt2 = (make_shared<BlendedTermQuery::Builder>())->add(t2)->add(t1)->build();
  QueryUtils::checkEqual(bt1, bt2);

  float boost1 = random()->nextFloat();
  float boost2 = random()->nextFloat();
  bt1 = (make_shared<BlendedTermQuery::Builder>())
            ->add(t1, boost1)
            ->add(t2, boost2)
            ->build();
  bt2 = (make_shared<BlendedTermQuery::Builder>())
            ->add(t2, boost2)
            ->add(t1, boost1)
            ->build();
  QueryUtils::checkEqual(bt1, bt2);
}

void TestBlendedTermQuery::testToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Blended()",
               (make_shared<BlendedTermQuery::Builder>())->build()->toString());
  shared_ptr<Term> t1 = make_shared<Term>(L"foo", L"bar");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"Blended(foo:bar)",
      (make_shared<BlendedTermQuery::Builder>())->add(t1)->build()->toString());
  shared_ptr<Term> t2 = make_shared<Term>(L"foo", L"baz");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Blended(foo:bar foo:baz)",
               (make_shared<BlendedTermQuery::Builder>())
                   ->add(t1)
                   ->add(t2)
                   ->build()
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Blended((foo:bar)^4.0 (foo:baz)^3.0)",
               (make_shared<BlendedTermQuery::Builder>())
                   ->add(t1, 4)
                   ->add(t2, 3)
                   ->build()
                   ->toString());
}

void TestBlendedTermQuery::testBlendedScores() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"a", Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"b", Store::NO));
  for (int i = 0; i < 10; ++i) {
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<BlendedTermQuery> query =
      (make_shared<BlendedTermQuery::Builder>())
          ->setRewriteMethod(
              make_shared<BlendedTermQuery::DisjunctionMaxRewrite>(0.0f))
          ->add(make_shared<Term>(L"f", L"a"))
          ->add(make_shared<Term>(L"f", L"b"))
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 20);
  assertEquals(11, topDocs->totalHits);
  // All docs must have the same score
  for (int i = 0; i < topDocs->scoreDocs.size(); ++i) {
    assertEquals(topDocs->scoreDocs[0]->score, topDocs->scoreDocs[i]->score,
                 0.0f);
  }

  delete reader;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::search