using namespace std;

#include "TestBlockJoinValidation.h"

namespace org::apache::lucene::search::join
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TermQuery = org::apache::lucene::search::TermQuery;
using Weight = org::apache::lucene::search::Weight;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBlockJoinValidation::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<IndexWriterConfig> *const config =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const indexWriter =
      make_shared<IndexWriter>(directory, config);
  for (int i = 0; i < AMOUNT_OF_SEGMENTS; i++) {
    deque<std::shared_ptr<Document>> segmentDocs = createDocsForSegment(i);
    indexWriter->addDocuments(segmentDocs);
    indexWriter->commit();
  }
  indexReader = DirectoryReader::open(indexWriter);
  delete indexWriter;
  indexSearcher = make_shared<IndexSearcher>(indexReader);
  parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<WildcardQuery>(make_shared<Term>(L"parent", L"*")));
}

void TestBlockJoinValidation::tearDown() 
{
  delete indexReader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestBlockJoinValidation::testNextDocValidationForToParentBjq() throw(
    runtime_error)
{
  shared_ptr<Query> parentQueryWithRandomChild =
      createChildrenQueryWithOneParent(getRandomChildNumber(0));
  shared_ptr<ToParentBlockJoinQuery> blockJoinQuery =
      make_shared<ToParentBlockJoinQuery>(parentQueryWithRandomChild,
                                          parentsFilter, ScoreMode::None);
  shared_ptr<IllegalStateException> expected =
      expectThrows(IllegalStateException::typeid,
                   [&]() { indexSearcher->search(blockJoinQuery, 1); });
  assertTrue(expected->getMessage() != nullptr &&
             expected->getMessage()->contains(
                 L"Child query must not match same docs with parent filter"));
}

void TestBlockJoinValidation::testNextDocValidationForToChildBjq() throw(
    runtime_error)
{
  shared_ptr<Query> parentQueryWithRandomChild =
      createParentsQueryWithOneChild(getRandomChildNumber(0));

  shared_ptr<ToChildBlockJoinQuery> blockJoinQuery =
      make_shared<ToChildBlockJoinQuery>(parentQueryWithRandomChild,
                                         parentsFilter);

  shared_ptr<IllegalStateException> expected =
      expectThrows(IllegalStateException::typeid,
                   [&]() { indexSearcher->search(blockJoinQuery, 1); });
  assertTrue(expected->getMessage() != nullptr &&
             expected->getMessage()->contains(
                 ToChildBlockJoinQuery::INVALID_QUERY_MESSAGE));
}

void TestBlockJoinValidation::testAdvanceValidationForToChildBjq() throw(
    runtime_error)
{
  shared_ptr<Query> parentQuery = make_shared<MatchAllDocsQuery>();
  shared_ptr<ToChildBlockJoinQuery> blockJoinQuery =
      make_shared<ToChildBlockJoinQuery>(parentQuery, parentsFilter);

  shared_ptr<LeafReaderContext> *const context =
      indexSearcher->getIndexReader()->leaves()[0];
  shared_ptr<Weight> weight = indexSearcher->createWeight(
      indexSearcher->rewrite(blockJoinQuery), true, 1);
  shared_ptr<Scorer> scorer = weight->scorer(context);
  shared_ptr<Bits> *const parentDocs = parentsFilter->getBitSet(context);

  int target;
  do {
    // make the parent scorer advance to a doc ID which is not a parent
    target = TestUtil::nextInt(random(), 0, context->reader()->maxDoc() - 2);
  } while (parentDocs->get(target + 1));

  constexpr int illegalTarget = target;
  shared_ptr<IllegalStateException> expected =
      expectThrows(IllegalStateException::typeid,
                   [&]() { scorer->begin().advance(illegalTarget); });
  assertTrue(expected->getMessage() != nullptr &&
             expected->getMessage()->contains(
                 ToChildBlockJoinQuery::INVALID_QUERY_MESSAGE));
}

deque<std::shared_ptr<Document>>
TestBlockJoinValidation::createDocsForSegment(int segmentNumber)
{
  deque<deque<std::shared_ptr<Document>>> blocks =
      deque<deque<std::shared_ptr<Document>>>(AMOUNT_OF_PARENT_DOCS);
  for (int i = 0; i < AMOUNT_OF_PARENT_DOCS; i++) {
    blocks.push_back(createParentDocWithChildren(segmentNumber, i));
  }
  deque<std::shared_ptr<Document>> result =
      deque<std::shared_ptr<Document>>(AMOUNT_OF_DOCS_IN_SEGMENT);
  for (auto block : blocks) {
    result.insert(result.end(), block.begin(), block.end());
  }
  return result;
}

deque<std::shared_ptr<Document>>
TestBlockJoinValidation::createParentDocWithChildren(int segmentNumber,
                                                     int parentNumber)
{
  deque<std::shared_ptr<Document>> result =
      deque<std::shared_ptr<Document>>(AMOUNT_OF_CHILD_DOCS + 1);
  for (int i = 0; i < AMOUNT_OF_CHILD_DOCS; i++) {
    result.push_back(createChildDoc(segmentNumber, parentNumber, i));
  }
  result.push_back(createParentDoc(segmentNumber, parentNumber));
  return result;
}

shared_ptr<Document> TestBlockJoinValidation::createParentDoc(int segmentNumber,
                                                              int parentNumber)
{
  shared_ptr<Document> result = make_shared<Document>();
  result->push_back(newStringField(
      L"id",
      createFieldValue({segmentNumber * AMOUNT_OF_PARENT_DOCS + parentNumber}),
      Field::Store::YES));
  result->push_back(newStringField(L"parent", createFieldValue({parentNumber}),
                                   Field::Store::NO));
  result->push_back(newStringField(L"common_field", L"1", Field::Store::NO));
  return result;
}

shared_ptr<Document> TestBlockJoinValidation::createChildDoc(int segmentNumber,
                                                             int parentNumber,
                                                             int childNumber)
{
  shared_ptr<Document> result = make_shared<Document>();
  result->push_back(newStringField(
      L"id",
      createFieldValue(
          {segmentNumber * AMOUNT_OF_PARENT_DOCS + parentNumber, childNumber}),
      Field::Store::YES));
  result->push_back(newStringField(L"child", createFieldValue({childNumber}),
                                   Field::Store::NO));
  result->push_back(newStringField(L"common_field", L"1", Field::Store::NO));
  return result;
}

wstring TestBlockJoinValidation::createFieldValue(deque<int> &documentNumbers)
{
  shared_ptr<StringBuilder> stringBuilder = make_shared<StringBuilder>();
  for (int documentNumber : documentNumbers) {
    if (stringBuilder->length() > 0) {
      stringBuilder->append(L"_");
    }
    stringBuilder->append(documentNumber);
  }
  return stringBuilder->toString();
}

shared_ptr<Query>
TestBlockJoinValidation::createChildrenQueryWithOneParent(int childNumber)
{
  shared_ptr<TermQuery> childQuery = make_shared<TermQuery>(
      make_shared<Term>(L"child", createFieldValue({childNumber})));
  shared_ptr<Query> randomParentQuery = make_shared<TermQuery>(
      make_shared<Term>(L"id", createFieldValue({getRandomParentId()})));
  shared_ptr<BooleanQuery::Builder> childrenQueryWithRandomParent =
      make_shared<BooleanQuery::Builder>();
  childrenQueryWithRandomParent->add(
      make_shared<BooleanClause>(childQuery, BooleanClause::Occur::SHOULD));
  childrenQueryWithRandomParent->add(make_shared<BooleanClause>(
      randomParentQuery, BooleanClause::Occur::SHOULD));
  return childrenQueryWithRandomParent->build();
}

shared_ptr<Query>
TestBlockJoinValidation::createParentsQueryWithOneChild(int randomChildNumber)
{
  shared_ptr<BooleanQuery::Builder> childQueryWithRandomParent =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<Query> parentsQuery = make_shared<TermQuery>(make_shared<Term>(
      L"parent", createFieldValue({getRandomParentNumber()})));
  childQueryWithRandomParent->add(
      make_shared<BooleanClause>(parentsQuery, BooleanClause::Occur::SHOULD));
  childQueryWithRandomParent->add(make_shared<BooleanClause>(
      randomChildQuery(randomChildNumber), BooleanClause::Occur::SHOULD));
  return childQueryWithRandomParent->build();
}

int TestBlockJoinValidation::getRandomParentId()
{
  return random()->nextInt(AMOUNT_OF_PARENT_DOCS * AMOUNT_OF_SEGMENTS);
}

int TestBlockJoinValidation::getRandomParentNumber()
{
  return random()->nextInt(AMOUNT_OF_PARENT_DOCS);
}

shared_ptr<Query>
TestBlockJoinValidation::randomChildQuery(int randomChildNumber)
{
  return make_shared<TermQuery>(make_shared<Term>(
      L"id", createFieldValue({getRandomParentId(), randomChildNumber})));
}

int TestBlockJoinValidation::getRandomChildNumber(int notLessThan)
{
  return notLessThan + random()->nextInt(AMOUNT_OF_CHILD_DOCS - notLessThan);
}
} // namespace org::apache::lucene::search::join