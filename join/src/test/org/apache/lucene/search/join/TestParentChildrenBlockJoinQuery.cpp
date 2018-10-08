using namespace std;

#include "TestParentChildrenBlockJoinQuery.h"

namespace org::apache::lucene::search::join
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestParentChildrenBlockJoinQuery::testParentChildrenBlockJoinQuery() throw(
    runtime_error)
{
  int numParentDocs = 8 + random()->nextInt(8);
  int maxChildDocsPerParent = 8 + random()->nextInt(8);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < numParentDocs; i++) {
    int numChildDocs = random()->nextInt(maxChildDocsPerParent);
    deque<std::shared_ptr<Document>> docs =
        deque<std::shared_ptr<Document>>(numChildDocs + 1);
    for (int j = 0; j < numChildDocs; j++) {
      shared_ptr<Document> childDoc = make_shared<Document>();
      childDoc->push_back(
          make_shared<StringField>(L"type", L"child", Field::Store::NO));
      childDoc->push_back(make_shared<NumericDocValuesField>(L"score", j + 1));
      docs.push_back(childDoc);
    }

    shared_ptr<Document> parenDoc = make_shared<Document>();
    parenDoc->push_back(
        make_shared<StringField>(L"type", L"parent", Field::Store::NO));
    parenDoc->push_back(
        make_shared<NumericDocValuesField>(L"num_child_docs", numChildDocs));
    docs.push_back(parenDoc);
    writer->addDocuments(docs);
  }

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"type", L"parent")));
  shared_ptr<Query> childQuery =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"type", L"child")),
                BooleanClause::Occur::FILTER)
          ->add(TestJoinUtil::numericDocValuesScoreQuery(L"score"),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> parentDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"type", L"parent")),
      numParentDocs);
  assertEquals(parentDocs->scoreDocs.size(), numParentDocs);
  for (auto parentScoreDoc : parentDocs->scoreDocs) {
    shared_ptr<LeafReaderContext> leafReader =
        reader->leaves()[ReaderUtil::subIndex(parentScoreDoc->doc,
                                              reader->leaves())];
    shared_ptr<NumericDocValues> numericDocValuesField =
        leafReader->reader()->getNumericDocValues(L"num_child_docs");
    numericDocValuesField->advance(parentScoreDoc->doc - leafReader->docBase);
    int64_t expectedChildDocs = numericDocValuesField->longValue();

    shared_ptr<ParentChildrenBlockJoinQuery> parentChildrenBlockJoinQuery =
        make_shared<ParentChildrenBlockJoinQuery>(parentFilter, childQuery,
                                                  parentScoreDoc->doc);
    shared_ptr<TopDocs> topDocs =
        searcher->search(parentChildrenBlockJoinQuery, maxChildDocsPerParent);
    assertEquals(expectedChildDocs, topDocs->totalHits);
    if (expectedChildDocs > 0) {
      assertEquals(expectedChildDocs, topDocs->getMaxScore(), 0);
      for (int i = 0; i < topDocs->scoreDocs.size(); i++) {
        shared_ptr<ScoreDoc> childScoreDoc = topDocs->scoreDocs[i];
        assertEquals(expectedChildDocs - i, childScoreDoc->score, 0);
      }
    }
  }

  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::search::join