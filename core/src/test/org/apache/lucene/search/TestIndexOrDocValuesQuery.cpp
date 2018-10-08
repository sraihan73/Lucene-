using namespace std;

#include "TestIndexOrDocValuesQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexOrDocValuesQuery::testUseIndexForSelectiveQueries() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig()->setCodec(TestUtil::getDefaultCodec()));
  for (int i = 0; i < 2000; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (i == 42) {
      doc->push_back(make_shared<StringField>(L"f1", L"bar", Store::NO));
      doc->push_back(make_shared<LongPoint>(L"f2", 42LL));
      doc->push_back(make_shared<NumericDocValuesField>(L"f2", 42LL));
    } else if (i == 100) {
      doc->push_back(make_shared<StringField>(L"f1", L"foo", Store::NO));
      doc->push_back(make_shared<LongPoint>(L"f2", 2LL));
      doc->push_back(make_shared<NumericDocValuesField>(L"f2", 2LL));
    } else {
      doc->push_back(make_shared<StringField>(L"f1", L"bar", Store::NO));
      doc->push_back(make_shared<LongPoint>(L"f2", 2LL));
      doc->push_back(make_shared<NumericDocValuesField>(L"f2", 2LL));
    }
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> reader = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setQueryCache(nullptr);

  // The term query is more selective, so the IndexOrDocValuesQuery should use
  // doc values
  shared_ptr<Query> *const q1 =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"f1", L"foo")),
                Occur::MUST)
          ->add(make_shared<IndexOrDocValuesQuery>(
                    LongPoint::newExactQuery(L"f2", 2),
                    NumericDocValuesField::newSlowRangeQuery(L"f2", 2LL, 2LL)),
                Occur::MUST)
          ->build();

  shared_ptr<Weight> *const w1 =
      searcher->createWeight(searcher->rewrite(q1), random()->nextBoolean(), 1);
  shared_ptr<Scorer> *const s1 =
      w1->scorer(searcher->getIndexReader()->leaves()[0]);
  assertNotNull(s1->twoPhaseIterator()); // means we use doc values

  // The term query is less selective, so the IndexOrDocValuesQuery should use
  // points
  shared_ptr<Query> *const q2 =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"f1", L"bar")),
                Occur::MUST)
          ->add(
              make_shared<IndexOrDocValuesQuery>(
                  LongPoint::newExactQuery(L"f2", 42),
                  NumericDocValuesField::newSlowRangeQuery(L"f2", 42LL, 42LL)),
              Occur::MUST)
          ->build();

  shared_ptr<Weight> *const w2 =
      searcher->createWeight(searcher->rewrite(q2), random()->nextBoolean(), 1);
  shared_ptr<Scorer> *const s2 =
      w2->scorer(searcher->getIndexReader()->leaves()[0]);
  assertNull(s2->twoPhaseIterator()); // means we use points

  delete reader;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::search