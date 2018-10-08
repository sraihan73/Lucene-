using namespace std;

#include "TestPrefixInBooleanQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
const wstring TestPrefixInBooleanQuery::FIELD = L"name";
shared_ptr<org::apache::lucene::store::Directory>
    TestPrefixInBooleanQuery::directory;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestPrefixInBooleanQuery::reader;
shared_ptr<IndexSearcher> TestPrefixInBooleanQuery::searcher;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestPrefixInBooleanQuery::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      newStringField(FIELD, L"meaninglessnames", Field::Store::NO);
  doc->push_back(field);

  for (int i = 0; i < 5137; ++i) {
    writer->addDocument(doc);
  }

  field->setStringValue(L"tangfulin");
  writer->addDocument(doc);

  field->setStringValue(L"meaninglessnames");
  for (int i = 5138; i < 11377; ++i) {
    writer->addDocument(doc);
  }

  field->setStringValue(L"tangfulin");
  writer->addDocument(doc);

  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestPrefixInBooleanQuery::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
}

void TestPrefixInBooleanQuery::testPrefixQuery() 
{
  shared_ptr<Query> query =
      make_shared<PrefixQuery>(make_shared<Term>(FIELD, L"tang"));
  assertEquals(L"Number of matched documents", 2,
               searcher->search(query, 1000)->totalHits);
}

void TestPrefixInBooleanQuery::testTermQuery() 
{
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"tangfulin"));
  assertEquals(L"Number of matched documents", 2,
               searcher->search(query, 1000)->totalHits);
}

void TestPrefixInBooleanQuery::testTermBooleanQuery() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"tangfulin")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"notexistnames")),
             BooleanClause::Occur::SHOULD);
  assertEquals(L"Number of matched documents", 2,
               searcher->search(query->build(), 1000)->totalHits);
}

void TestPrefixInBooleanQuery::testPrefixBooleanQuery() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<PrefixQuery>(make_shared<Term>(FIELD, L"tang")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"notexistnames")),
             BooleanClause::Occur::SHOULD);
  assertEquals(L"Number of matched documents", 2,
               searcher->search(query->build(), 1000)->totalHits);
}
} // namespace org::apache::lucene::search