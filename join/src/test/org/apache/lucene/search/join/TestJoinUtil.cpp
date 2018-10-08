using namespace std;

#include "TestJoinUtil.h"

namespace org::apache::lucene::search::join
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using namespace org::apache::lucene::search;
using Directory = org::apache::lucene::store::Directory;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using org::junit::Test;

void TestJoinUtil::testSimple() 
{
  const wstring idField = L"id";
  const wstring toField = L"productId";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"description", L"random text", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"2", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"2")));
  doc->push_back(make_shared<TextField>(toField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"3", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"3")));
  doc->push_back(make_shared<TextField>(toField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 3
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"description", L"more random text",
                                        Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name2", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"4", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"4")));
  w->addDocument(doc);
  w->commit();

  // 4
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"5", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"5")));
  doc->push_back(make_shared<TextField>(toField, L"4", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"4")));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"6", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"6")));
  doc->push_back(make_shared<TextField>(toField, L"4", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"4")));
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(w->getReader());
  delete w;

  // Search for product
  shared_ptr<Query> joinQuery = JoinUtil::createJoinQuery(
      idField, false, toField,
      make_shared<TermQuery>(make_shared<Term>(L"name", L"name2")),
      indexSearcher, ScoreMode::None);

  shared_ptr<TopDocs> result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(4, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(5, result->scoreDocs[1]->doc);

  joinQuery = JoinUtil::createJoinQuery(
      idField, false, toField,
      make_shared<TermQuery>(make_shared<Term>(L"name", L"name1")),
      indexSearcher, ScoreMode::None);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(1, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(2, result->scoreDocs[1]->doc);

  // Search for offer
  joinQuery = JoinUtil::createJoinQuery(
      toField, false, idField,
      make_shared<TermQuery>(make_shared<Term>(L"id", L"5")), indexSearcher,
      ScoreMode::None);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(1, result->totalHits);
  TestUtil::assertEquals(3, result->scoreDocs[0]->doc);

  delete indexSearcher->getIndexReader();
  delete dir;
}

void TestJoinUtil::testSimpleOrdinalsJoin() 
{
  const wstring idField = L"id";
  const wstring productIdField = L"productId";
  // A field indicating to what type a document belongs, which is then used to
  // distinques between documents during joining.
  const wstring typeField = L"type";
  // A single sorted doc values field that holds the join values for all
  // document types. Typically during indexing a schema will automatically
  // create this field with the values
  const wstring joinField = idField + productIdField;

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(NoMergePolicy::INSTANCE));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(idField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(typeField, L"product", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"description", L"random text", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name1", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 3
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(idField, L"2", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(typeField, L"product", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"description", L"more random text",
                                        Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name2", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"2")));
  w->addDocument(doc);
  w->commit();

  // 4
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"2", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"2")));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"2", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"2")));
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(w->getReader());
  delete w;

  shared_ptr<IndexReader> r = indexSearcher->getIndexReader();
  std::deque<std::shared_ptr<SortedDocValues>> values(r->leaves().size());
  for (int i = 0; i < values.size(); i++) {
    shared_ptr<LeafReader> leafReader = r->leaves()[i]->reader();
    values[i] = DocValues::getSorted(leafReader, joinField);
  }
  shared_ptr<OrdinalMap> ordinalMap =
      OrdinalMap::build(nullptr, values, PackedInts::DEFAULT);

  shared_ptr<Query> toQuery =
      make_shared<TermQuery>(make_shared<Term>(typeField, L"price"));
  shared_ptr<Query> fromQuery =
      make_shared<TermQuery>(make_shared<Term>(L"name", L"name2"));
  // Search for product and return prices
  shared_ptr<Query> joinQuery =
      JoinUtil::createJoinQuery(joinField, fromQuery, toQuery, indexSearcher,
                                ScoreMode::None, ordinalMap);
  shared_ptr<TopDocs> result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(4, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(5, result->scoreDocs[1]->doc);

  fromQuery = make_shared<TermQuery>(make_shared<Term>(L"name", L"name1"));
  joinQuery =
      JoinUtil::createJoinQuery(joinField, fromQuery, toQuery, indexSearcher,
                                ScoreMode::None, ordinalMap);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(1, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(2, result->scoreDocs[1]->doc);

  // Search for prices and return products
  fromQuery = make_shared<TermQuery>(make_shared<Term>(L"price", L"20.0"));
  toQuery = make_shared<TermQuery>(make_shared<Term>(typeField, L"product"));
  joinQuery =
      JoinUtil::createJoinQuery(joinField, fromQuery, toQuery, indexSearcher,
                                ScoreMode::None, ordinalMap);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(0, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(3, result->scoreDocs[1]->doc);

  delete indexSearcher->getIndexReader();
  delete dir;
}

void TestJoinUtil::testOrdinalsJoinExplainNoMatches() 
{
  const wstring idField = L"id";
  const wstring productIdField = L"productId";
  // A field indicating to what type a document belongs, which is then used to
  // distinques between documents during joining.
  const wstring typeField = L"type";
  // A single sorted doc values field that holds the join values for all
  // document types. Typically during indexing a schema will automatically
  // create this field with the values
  const wstring joinField = idField + productIdField;

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(NoMergePolicy::INSTANCE));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(idField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(typeField, L"product", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"description", L"random text", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name1", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(idField, L"2", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(typeField, L"product", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"description", L"random text", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name2", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"2")));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 3
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"2", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  if (random()->nextBoolean()) {
    w->flush();
  }

  // 4
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(productIdField, L"3", Field::Store::NO));
  doc->push_back(make_shared<TextField>(typeField, L"price", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"price", L"5.0", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      joinField, make_shared<BytesRef>(L"2")));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"field", L"value", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
  std::deque<std::shared_ptr<SortedDocValues>> values(r->leaves().size());
  for (int i = 0; i < values.size(); i++) {
    shared_ptr<LeafReader> leafReader = r->leaves()[i]->reader();
    values[i] = DocValues::getSorted(leafReader, joinField);
  }
  shared_ptr<OrdinalMap> ordinalMap =
      OrdinalMap::build(nullptr, values, PackedInts::DEFAULT);

  shared_ptr<Query> toQuery =
      make_shared<TermQuery>(make_shared<Term>(L"price", L"5.0"));
  shared_ptr<Query> fromQuery =
      make_shared<TermQuery>(make_shared<Term>(L"name", L"name2"));

  for (ScoreMode scoreMode : ScoreMode::values()) {
    shared_ptr<Query> joinQuery = JoinUtil::createJoinQuery(
        joinField, fromQuery, toQuery, indexSearcher, scoreMode, ordinalMap);
    shared_ptr<TopDocs> result = indexSearcher->search(joinQuery, 10);
    TestUtil::assertEquals(1, result->totalHits);
    TestUtil::assertEquals(4, result->scoreDocs[0]->doc); // doc with price: 5.0
    shared_ptr<Explanation> explanation = indexSearcher->explain(joinQuery, 4);
    assertTrue(explanation->isMatch());
    TestUtil::assertEquals(explanation->getDescription(),
                           L"A match, join value 2");

    explanation = indexSearcher->explain(joinQuery, 3);
    assertFalse(explanation->isMatch());
    TestUtil::assertEquals(explanation->getDescription(),
                           L"Not a match, join value 1");

    explanation = indexSearcher->explain(joinQuery, 5);
    assertFalse(explanation->isMatch());
    TestUtil::assertEquals(explanation->getDescription(), L"Not a match");
  }

  delete w;
  delete indexSearcher->getIndexReader();
  delete dir;
}

void TestJoinUtil::testRandomOrdinalsJoin() 
{
  shared_ptr<IndexIterationContext> context = createContext(512, false, true);
  int searchIters = 10;
  shared_ptr<IndexSearcher> indexSearcher = context->searcher;
  for (int i = 0; i < searchIters; i++) {
    if (VERBOSE) {
      wcout << L"search iter=" << i << endl;
    }
    int r = random()->nextInt(context->randomUniqueValues.size());
    bool from = context->randomFrom[r];
    wstring randomValue = context->randomUniqueValues[r];
    shared_ptr<BitSet> expectedResult = createExpectedResult(
        randomValue, from, indexSearcher->getIndexReader(), context);

    shared_ptr<Query> *const actualQuery =
        make_shared<TermQuery>(make_shared<Term>(L"value", randomValue));
    if (VERBOSE) {
      wcout << L"actualQuery=" << actualQuery << endl;
    }
    constexpr ScoreMode scoreMode =
        ScoreMode::values()[random()->nextInt(ScoreMode::values()->length)];
    if (VERBOSE) {
      wcout << L"scoreMode=" << scoreMode << endl;
    }

    shared_ptr<Query> *const joinQuery;
    if (from) {
      shared_ptr<BooleanQuery::Builder> fromQuery =
          make_shared<BooleanQuery::Builder>();
      fromQuery->add(
          make_shared<TermQuery>(make_shared<Term>(L"type", L"from")),
          BooleanClause::Occur::FILTER);
      fromQuery->add(actualQuery, BooleanClause::Occur::MUST);
      shared_ptr<Query> toQuery =
          make_shared<TermQuery>(make_shared<Term>(L"type", L"to"));
      joinQuery = JoinUtil::createJoinQuery(L"join_field", fromQuery->build(),
                                            toQuery, indexSearcher, scoreMode,
                                            context->ordinalMap);
    } else {
      shared_ptr<BooleanQuery::Builder> fromQuery =
          make_shared<BooleanQuery::Builder>();
      fromQuery->add(make_shared<TermQuery>(make_shared<Term>(L"type", L"to")),
                     BooleanClause::Occur::FILTER);
      fromQuery->add(actualQuery, BooleanClause::Occur::MUST);
      shared_ptr<Query> toQuery =
          make_shared<TermQuery>(make_shared<Term>(L"type", L"from"));
      joinQuery = JoinUtil::createJoinQuery(L"join_field", fromQuery->build(),
                                            toQuery, indexSearcher, scoreMode,
                                            context->ordinalMap);
    }
    if (VERBOSE) {
      wcout << L"joinQuery=" << joinQuery << endl;
    }

    shared_ptr<BitSet> *const actualResult =
        make_shared<FixedBitSet>(indexSearcher->getIndexReader()->maxDoc());
    shared_ptr<TopScoreDocCollector> *const topScoreDocCollector =
        TopScoreDocCollector::create(10);
    indexSearcher->search(
        joinQuery,
        MultiCollector::wrap({make_shared<BitSetCollector>(actualResult),
                              topScoreDocCollector}));
    assertBitSet(expectedResult, actualResult, indexSearcher);
    shared_ptr<TopDocs> expectedTopDocs =
        createExpectedTopDocs(randomValue, from, scoreMode, context);
    shared_ptr<TopDocs> actualTopDocs = topScoreDocCollector->topDocs();
    assertTopDocs(expectedTopDocs, actualTopDocs, scoreMode, indexSearcher,
                  joinQuery);
  }
  context->close();
}

void TestJoinUtil::testMinMaxScore() 
{
  wstring priceField = L"price";
  shared_ptr<Query> priceQuery = numericDocValuesScoreQuery(priceField);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false)));

  unordered_map<wstring, float> lowestScoresPerParent =
      unordered_map<wstring, float>();
  unordered_map<wstring, float> highestScoresPerParent =
      unordered_map<wstring, float>();
  int numParents = RandomNumbers::randomIntBetween(random(), 16, 64);
  for (int p = 0; p < numParents; p++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring parentId = Integer::toString(p);
    shared_ptr<Document> parentDoc = make_shared<Document>();
    parentDoc->push_back(
        make_shared<StringField>(L"id", parentId, Field::Store::YES));
    parentDoc->push_back(
        make_shared<StringField>(L"type", L"to", Field::Store::NO));
    parentDoc->push_back(make_shared<SortedDocValuesField>(
        L"join_field", make_shared<BytesRef>(parentId)));
    iw->addDocument(parentDoc);
    int numChildren = RandomNumbers::randomIntBetween(random(), 2, 16);
    int lowest = numeric_limits<int>::max();
    int highest = numeric_limits<int>::min();
    for (int c = 0; c < numChildren; c++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring childId = Integer::toString(p + c);
      shared_ptr<Document> childDoc = make_shared<Document>();
      childDoc->push_back(
          make_shared<StringField>(L"id", childId, Field::Store::YES));
      childDoc->push_back(
          make_shared<StringField>(L"type", L"from", Field::Store::NO));
      childDoc->push_back(make_shared<SortedDocValuesField>(
          L"join_field", make_shared<BytesRef>(parentId)));
      int price = random()->nextInt(1000);
      childDoc->push_back(
          make_shared<NumericDocValuesField>(priceField, price));
      iw->addDocument(childDoc);
      lowest = min(lowest, price);
      highest = max(highest, price);
    }
    lowestScoresPerParent.emplace(parentId, static_cast<float>(lowest));
    highestScoresPerParent.emplace(parentId, static_cast<float>(highest));
  }
  delete iw;

  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(DirectoryReader::open(dir));
  std::deque<std::shared_ptr<SortedDocValues>> values(
      searcher->getIndexReader()->leaves().size());
  for (auto leadContext : searcher->getIndexReader()->leaves()) {
    values[leadContext->ord] =
        DocValues::getSorted(leadContext->reader(), L"join_field");
  }
  shared_ptr<OrdinalMap> ordinalMap =
      OrdinalMap::build(nullptr, values, PackedInts::DEFAULT);
  shared_ptr<BooleanQuery::Builder> fromQuery =
      make_shared<BooleanQuery::Builder>();
  fromQuery->add(priceQuery, BooleanClause::Occur::MUST);
  shared_ptr<Query> toQuery =
      make_shared<TermQuery>(make_shared<Term>(L"type", L"to"));
  shared_ptr<Query> joinQuery =
      JoinUtil::createJoinQuery(L"join_field", fromQuery->build(), toQuery,
                                searcher, ScoreMode::Min, ordinalMap);
  shared_ptr<TopDocs> topDocs = searcher->search(joinQuery, numParents);
  TestUtil::assertEquals(numParents, topDocs->totalHits);
  for (int i = 0; i < topDocs->scoreDocs.size(); i++) {
    shared_ptr<ScoreDoc> scoreDoc = topDocs->scoreDocs[i];
    wstring id = searcher->doc(scoreDoc->doc)[L"id"];
    assertEquals(lowestScoresPerParent[id], scoreDoc->score, 0.0f);
  }

  joinQuery =
      JoinUtil::createJoinQuery(L"join_field", fromQuery->build(), toQuery,
                                searcher, ScoreMode::Max, ordinalMap);
  topDocs = searcher->search(joinQuery, numParents);
  TestUtil::assertEquals(numParents, topDocs->totalHits);
  for (int i = 0; i < topDocs->scoreDocs.size(); i++) {
    shared_ptr<ScoreDoc> scoreDoc = topDocs->scoreDocs[i];
    wstring id = searcher->doc(scoreDoc->doc)[L"id"];
    assertEquals(highestScoresPerParent[id], scoreDoc->score, 0.0f);
  }

  delete searcher->getIndexReader();
  delete dir;
}

shared_ptr<Query> TestJoinUtil::numericDocValuesScoreQuery(const wstring &field)
{
  return make_shared<QueryAnonymousInnerClass>(field);
}

TestJoinUtil::QueryAnonymousInnerClass::QueryAnonymousInnerClass(
    const wstring &field)
{
  this->field = field;
}

shared_ptr<Weight> TestJoinUtil::QueryAnonymousInnerClass::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  shared_ptr<Weight> fieldWeight =
      fieldQuery::createWeight(searcher, false, boost);
  return make_shared<WeightAnonymousInnerClass>(shared_from_this(),
                                                fieldWeight);
}

TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::
    WeightAnonymousInnerClass(
        shared_ptr<QueryAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::search::Weight> fieldWeight)
    : Weight(outerInstance)
{
  this->outerInstance = outerInstance;
  this->fieldWeight = fieldWeight;
}

void TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::
    extractTerms(shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation>
TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return nullptr;
}

shared_ptr<Scorer>
TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> fieldScorer = fieldWeight->scorer(context);
  if (fieldScorer == nullptr) {
    return nullptr;
  }
  shared_ptr<NumericDocValues> price =
      context->reader()->getNumericDocValues(outerInstance->field);
  return make_shared<FilterScorerAnonymousInnerClass>(shared_from_this(),
                                                      fieldScorer, price);
}

TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::
    FilterScorerAnonymousInnerClass::FilterScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::search::Scorer> fieldScorer,
        shared_ptr<NumericDocValues> price)
    : FilterScorer(fieldScorer, outerInstance)
{
  this->outerInstance = outerInstance;
  this->price = price;
}

float TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::
    FilterScorerAnonymousInnerClass::score() 
{
  TestUtil::assertEquals(in_::docID(), price->advance(in_::docID()));
  return static_cast<float>(price->longValue());
}

bool TestJoinUtil::QueryAnonymousInnerClass::WeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

// C++ TODO: There is no native C++ equivalent to 'toString':
wstring
TestJoinUtil::QueryAnonymousInnerClass::Term::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return fieldQuery->toString(field);
}

bool TestJoinUtil::QueryAnonymousInnerClass::equals(any o)
{
  return o == shared_from_this();
}

int TestJoinUtil::QueryAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

void TestJoinUtil::testMinMaxDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false)));

  int minChildDocsPerParent = 2;
  int maxChildDocsPerParent = 16;
  int numParents = RandomNumbers::randomIntBetween(random(), 16, 64);
  std::deque<int> childDocsPerParent(numParents);
  for (int p = 0; p < numParents; p++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring parentId = Integer::toString(p);
    shared_ptr<Document> parentDoc = make_shared<Document>();
    parentDoc->push_back(
        make_shared<StringField>(L"id", parentId, Field::Store::YES));
    parentDoc->push_back(
        make_shared<StringField>(L"type", L"to", Field::Store::NO));
    parentDoc->push_back(make_shared<SortedDocValuesField>(
        L"join_field", make_shared<BytesRef>(parentId)));
    iw->addDocument(parentDoc);
    int numChildren = RandomNumbers::randomIntBetween(
        random(), minChildDocsPerParent, maxChildDocsPerParent);
    childDocsPerParent[p] = numChildren;
    for (int c = 0; c < numChildren; c++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring childId = Integer::toString(p + c);
      shared_ptr<Document> childDoc = make_shared<Document>();
      childDoc->push_back(
          make_shared<StringField>(L"id", childId, Field::Store::YES));
      childDoc->push_back(
          make_shared<StringField>(L"type", L"from", Field::Store::NO));
      childDoc->push_back(make_shared<SortedDocValuesField>(
          L"join_field", make_shared<BytesRef>(parentId)));
      iw->addDocument(childDoc);
    }
  }
  delete iw;

  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(DirectoryReader::open(dir));
  std::deque<std::shared_ptr<SortedDocValues>> values(
      searcher->getIndexReader()->leaves().size());
  for (auto leadContext : searcher->getIndexReader()->leaves()) {
    values[leadContext->ord] =
        DocValues::getSorted(leadContext->reader(), L"join_field");
  }
  shared_ptr<OrdinalMap> ordinalMap =
      OrdinalMap::build(nullptr, values, PackedInts::DEFAULT);
  shared_ptr<Query> fromQuery =
      make_shared<TermQuery>(make_shared<Term>(L"type", L"from"));
  shared_ptr<Query> toQuery =
      make_shared<TermQuery>(make_shared<Term>(L"type", L"to"));

  int iters = RandomNumbers::randomIntBetween(random(), 3, 9);
  for (int i = 1; i <= iters; i++) {
    constexpr ScoreMode scoreMode =
        ScoreMode::values()[random()->nextInt(ScoreMode::values()->length)];
    int min = RandomNumbers::randomIntBetween(random(), minChildDocsPerParent,
                                              maxChildDocsPerParent - 1);
    int max =
        RandomNumbers::randomIntBetween(random(), min, maxChildDocsPerParent);
    if (VERBOSE) {
      wcout << L"iter=" << i << endl;
      wcout << L"scoreMode=" << scoreMode << endl;
      wcout << L"min=" << min << endl;
      wcout << L"max=" << max << endl;
    }
    shared_ptr<Query> joinQuery =
        JoinUtil::createJoinQuery(L"join_field", fromQuery, toQuery, searcher,
                                  scoreMode, ordinalMap, min, max);
    shared_ptr<TotalHitCountCollector> collector =
        make_shared<TotalHitCountCollector>();
    searcher->search(joinQuery, collector);
    int expectedCount = 0;
    for (auto numChildDocs : childDocsPerParent) {
      if (numChildDocs >= min && numChildDocs <= max) {
        expectedCount++;
      }
    }
    TestUtil::assertEquals(expectedCount, collector->getTotalHits());
  }

  delete searcher->getIndexReader();
  delete dir;
}

void TestJoinUtil::testRewrite() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"join_field", make_shared<BytesRef>(L"abc")));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"join_field", make_shared<BytesRef>(L"abd")));
  w->addDocument(doc);
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<OrdinalMap> ordMap = OrdinalMap::build(
      nullptr, std::deque<std::shared_ptr<SortedDocValues>>(0), 0.0f);
  shared_ptr<Query> joinQuery = JoinUtil::createJoinQuery(
      L"join_field", make_shared<MatchNoDocsQuery>(),
      make_shared<MatchNoDocsQuery>(), searcher,
      RandomPicks::randomFrom(random(), ScoreMode::values()), ordMap, 0,
      numeric_limits<int>::max());
  searcher->search(joinQuery, 1); // no exception due to missing rewrites
  delete reader;
  delete w;
  delete dir;
}

void TestJoinUtil::testOverflowTermsWithScoreCollector() 
{
  test300spartans(true, ScoreMode::Avg);
}

void TestJoinUtil::testOverflowTermsWithScoreCollectorRandom() throw(
    runtime_error)
{
  test300spartans(
      random()->nextBoolean(),
      ScoreMode::values()[random()->nextInt(ScoreMode::values()->length)]);
}

void TestJoinUtil::test300spartans(bool multipleValues,
                                   ScoreMode scoreMode) 
{
  const wstring idField = L"id";
  const wstring toField = L"productId";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"description", L"random text", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"0", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"0")));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));

  if (multipleValues) {
    for (int i = 0; i < 300; i++) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          toField, make_shared<BytesRef>(L"" + to_wstring(i))));
    }
  } else {
    doc->push_back(make_shared<SortedDocValuesField>(
        toField, make_shared<BytesRef>(L"0")));
  }
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(w->getReader());
  delete w;

  // Search for product
  shared_ptr<Query> joinQuery = JoinUtil::createJoinQuery(
      toField, multipleValues, idField,
      make_shared<TermQuery>(make_shared<Term>(L"price", L"10.0")),
      indexSearcher, scoreMode);

  shared_ptr<TopDocs> result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(1, result->totalHits);
  TestUtil::assertEquals(0, result->scoreDocs[0]->doc);

  delete indexSearcher->getIndexReader();
  delete dir;
}

void TestJoinUtil::testInsideBooleanQuery() 
{
  const wstring idField = L"id";
  const wstring toField = L"productId";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"description", L"random text", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"7", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"7")));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"2", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"2")));
  doc->push_back(make_shared<TextField>(toField, L"7", Field::Store::NO));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"3", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"3")));
  doc->push_back(make_shared<TextField>(toField, L"7", Field::Store::NO));
  w->addDocument(doc);

  // 3
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"description", L"more random text",
                                        Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"name2", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"0", Field::Store::NO));
  w->addDocument(doc);
  w->commit();

  // 4
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"10.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"5", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"5")));
  doc->push_back(make_shared<TextField>(toField, L"0", Field::Store::NO));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"price", L"20.0", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"6", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"6")));
  doc->push_back(make_shared<TextField>(toField, L"0", Field::Store::NO));
  w->addDocument(doc);

  w->forceMerge(1);

  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(w->getReader());
  delete w;

  // Search for product
  shared_ptr<Query> joinQuery = JoinUtil::createJoinQuery(
      idField, false, toField,
      make_shared<TermQuery>(make_shared<Term>(L"description", L"random")),
      indexSearcher, ScoreMode::Avg);

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(joinQuery, BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"id", L"3")),
          BooleanClause::Occur::SHOULD);

  indexSearcher->search(
      bq->build(),
      make_shared<SimpleCollectorAnonymousInnerClass>(shared_from_this()));

  delete indexSearcher->getIndexReader();
  delete dir;
}

TestJoinUtil::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<TestJoinUtil> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass::collect(int docID)
{
  // Hairy / evil (depends on how BooleanScorer
  // stores temporarily collected docIDs by
  // appending to head of linked deque):
  if (docID == 5) {
    sawFive = true;
  } else if (docID == 1) {
    assertFalse(L"optimized bulkScorer was not used for join query embedded in "
                L"bool query!",
                sawFive);
  }
}

bool TestJoinUtil::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void TestJoinUtil::testSimpleWithScoring() 
{
  const wstring idField = L"id";
  const wstring toField = L"movieId";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"description", L"A random movie",
                                        Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"Movie 1", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"subtitle", L"The first subtitle of this movie", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"2", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"2")));
  doc->push_back(make_shared<TextField>(toField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"subtitle", L"random subtitle; random event movie", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"3", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"3")));
  doc->push_back(make_shared<TextField>(toField, L"1", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 3
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"description", L"A second random movie", Field::Store::NO));
  doc->push_back(make_shared<TextField>(L"name", L"Movie 2", Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"4", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"4")));
  w->addDocument(doc);
  w->commit();

  // 4
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"subtitle", L"a very random event happened during christmas night",
      Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"5", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"5")));
  doc->push_back(make_shared<TextField>(toField, L"4", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"4")));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"subtitle", L"movie end movie test 123 test 123 random",
      Field::Store::NO));
  doc->push_back(make_shared<TextField>(idField, L"6", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(idField, make_shared<BytesRef>(L"6")));
  doc->push_back(make_shared<TextField>(toField, L"4", Field::Store::NO));
  doc->push_back(
      make_shared<SortedDocValuesField>(toField, make_shared<BytesRef>(L"4")));
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(w->getReader());
  delete w;

  // Search for movie via subtitle
  shared_ptr<Query> joinQuery = JoinUtil::createJoinQuery(
      toField, false, idField,
      make_shared<TermQuery>(make_shared<Term>(L"subtitle", L"random")),
      indexSearcher, ScoreMode::Max);
  shared_ptr<TopDocs> result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(0, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(3, result->scoreDocs[1]->doc);

  // Score mode max.
  joinQuery = JoinUtil::createJoinQuery(
      toField, false, idField,
      make_shared<TermQuery>(make_shared<Term>(L"subtitle", L"movie")),
      indexSearcher, ScoreMode::Max);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(3, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(0, result->scoreDocs[1]->doc);

  // Score mode total
  joinQuery = JoinUtil::createJoinQuery(
      toField, false, idField,
      make_shared<TermQuery>(make_shared<Term>(L"subtitle", L"movie")),
      indexSearcher, ScoreMode::Total);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(0, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(3, result->scoreDocs[1]->doc);

  // Score mode avg
  joinQuery = JoinUtil::createJoinQuery(
      toField, false, idField,
      make_shared<TermQuery>(make_shared<Term>(L"subtitle", L"movie")),
      indexSearcher, ScoreMode::Avg);
  result = indexSearcher->search(joinQuery, 10);
  TestUtil::assertEquals(2, result->totalHits);
  TestUtil::assertEquals(3, result->scoreDocs[0]->doc);
  TestUtil::assertEquals(0, result->scoreDocs[1]->doc);

  delete indexSearcher->getIndexReader();
  delete dir;
}

void TestJoinUtil::testEquals() 
{
  constexpr int numDocs = atLeast(random(), 50);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final org.apache.lucene.store.Directory dir
  // = newDirectory())
  {
    shared_ptr<org::apache::lucene::store::Directory> *const dir =
        newDirectory();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final
    // org.apache.lucene.index.RandomIndexWriter w = new
    // org.apache.lucene.index.RandomIndexWriter(random(), dir,
    // newIndexWriterConfig(new
    // org.apache.lucene.analysis.MockAnalyzer(random())).setMergePolicy(newLogMergePolicy())))
    {
      shared_ptr<org::apache::lucene::index::RandomIndexWriter> *const w =
          make_shared<org.apache::lucene::index::RandomIndexWriter>(
              random(), dir,
              newIndexWriterConfig(
                  make_shared<org.apache::lucene::analysis::MockAnalyzer>(
                      random()))
                  ->setMergePolicy(newLogMergePolicy()));
      bool multiValued = random()->nextBoolean();
      wstring joinField = multiValued ? L"mvField" : L"svField";
      for (int id = 0; id < numDocs; id++) {
        shared_ptr<Document> doc =
            make_shared<org.apache::lucene::document::Document>();
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"id", L"" + to_wstring(id),
            org.apache::lucene::document::Field::Store::NO));
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"name", L"name" + (id % 7),
            org.apache::lucene::document::Field::Store::NO));
        if (multiValued) {
          int numValues = 1 + random()->nextInt(2);
          for (int i = 0; i < numValues; i++) {
            doc->push_back(
                make_shared<
                    org.apache::lucene::document::SortedSetDocValuesField>(
                    joinField, make_shared<org.apache::lucene::util::BytesRef>(
                                   L"" + random()->nextInt(13))));
          }
        } else {
          doc->push_back(
              make_shared<org.apache::lucene::document::SortedDocValuesField>(
                  joinField, make_shared<org.apache::lucene::util::BytesRef>(
                                 L"" + random()->nextInt(13))));
        }
        w->addDocument(doc);
      }

      shared_ptr<Set<ScoreMode>> scoreModes = EnumSet::allOf(ScoreMode::typeid);
      ScoreMode scoreMode1 = RandomPicks::randomFrom(random(), scoreModes);
      scoreModes->remove(scoreMode1);
      ScoreMode scoreMode2 = RandomPicks::randomFrom(random(), scoreModes);

      shared_ptr<Query> *const x;
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // r = w.getReader())
      {
        org::apache::lucene::index::IndexReader r = w->getReader();
        shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
        x = JoinUtil::createJoinQuery(
            joinField, multiValued, joinField,
            make_shared<TermQuery>(make_shared<org.apache::lucene::index::Term>(
                L"name", L"name5")),
            indexSearcher, scoreMode1);
        assertEquals(L"identical calls to createJoinQuery", x,
                     JoinUtil::createJoinQuery(
                         joinField, multiValued, joinField,
                         make_shared<TermQuery>(
                             make_shared<org.apache::lucene::index::Term>(
                                 L"name", L"name5")),
                         indexSearcher, scoreMode1));

        assertFalse(L"score mode (" + scoreMode1 + L" != " + scoreMode2 +
                        L"), but queries are equal",
                    x->equals(JoinUtil::createJoinQuery(
                        joinField, multiValued, joinField,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name5")),
                        indexSearcher, scoreMode2)));

        assertFalse(
            L"from fields (joinField != \"other_field\") but queries equals",
            x->equals(JoinUtil::createJoinQuery(
                joinField, multiValued, L"other_field",
                make_shared<TermQuery>(
                    make_shared<org.apache::lucene::index::Term>(L"name",
                                                                 L"name5")),
                indexSearcher, scoreMode1)));

        assertFalse(
            L"from fields (\"other_field\" != joinField) but queries equals",
            x->equals(JoinUtil::createJoinQuery(
                L"other_field", multiValued, joinField,
                make_shared<TermQuery>(
                    make_shared<org.apache::lucene::index::Term>(L"name",
                                                                 L"name5")),
                indexSearcher, scoreMode1)));

        assertFalse(L"fromQuery (name:name5 != name:name6) but queries equals",
                    x->equals(JoinUtil::createJoinQuery(
                        L"other_field", multiValued, joinField,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name6")),
                        indexSearcher, scoreMode1)));
      }

      for (int i = 0; i < 13; i++) {
        shared_ptr<Document> doc =
            make_shared<org.apache::lucene::document::Document>();
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"id", L"new_id", org.apache::lucene::document::Field::Store::NO));
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"name", L"name5", org.apache::lucene::document::Field::Store::NO));
        if (multiValued) {
          int numValues = 1 + random()->nextInt(2);
          for (int j = 0; j < numValues; j++) {
            doc->push_back(
                make_shared<
                    org.apache::lucene::document::SortedSetDocValuesField>(
                    joinField, make_shared<org.apache::lucene::util::BytesRef>(
                                   L"" + to_wstring(i))));
          }
        } else {
          doc->push_back(
              make_shared<org.apache::lucene::document::SortedDocValuesField>(
                  joinField, make_shared<org.apache::lucene::util::BytesRef>(
                                 L"" + to_wstring(i))));
        }
        w->addDocument(doc);
      }
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // r = w.getReader())
      {
        org::apache::lucene::index::IndexReader r = w->getReader();
        shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
        assertFalse(
            L"Query shouldn't be equal, because different index readers ",
            x->equals(JoinUtil::createJoinQuery(
                joinField, multiValued, joinField,
                make_shared<TermQuery>(
                    make_shared<org.apache::lucene::index::Term>(L"name",
                                                                 L"name5")),
                indexSearcher, scoreMode1)));
      }
    }
  }
}

void TestJoinUtil::testEquals_globalOrdinalsJoin() 
{
  constexpr int numDocs = atLeast(random(), 50);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final org.apache.lucene.store.Directory dir
  // = newDirectory())
  {
    shared_ptr<org::apache::lucene::store::Directory> *const dir =
        newDirectory();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final
    // org.apache.lucene.index.RandomIndexWriter w = new
    // org.apache.lucene.index.RandomIndexWriter(random(), dir,
    // newIndexWriterConfig(new
    // org.apache.lucene.analysis.MockAnalyzer(random())).setMergePolicy(newLogMergePolicy())))
    {
      shared_ptr<org::apache::lucene::index::RandomIndexWriter> *const w =
          make_shared<org.apache::lucene::index::RandomIndexWriter>(
              random(), dir,
              newIndexWriterConfig(
                  make_shared<org.apache::lucene::analysis::MockAnalyzer>(
                      random()))
                  ->setMergePolicy(newLogMergePolicy()));
      wstring joinField = L"field";
      for (int id = 0; id < numDocs; id++) {
        shared_ptr<Document> doc =
            make_shared<org.apache::lucene::document::Document>();
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"id", L"" + to_wstring(id),
            org.apache::lucene::document::Field::Store::NO));
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"name", L"name" + (id % 7),
            org.apache::lucene::document::Field::Store::NO));
        doc->push_back(
            make_shared<org.apache::lucene::document::SortedDocValuesField>(
                joinField, make_shared<org.apache::lucene::util::BytesRef>(
                               L"" + random()->nextInt(13))));
        w->addDocument(doc);
      }

      shared_ptr<Set<ScoreMode>> scoreModes = EnumSet::allOf(ScoreMode::typeid);
      ScoreMode scoreMode1 = RandomPicks::randomFrom(random(), scoreModes);
      scoreModes->remove(scoreMode1);
      ScoreMode scoreMode2 = RandomPicks::randomFrom(random(), scoreModes);

      shared_ptr<Query> *const x;
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // r = w.getReader())
      {
        org::apache::lucene::index::IndexReader r = w->getReader();
        std::deque<std::shared_ptr<SortedDocValues>> values(
            r->leaves().size());
        for (int i = 0; i < values.size(); i++) {
          shared_ptr<LeafReader> leafReader = r->leaves()[i]->reader();
          values[i] = org.apache::lucene::index::DocValues::getSorted(
              leafReader, joinField);
        }
        shared_ptr<OrdinalMap> ordinalMap =
            org.apache::lucene::index::OrdinalMap::build(
                nullptr, values,
                org.apache::lucene::util::packed::PackedInts::DEFAULT);
        shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
        x = JoinUtil::createJoinQuery(
            joinField,
            make_shared<TermQuery>(make_shared<org.apache::lucene::index::Term>(
                L"name", L"name5")),
            make_shared<MatchAllDocsQuery>(), indexSearcher, scoreMode1,
            ordinalMap);
        assertEquals(L"identical calls to createJoinQuery", x,
                     JoinUtil::createJoinQuery(
                         joinField,
                         make_shared<TermQuery>(
                             make_shared<org.apache::lucene::index::Term>(
                                 L"name", L"name5")),
                         make_shared<MatchAllDocsQuery>(), indexSearcher,
                         scoreMode1, ordinalMap));

        assertFalse(L"score mode (" + scoreMode1 + L" != " + scoreMode2 +
                        L"), but queries are equal",
                    x->equals(JoinUtil::createJoinQuery(
                        joinField,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name5")),
                        make_shared<MatchAllDocsQuery>(), indexSearcher,
                        scoreMode2, ordinalMap)));
        assertFalse(L"fromQuery (name:name5 != name:name6) but queries equals",
                    x->equals(JoinUtil::createJoinQuery(
                        joinField,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name6")),
                        make_shared<MatchAllDocsQuery>(), indexSearcher,
                        scoreMode1, ordinalMap)));
      }

      for (int i = 0; i < 13; i++) {
        shared_ptr<Document> doc =
            make_shared<org.apache::lucene::document::Document>();
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"id", L"new_id", org.apache::lucene::document::Field::Store::NO));
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"name", L"name5", org.apache::lucene::document::Field::Store::NO));
        doc->push_back(
            make_shared<org.apache::lucene::document::SortedDocValuesField>(
                joinField, make_shared<org.apache::lucene::util::BytesRef>(
                               L"" + to_wstring(i))));
        w->addDocument(doc);
      }
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // r = w.getReader())
      {
        org::apache::lucene::index::IndexReader r = w->getReader();
        std::deque<std::shared_ptr<SortedDocValues>> values(
            r->leaves().size());
        for (int i = 0; i < values.size(); i++) {
          shared_ptr<LeafReader> leafReader = r->leaves()[i]->reader();
          values[i] = org.apache::lucene::index::DocValues::getSorted(
              leafReader, joinField);
        }
        shared_ptr<OrdinalMap> ordinalMap =
            org.apache::lucene::index::OrdinalMap::build(
                nullptr, values,
                org.apache::lucene::util::packed::PackedInts::DEFAULT);
        shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
        assertFalse(
            L"Query shouldn't be equal, because different index readers ",
            x->equals(JoinUtil::createJoinQuery(
                joinField,
                make_shared<TermQuery>(
                    make_shared<org.apache::lucene::index::Term>(L"name",
                                                                 L"name5")),
                make_shared<MatchAllDocsQuery>(), indexSearcher, scoreMode1,
                ordinalMap)));
      }
    }
  }
}

void TestJoinUtil::testEquals_numericJoin() 
{
  constexpr int numDocs = atLeast(random(), 50);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final org.apache.lucene.store.Directory dir
  // = newDirectory())
  {
    shared_ptr<org::apache::lucene::store::Directory> *const dir =
        newDirectory();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final
    // org.apache.lucene.index.RandomIndexWriter w = new
    // org.apache.lucene.index.RandomIndexWriter(random(), dir,
    // newIndexWriterConfig(new
    // org.apache.lucene.analysis.MockAnalyzer(random())).setMergePolicy(newLogMergePolicy())))
    {
      shared_ptr<org::apache::lucene::index::RandomIndexWriter> *const w =
          make_shared<org.apache::lucene::index::RandomIndexWriter>(
              random(), dir,
              newIndexWriterConfig(
                  make_shared<org.apache::lucene::analysis::MockAnalyzer>(
                      random()))
                  ->setMergePolicy(newLogMergePolicy()));
      bool multiValued = random()->nextBoolean();
      wstring joinField = multiValued ? L"mvField" : L"svField";
      for (int id = 0; id < numDocs; id++) {
        shared_ptr<Document> doc =
            make_shared<org.apache::lucene::document::Document>();
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"id", L"" + to_wstring(id),
            org.apache::lucene::document::Field::Store::NO));
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"name", L"name" + (id % 7),
            org.apache::lucene::document::Field::Store::NO));
        if (multiValued) {
          int numValues = 1 + random()->nextInt(2);
          for (int i = 0; i < numValues; i++) {
            doc->push_back(make_shared<org.apache::lucene::document::IntPoint>(
                joinField, random()->nextInt(13)));
            doc->push_back(
                make_shared<
                    org.apache::lucene::document::SortedNumericDocValuesField>(
                    joinField, random()->nextInt(13)));
          }
        } else {
          doc->push_back(make_shared<org.apache::lucene::document::IntPoint>(
              joinField, random()->nextInt(13)));
          doc->push_back(
              make_shared<org.apache::lucene::document::NumericDocValuesField>(
                  joinField, random()->nextInt(13)));
        }
        w->addDocument(doc);
      }

      shared_ptr<Set<ScoreMode>> scoreModes = EnumSet::allOf(ScoreMode::typeid);
      ScoreMode scoreMode1 = scoreModes->toArray(
          std::deque<ScoreMode>(0))[random()->nextInt(scoreModes->size())];
      scoreModes->remove(scoreMode1);
      ScoreMode scoreMode2 = scoreModes->toArray(
          std::deque<ScoreMode>(0))[random()->nextInt(scoreModes->size())];

      shared_ptr<Query> *const x;
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // r = w.getReader())
      {
        org::apache::lucene::index::IndexReader r = w->getReader();
        shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
        x = JoinUtil::createJoinQuery(
            joinField, multiValued, joinField, Integer::typeid,
            make_shared<TermQuery>(make_shared<org.apache::lucene::index::Term>(
                L"name", L"name5")),
            indexSearcher, scoreMode1);
        assertEquals(L"identical calls to createJoinQuery", x,
                     JoinUtil::createJoinQuery(
                         joinField, multiValued, joinField, Integer::typeid,
                         make_shared<TermQuery>(
                             make_shared<org.apache::lucene::index::Term>(
                                 L"name", L"name5")),
                         indexSearcher, scoreMode1));

        assertFalse(L"score mode (" + scoreMode1 + L" != " + scoreMode2 +
                        L"), but queries are equal",
                    x->equals(JoinUtil::createJoinQuery(
                        joinField, multiValued, joinField, Integer::typeid,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name5")),
                        indexSearcher, scoreMode2)));

        assertFalse(
            L"from fields (joinField != \"other_field\") but queries equals",
            x->equals(JoinUtil::createJoinQuery(
                joinField, multiValued, L"other_field", Integer::typeid,
                make_shared<TermQuery>(
                    make_shared<org.apache::lucene::index::Term>(L"name",
                                                                 L"name5")),
                indexSearcher, scoreMode1)));

        assertFalse(
            L"from fields (\"other_field\" != joinField) but queries equals",
            x->equals(JoinUtil::createJoinQuery(
                L"other_field", multiValued, joinField, Integer::typeid,
                make_shared<TermQuery>(
                    make_shared<org.apache::lucene::index::Term>(L"name",
                                                                 L"name5")),
                indexSearcher, scoreMode1)));

        assertFalse(L"fromQuery (name:name5 != name:name6) but queries equals",
                    x->equals(JoinUtil::createJoinQuery(
                        L"other_field", multiValued, joinField, Integer::typeid,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name6")),
                        indexSearcher, scoreMode1)));
      }

      for (int i = 14; i < 26; i++) {
        shared_ptr<Document> doc =
            make_shared<org.apache::lucene::document::Document>();
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"id", L"new_id", org.apache::lucene::document::Field::Store::NO));
        doc->push_back(make_shared<org.apache::lucene::document::TextField>(
            L"name", L"name5", org.apache::lucene::document::Field::Store::NO));
        if (multiValued) {
          int numValues = 1 + random()->nextInt(2);
          for (int j = 0; j < numValues; j++) {
            doc->push_back(
                make_shared<
                    org.apache::lucene::document::SortedNumericDocValuesField>(
                    joinField, i));
            doc->push_back(make_shared<org.apache::lucene::document::IntPoint>(
                joinField, i));
          }
        } else {
          doc->push_back(
              make_shared<org.apache::lucene::document::NumericDocValuesField>(
                  joinField, i));
          doc->push_back(make_shared<org.apache::lucene::document::IntPoint>(
              joinField, i));
        }
        w->addDocument(doc);
      }
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // r = w.getReader())
      {
        org::apache::lucene::index::IndexReader r = w->getReader();
        shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(r);
        assertFalse(L"Query shouldn't be equal, because new join values have "
                    L"been indexed",
                    x->equals(JoinUtil::createJoinQuery(
                        joinField, multiValued, joinField, Integer::typeid,
                        make_shared<TermQuery>(
                            make_shared<org.apache::lucene::index::Term>(
                                L"name", L"name5")),
                        indexSearcher, scoreMode1)));
      }
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Slow public void testSingleValueRandomJoin() throws
// Exception
void TestJoinUtil::testSingleValueRandomJoin() 
{
  int maxIndexIter = TestUtil::nextInt(random(), 6, 12);
  int maxSearchIter = TestUtil::nextInt(random(), 13, 26);
  executeRandomJoin(false, maxIndexIter, maxSearchIter,
                    TestUtil::nextInt(random(), 87, 764));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Slow public void testMultiValueRandomJoin() throws
// Exception
void TestJoinUtil::testMultiValueRandomJoin() 
{
  int maxIndexIter = TestUtil::nextInt(random(), 3, 6);
  int maxSearchIter = TestUtil::nextInt(random(), 6, 12);
  executeRandomJoin(true, maxIndexIter, maxSearchIter,
                    TestUtil::nextInt(random(), 11, 57));
}

void TestJoinUtil::executeRandomJoin(
    bool multipleValuesPerDocument, int maxIndexIter, int maxSearchIter,
    int numberOfDocumentsToIndex) 
{
  for (int indexIter = 1; indexIter <= maxIndexIter; indexIter++) {
    if (VERBOSE) {
      wcout << L"TEST: indexIter=" << indexIter << L" numDocs="
            << numberOfDocumentsToIndex << endl;
    }
    shared_ptr<IndexIterationContext> context = createContext(
        numberOfDocumentsToIndex, multipleValuesPerDocument, false);
    shared_ptr<IndexSearcher> indexSearcher = context->searcher;
    if (VERBOSE) {
      wcout << L"TEST: got searcher=" << indexSearcher << endl;
    }
    for (int searchIter = 1; searchIter <= maxSearchIter; searchIter++) {
      if (VERBOSE) {
        wcout << L"TEST: searchIter=" << searchIter << endl;
      }

      int r = random()->nextInt(context->randomUniqueValues.size());
      bool from = context->randomFrom[r];
      wstring randomValue = context->randomUniqueValues[r];
      shared_ptr<BitSet> expectedResult = createExpectedResult(
          randomValue, from, indexSearcher->getIndexReader(), context);

      shared_ptr<Query> *const actualQuery =
          make_shared<TermQuery>(make_shared<Term>(L"value", randomValue));
      if (VERBOSE) {
        wcout << L"actualQuery=" << actualQuery << endl;
      }
      constexpr ScoreMode scoreMode =
          ScoreMode::values()[random()->nextInt(ScoreMode::values()->length)];
      if (VERBOSE) {
        wcout << L"scoreMode=" << scoreMode << endl;
      }

      shared_ptr<Query> *const joinQuery;
      {
        // single val can be handled by multiple-vals
        constexpr bool muliValsQuery =
            multipleValuesPerDocument || random()->nextBoolean();
        const wstring fromField = from ? L"from" : L"to";
        const wstring toField = from ? L"to" : L"from";

        int surpriseMe = random()->nextInt(2);
        switch (surpriseMe) {
        case 0: {
          type_info numType;
          wstring suffix;
          if (random()->nextBoolean()) {
            numType = Integer::typeid;
            suffix = L"INT";
          } else if (random()->nextBoolean()) {
            numType = Float::typeid;
            suffix = L"FLOAT";
          } else if (random()->nextBoolean()) {
            numType = Long::typeid;
            suffix = L"LONG";
          } else {
            numType = Double::typeid;
            suffix = L"DOUBLE";
          }
          joinQuery = JoinUtil::createJoinQuery(
              fromField + suffix, muliValsQuery, toField + suffix, numType,
              actualQuery, indexSearcher, scoreMode);
          break;
        }
        case 1:
          joinQuery =
              JoinUtil::createJoinQuery(fromField, muliValsQuery, toField,
                                        actualQuery, indexSearcher, scoreMode);
          break;
        default:
          throw runtime_error(L"unexpected value " + to_wstring(surpriseMe));
        }
      }
      if (VERBOSE) {
        wcout << L"joinQuery=" << joinQuery << endl;
      }

      // Need to know all documents that have matches. TopDocs doesn't give me
      // that and then I'd be also testing TopDocsCollector...
      shared_ptr<BitSet> *const actualResult =
          make_shared<FixedBitSet>(indexSearcher->getIndexReader()->maxDoc());
      shared_ptr<TopScoreDocCollector> *const topScoreDocCollector =
          TopScoreDocCollector::create(10);
      indexSearcher->search(
          joinQuery,
          MultiCollector::wrap({make_shared<BitSetCollector>(actualResult),
                                topScoreDocCollector}));
      // Asserting bit set...
      assertBitSet(expectedResult, actualResult, indexSearcher);
      // Asserting TopDocs...
      shared_ptr<TopDocs> expectedTopDocs =
          createExpectedTopDocs(randomValue, from, scoreMode, context);
      shared_ptr<TopDocs> actualTopDocs = topScoreDocCollector->topDocs();
      assertTopDocs(expectedTopDocs, actualTopDocs, scoreMode, indexSearcher,
                    joinQuery);
    }
    context->close();
  }
}

void TestJoinUtil::assertBitSet(
    shared_ptr<BitSet> expectedResult, shared_ptr<BitSet> actualResult,
    shared_ptr<IndexSearcher> indexSearcher) 
{
  if (VERBOSE) {
    wcout << L"expected cardinality:" << expectedResult->cardinality() << endl;
    shared_ptr<DocIdSetIterator> iterator = make_shared<BitSetIterator>(
        expectedResult, expectedResult->cardinality());
    for (int doc = iterator->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = iterator->nextDoc()) {
      wcout << wstring::format(Locale::ROOT,
                               L"Expected doc[%d] with id value %s", doc,
                               indexSearcher->doc(doc)[L"id"])
            << endl;
    }
    wcout << L"actual cardinality:" << actualResult->cardinality() << endl;
    iterator =
        make_shared<BitSetIterator>(actualResult, actualResult->cardinality());
    for (int doc = iterator->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = iterator->nextDoc()) {
      wcout << wstring::format(Locale::ROOT, L"Actual doc[%d] with id value %s",
                               doc, indexSearcher->doc(doc)[L"id"])
            << endl;
    }
  }
  TestUtil::assertEquals(expectedResult, actualResult);
}

void TestJoinUtil::assertTopDocs(shared_ptr<TopDocs> expectedTopDocs,
                                 shared_ptr<TopDocs> actualTopDocs,
                                 ScoreMode scoreMode,
                                 shared_ptr<IndexSearcher> indexSearcher,
                                 shared_ptr<Query> joinQuery) 
{
  TestUtil::assertEquals(expectedTopDocs->totalHits, actualTopDocs->totalHits);
  TestUtil::assertEquals(expectedTopDocs->scoreDocs.size(),
                         actualTopDocs->scoreDocs.size());
  if (scoreMode == ScoreMode::None) {
    return;
  }

  if (VERBOSE) {
    for (int i = 0; i < expectedTopDocs->scoreDocs.size(); i++) {
      printf(Locale::ENGLISH, L"Expected doc: %d | Actual doc: %d\n",
             expectedTopDocs->scoreDocs[i]->doc,
             actualTopDocs->scoreDocs[i]->doc);
      printf(Locale::ENGLISH, L"Expected score: %f | Actual score: %f\n",
             expectedTopDocs->scoreDocs[i]->score,
             actualTopDocs->scoreDocs[i]->score);
    }
  }
  assertEquals(expectedTopDocs->getMaxScore(), actualTopDocs->getMaxScore(),
               0.0f);

  for (int i = 0; i < expectedTopDocs->scoreDocs.size(); i++) {
    TestUtil::assertEquals(expectedTopDocs->scoreDocs[i]->doc,
                           actualTopDocs->scoreDocs[i]->doc);
    assertEquals(expectedTopDocs->scoreDocs[i]->score,
                 actualTopDocs->scoreDocs[i]->score, 0.0f);
    shared_ptr<Explanation> explanation =
        indexSearcher->explain(joinQuery, expectedTopDocs->scoreDocs[i]->doc);
    assertEquals(expectedTopDocs->scoreDocs[i]->score, explanation->getValue(),
                 0.0f);
  }
}

shared_ptr<IndexIterationContext>
TestJoinUtil::createContext(int nDocs, bool multipleValuesPerDocument,
                            bool globalOrdinalJoin) 
{
  if (globalOrdinalJoin) {
    assertFalse(
        L"ordinal join doesn't support multiple join values per document",
        multipleValuesPerDocument);
  }

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestJoinUtil::random();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random, dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random, MockTokenizer::KEYWORD, false)));

  shared_ptr<IndexIterationContext> context =
      make_shared<IndexIterationContext>();
  int numRandomValues = nDocs / RandomNumbers::randomIntBetween(random, 1, 4);
  context->randomUniqueValues = std::deque<wstring>(numRandomValues);
  shared_ptr<Set<wstring>> trackSet = unordered_set<wstring>();
  context->randomFrom = std::deque<bool>(numRandomValues);
  for (int i = 0; i < numRandomValues; i++) {
    wstring uniqueRandomValue;
    do {
      // the trick is to generate values which will be ordered similarly for
      // string, ints&longs, positive nums makes it easier
      //
      // Additionally in order to avoid precision loss when joining via a float
      // field we can't generate values higher than 0xFFFFFF, so we can't use
      // Integer#MAX_VALUE as upper bound here:
      constexpr int nextInt = random->nextInt(0xFFFFFF);
      uniqueRandomValue = wstring::format(Locale::ROOT, L"%08x", nextInt);
      assert((nextInt == Integer::parseUnsignedInt(uniqueRandomValue, 16)));
    } while (L"" == uniqueRandomValue || trackSet->contains(uniqueRandomValue));

    // Generate unique values and empty strings aren't allowed.
    trackSet->add(uniqueRandomValue);

    context->randomFrom[i] = random->nextBoolean();
    context->randomUniqueValues[i] = uniqueRandomValue;
  }

  deque<wstring> randomUniqueValuesReplica =
      deque<wstring>(Arrays::asList(context->randomUniqueValues));

  std::deque<std::shared_ptr<RandomDoc>> docs(nDocs);
  for (int i = 0; i < nDocs; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring id = Integer::toString(i);
    int randomI = random->nextInt(context->randomUniqueValues.size());
    wstring value = context->randomUniqueValues[randomI];
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(newTextField(random, L"id", id, Field::Store::YES));
    document->push_back(
        newTextField(random, L"value", value, Field::Store::NO));

    bool from = context->randomFrom[randomI];
    int numberOfLinkValues =
        multipleValuesPerDocument
            ? min(2 + random->nextInt(10), context->randomUniqueValues.size())
            : 1;
    docs[i] = make_shared<RandomDoc>(id, numberOfLinkValues, value, from);
    if (globalOrdinalJoin) {
      document->push_back(
          newStringField(L"type", from ? L"from" : L"to", Field::Store::NO));
    }
    const deque<wstring> subValues;
    {
      int start = randomUniqueValuesReplica.size() == numberOfLinkValues
                      ? 0
                      : random->nextInt(randomUniqueValuesReplica.size() -
                                        numberOfLinkValues);
      subValues =
          randomUniqueValuesReplica.subList(start, start + numberOfLinkValues);
      Collections::shuffle(subValues, random);
    }
    for (auto linkValue : subValues) {

      assert((!find(docs[i]->linkValues.begin(), docs[i]->linkValues.end(),
                    linkValue) != docs[i]->linkValues.end()));
      docs[i]->linkValues.push_back(linkValue);
      if (from) {
        if (context->fromDocuments.find(linkValue) ==
            context->fromDocuments.end()) {
          context->fromDocuments.emplace(linkValue, deque<>());
        }
        if (context->randomValueFromDocs.find(value) ==
            context->randomValueFromDocs.end()) {
          context->randomValueFromDocs.emplace(value, deque<>());
        }

        context->fromDocuments[linkValue].push_back(docs[i]);
        context->randomValueFromDocs[value].push_back(docs[i]);
        addLinkFields(random, document, L"from", linkValue,
                      multipleValuesPerDocument, globalOrdinalJoin);

      } else {
        if (context->toDocuments.find(linkValue) ==
            context->toDocuments.end()) {
          context->toDocuments.emplace(linkValue, deque<>());
        }
        if (context->randomValueToDocs.find(value) ==
            context->randomValueToDocs.end()) {
          context->randomValueToDocs.emplace(value, deque<>());
        }

        context->toDocuments[linkValue].push_back(docs[i]);
        context->randomValueToDocs[value].push_back(docs[i]);
        addLinkFields(random, document, L"to", linkValue,
                      multipleValuesPerDocument, globalOrdinalJoin);
      }
    }

    w->addDocument(document);
    if (random->nextInt(10) == 4) {
      w->commit();
    }
    if (VERBOSE) {
      wcout << L"Added document[" << docs[i]->id << L"]: " << document << endl;
    }
  }

  if (random->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"TEST: now force merge" << endl;
    }
    w->forceMerge(1);
  }
  delete w;

  // Pre-compute all possible hits for all unique random values. On top of this
  // also compute all possible score for any ScoreMode.
  shared_ptr<DirectoryReader> topLevelReader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(topLevelReader);
  for (int i = 0; i < context->randomUniqueValues.size(); i++) {
    wstring uniqueRandomValue = context->randomUniqueValues[i];
    const wstring fromField;
    const wstring toField;
    const unordered_map<wstring, unordered_map<int, std::shared_ptr<JoinScore>>>
        queryVals;
    if (context->randomFrom[i]) {
      fromField = L"from";
      toField = L"to";
      queryVals = context->fromHitsToJoinScore;
    } else {
      fromField = L"to";
      toField = L"from";
      queryVals = context->toHitsToJoinScore;
    }
    const unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
        joinValueToJoinScores = unordered_map<std::shared_ptr<BytesRef>,
                                              std::shared_ptr<JoinScore>>();
    if (multipleValuesPerDocument) {
      searcher->search(
          make_shared<TermQuery>(
              make_shared<Term>(L"value", uniqueRandomValue)),
          make_shared<SimpleCollectorAnonymousInnerClass2>(
              shared_from_this(), context, fromField, joinValueToJoinScores));
    } else {
      searcher->search(
          make_shared<TermQuery>(
              make_shared<Term>(L"value", uniqueRandomValue)),
          make_shared<SimpleCollectorAnonymousInnerClass3>(
              shared_from_this(), context, fromField, joinValueToJoinScores));
    }

    const unordered_map<int, std::shared_ptr<JoinScore>> docToJoinScore =
        unordered_map<int, std::shared_ptr<JoinScore>>();
    if (multipleValuesPerDocument) {
      shared_ptr<Terms> terms = MultiFields::getTerms(topLevelReader, toField);
      if (terms != nullptr) {
        shared_ptr<PostingsEnum> postingsEnum = nullptr;
        shared_ptr<SortedSet<std::shared_ptr<BytesRef>>> joinValues =
            set<std::shared_ptr<BytesRef>>();
        joinValues->addAll(joinValueToJoinScores.keySet());
        for (auto joinValue : joinValues) {
          shared_ptr<TermsEnum> termsEnum = terms->begin();
          if (termsEnum->seekExact(joinValue)) {
            postingsEnum =
                termsEnum->postings(postingsEnum, PostingsEnum::NONE);
            shared_ptr<JoinScore> joinScore = joinValueToJoinScores[joinValue];

            for (int doc = postingsEnum->nextDoc();
                 doc != DocIdSetIterator::NO_MORE_DOCS;
                 doc = postingsEnum->nextDoc()) {
              // First encountered join value determines the score.
              // Something to keep in mind for many-to-many relations.
              if (docToJoinScore.find(doc) == docToJoinScore.end()) {
                docToJoinScore.emplace(doc, joinScore);
              }
            }
          }
        }
      }
    } else {
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<SimpleCollectorAnonymousInnerClass4>(
                           shared_from_this(), context, toField,
                           joinValueToJoinScores, docToJoinScore));
    }
    queryVals.emplace(uniqueRandomValue, docToJoinScore);
  }

  if (globalOrdinalJoin) {
    std::deque<std::shared_ptr<SortedDocValues>> values(
        topLevelReader->leaves()->size());
    for (shared_ptr<LeafReaderContext> leadContext : topLevelReader->leaves()) {
      values[leadContext->ord] =
          DocValues::getSorted(leadContext->reader(), L"join_field");
    }
    context->ordinalMap =
        OrdinalMap::build(nullptr, values, PackedInts::DEFAULT);
  }

  context->searcher = searcher;
  context->dir = dir;
  return context;
}

TestJoinUtil::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        shared_ptr<TestJoinUtil> outerInstance,
        shared_ptr<org::apache::lucene::search::join::TestJoinUtil::
                       IndexIterationContext>
            context,
        const wstring &fromField,
        unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
            &joinValueToJoinScores)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->fromField = fromField;
  this->joinValueToJoinScores = joinValueToJoinScores;
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass2::collect(int doc) throw(
    IOException)
{
  if (doc > docTermOrds::docID()) {
    docTermOrds::advance(doc);
  }
  if (doc == docTermOrds::docID()) {
    int64_t ord;
    while ((ord = docTermOrds::nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
      shared_ptr<BytesRef> *const joinValue = docTermOrds::lookupOrd(ord);
      shared_ptr<JoinScore> joinScore = joinValueToJoinScores[joinValue];
      if (joinScore == nullptr) {
        joinValueToJoinScores.emplace(BytesRef::deepCopyOf(joinValue),
                                      joinScore = make_shared<JoinScore>());
      }
      joinScore->addScore(scorer::score());
    }
  }
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass2::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docTermOrds = DocValues::getSortedSet(context->reader(), fromField);
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

bool TestJoinUtil::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return true;
}

TestJoinUtil::SimpleCollectorAnonymousInnerClass3::
    SimpleCollectorAnonymousInnerClass3(
        shared_ptr<TestJoinUtil> outerInstance,
        shared_ptr<org::apache::lucene::search::join::TestJoinUtil::
                       IndexIterationContext>
            context,
        const wstring &fromField,
        unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
            &joinValueToJoinScores)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->fromField = fromField;
  this->joinValueToJoinScores = joinValueToJoinScores;
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass3::collect(int doc) throw(
    IOException)
{
  shared_ptr<BytesRef> *const joinValue;
  if (terms::advanceExact(doc)) {
    joinValue = terms::binaryValue();
  } else {
    // missing;
    return;
  }

  shared_ptr<JoinScore> joinScore = joinValueToJoinScores[joinValue];
  if (joinScore == nullptr) {
    joinValueToJoinScores.emplace(BytesRef::deepCopyOf(joinValue),
                                  joinScore = make_shared<JoinScore>());
  }
  if (VERBOSE) {
    wcout << L"expected val=" << joinValue->utf8ToString()
          << L" expected score=" << scorer::score() << endl;
  }
  joinScore->addScore(scorer::score());
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass3::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  terms = DocValues::getBinary(context->reader(), fromField);
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass3::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

bool TestJoinUtil::SimpleCollectorAnonymousInnerClass3::needsScores()
{
  return true;
}

TestJoinUtil::SimpleCollectorAnonymousInnerClass4::
    SimpleCollectorAnonymousInnerClass4(
        shared_ptr<TestJoinUtil> outerInstance,
        shared_ptr<org::apache::lucene::search::join::TestJoinUtil::
                       IndexIterationContext>
            context,
        const wstring &toField,
        unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
            &joinValueToJoinScores,
        unordered_map<int, std::shared_ptr<JoinScore>> &docToJoinScore)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->toField = toField;
  this->joinValueToJoinScores = joinValueToJoinScores;
  this->docToJoinScore = docToJoinScore;
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass4::collect(int doc) throw(
    IOException)
{
  shared_ptr<BytesRef> *const joinValue;
  if (terms::advanceExact(doc)) {
    joinValue = terms::binaryValue();
  } else {
    // missing;
    joinValue = make_shared<BytesRef>(BytesRef::EMPTY_BYTES);
  }
  shared_ptr<JoinScore> joinScore = joinValueToJoinScores[joinValue];
  if (joinScore == nullptr) {
    return;
  }
  docToJoinScore.emplace(docBase + doc, joinScore);
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass4::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  terms = DocValues::getBinary(context->reader(), toField);
  docBase = context->docBase;
}

void TestJoinUtil::SimpleCollectorAnonymousInnerClass4::setScorer(
    shared_ptr<Scorer> scorer)
{
}

bool TestJoinUtil::SimpleCollectorAnonymousInnerClass4::needsScores()
{
  return false;
}

void TestJoinUtil::addLinkFields(shared_ptr<Random> random,
                                 shared_ptr<Document> document,
                                 const wstring &fieldName,
                                 const wstring &linkValue,
                                 bool multipleValuesPerDocument,
                                 bool globalOrdinalJoin)
{
  document->push_back(
      newTextField(random, fieldName, linkValue, Field::Store::NO));

  constexpr int linkInt = Integer::parseUnsignedInt(linkValue, 16);
  document->push_back(make_shared<IntPoint>(fieldName + L"INT", linkInt));
  document->push_back(make_shared<FloatPoint>(fieldName + L"FLOAT", linkInt));

  constexpr int64_t linkLong = linkInt << 32 | linkInt;
  document->push_back(make_shared<LongPoint>(fieldName + L"LONG", linkLong));
  document->push_back(
      make_shared<DoublePoint>(fieldName + L"DOUBLE", linkLong));

  if (multipleValuesPerDocument) {
    document->push_back(make_shared<SortedSetDocValuesField>(
        fieldName, make_shared<BytesRef>(linkValue)));
    document->push_back(
        make_shared<SortedNumericDocValuesField>(fieldName + L"INT", linkInt));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        fieldName + L"FLOAT", Float::floatToRawIntBits(linkInt)));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        fieldName + L"LONG", linkLong));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        fieldName + L"DOUBLE", Double::doubleToRawLongBits(linkLong)));
  } else {
    document->push_back(make_shared<SortedDocValuesField>(
        fieldName, make_shared<BytesRef>(linkValue)));
    document->push_back(
        make_shared<NumericDocValuesField>(fieldName + L"INT", linkInt));
    document->push_back(
        make_shared<FloatDocValuesField>(fieldName + L"FLOAT", linkInt));
    document->push_back(
        make_shared<NumericDocValuesField>(fieldName + L"LONG", linkLong));
    document->push_back(
        make_shared<DoubleDocValuesField>(fieldName + L"DOUBLE", linkLong));
  }
  if (globalOrdinalJoin) {
    document->push_back(make_shared<SortedDocValuesField>(
        L"join_field", make_shared<BytesRef>(linkValue)));
  }
}

shared_ptr<TopDocs>
TestJoinUtil::createExpectedTopDocs(const wstring &queryValue, bool const from,
                                    ScoreMode const scoreMode,
                                    shared_ptr<IndexIterationContext> context)
{

  unordered_map<int, std::shared_ptr<JoinScore>> hitsToJoinScores;
  if (from) {
    hitsToJoinScores = context->fromHitsToJoinScore[queryValue];
  } else {
    hitsToJoinScores = context->toHitsToJoinScore[queryValue];
  }
  deque<unordered_map::Entry<int, std::shared_ptr<JoinScore>>> hits =
      deque<unordered_map::Entry<int, std::shared_ptr<JoinScore>>>(
          hitsToJoinScores.entrySet());
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(hits, new
  // java.util.Comparator<java.util.Map.Entry<int, JoinScore>>()
  sort(hits.begin(), hits.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this(),
                                                  scoreMode));
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs(min(10, hits.size()));
  for (int i = 0; i < scoreDocs.size(); i++) {
    unordered_map::Entry<int, std::shared_ptr<JoinScore>> hit = hits[i];
    scoreDocs[i] =
        make_shared<ScoreDoc>(hit.getKey(), hit.getValue().score(scoreMode));
  }
  return make_shared<TopDocs>(
      hits.size(), scoreDocs,
      hits.empty() ? NAN : hits[0].getValue().score(scoreMode));
}

TestJoinUtil::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestJoinUtil> outerInstance,
    org::apache::lucene::search::join::ScoreMode scoreMode)
{
  this->outerInstance = outerInstance;
  this->scoreMode = scoreMode;
}

int TestJoinUtil::ComparatorAnonymousInnerClass::compare(
    unordered_map::Entry<int, std::shared_ptr<JoinScore>> hit1,
    unordered_map::Entry<int, std::shared_ptr<JoinScore>> hit2)
{
  float score1 = hit1.getValue().score(scoreMode);
  float score2 = hit2.getValue().score(scoreMode);

  int cmp = Float::compare(score2, score1);
  if (cmp != 0) {
    return cmp;
  }
  return hit1.getKey() - hit2.getKey();
}

shared_ptr<BitSet> TestJoinUtil::createExpectedResult(
    const wstring &queryValue, bool from,
    shared_ptr<IndexReader> topLevelReader,
    shared_ptr<IndexIterationContext> context) 
{
  const unordered_map<wstring, deque<std::shared_ptr<RandomDoc>>>
      randomValueDocs;
  const unordered_map<wstring, deque<std::shared_ptr<RandomDoc>>>
      linkValueDocuments;
  if (from) {
    randomValueDocs = context->randomValueFromDocs;
    linkValueDocuments = context->toDocuments;
  } else {
    randomValueDocs = context->randomValueToDocs;
    linkValueDocuments = context->fromDocuments;
  }

  shared_ptr<BitSet> expectedResult =
      make_shared<FixedBitSet>(topLevelReader->maxDoc());
  deque<std::shared_ptr<RandomDoc>> matchingDocs = randomValueDocs[queryValue];
  if (matchingDocs.empty()) {
    return make_shared<FixedBitSet>(topLevelReader->maxDoc());
  }

  for (auto matchingDoc : matchingDocs) {
    for (auto linkValue : matchingDoc->linkValues) {
      deque<std::shared_ptr<RandomDoc>> otherMatchingDocs =
          linkValueDocuments[linkValue];
      if (otherMatchingDocs.empty()) {
        continue;
      }

      for (auto otherSideDoc : otherMatchingDocs) {
        shared_ptr<PostingsEnum> postingsEnum = MultiFields::getTermDocsEnum(
            topLevelReader, L"id", make_shared<BytesRef>(otherSideDoc->id), 0);
        assert(postingsEnum != nullptr);
        int doc = postingsEnum->nextDoc();
        expectedResult->set(doc);
      }
    }
  }
  return expectedResult;
}

TestJoinUtil::IndexIterationContext::~IndexIterationContext()
{
  delete searcher->getIndexReader();
  delete dir;
}

TestJoinUtil::RandomDoc::RandomDoc(const wstring &id, int numberOfLinkValues,
                                   const wstring &value, bool from)
    : id(id), linkValues(deque<>(numberOfLinkValues)), value(value), from(from)
{
}

void TestJoinUtil::JoinScore::addScore(float score)
{
  if (score > maxScore) {
    maxScore = score;
  }
  if (score < minScore) {
    minScore = score;
  }
  total += score;
  count++;
}

float TestJoinUtil::JoinScore::score(ScoreMode mode)
{
  switch (mode) {
  case org::apache::lucene::search::join::ScoreMode::None:
    return 1.0f;
  case org::apache::lucene::search::join::ScoreMode::Total:
    return total;
  case org::apache::lucene::search::join::ScoreMode::Avg:
    return total / count;
  case org::apache::lucene::search::join::ScoreMode::Min:
    return minScore;
  case org::apache::lucene::search::join::ScoreMode::Max:
    return maxScore;
  }
  throw invalid_argument(L"Unsupported ScoreMode: " + mode);
}

TestJoinUtil::BitSetCollector::BitSetCollector(shared_ptr<BitSet> bitSet)
    : bitSet(bitSet)
{
}

void TestJoinUtil::BitSetCollector::collect(int doc) 
{
  bitSet->set(docBase + doc);
}

void TestJoinUtil::BitSetCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

bool TestJoinUtil::BitSetCollector::needsScores() { return false; }
} // namespace org::apache::lucene::search::join