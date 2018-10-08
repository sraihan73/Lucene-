using namespace std;

#include "TestDateSort.h"

namespace org::apache::lucene::search
{
using DateTools = org::apache::lucene::document::DateTools;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestDateSort::TEXT_FIELD = L"text";
const wstring TestDateSort::DATE_TIME_FIELD = L"dateTime";

void TestDateSort::setUp() 
{
  LuceneTestCase::setUp();
  // Create an index writer.
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);

  // oldest doc:
  // Add the first document.  text = "Document 1"  dateTime = Oct 10 03:25:22
  // EDT 2007
  writer->addDocument(createDocument(L"Document 1", 1192001122000LL));
  // Add the second document.  text = "Document 2"  dateTime = Oct 10 03:25:26
  // EDT 2007
  writer->addDocument(createDocument(L"Document 2", 1192001126000LL));
  // Add the third document.  text = "Document 3"  dateTime = Oct 11 07:12:13
  // EDT 2007
  writer->addDocument(createDocument(L"Document 3", 1192101133000LL));
  // Add the fourth document.  text = "Document 4"  dateTime = Oct 11 08:02:09
  // EDT 2007
  writer->addDocument(createDocument(L"Document 4", 1192104129000LL));
  // latest doc:
  // Add the fifth document.  text = "Document 5"  dateTime = Oct 12 13:25:43
  // EDT 2007
  writer->addDocument(createDocument(L"Document 5", 1192209943000LL));

  reader = writer->getReader();
  delete writer;
}

void TestDateSort::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestDateSort::testReverseDateSort() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(DATE_TIME_FIELD, SortField::Type::STRING, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"document"));

  // Execute the search and process the search results.
  std::deque<wstring> actualOrder(5);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000, sort)->scoreDocs;
  for (int i = 0; i < hits.size(); i++) {
    shared_ptr<Document> document = searcher->doc(hits[i]->doc);
    wstring text = document[TEXT_FIELD];
    actualOrder[i] = text;
  }

  // Set up the expected order (i.e. Document 5, 4, 3, 2, 1).
  std::deque<wstring> expectedOrder(5);
  expectedOrder[0] = L"Document 5";
  expectedOrder[1] = L"Document 4";
  expectedOrder[2] = L"Document 3";
  expectedOrder[3] = L"Document 2";
  expectedOrder[4] = L"Document 1";

  assertEquals(Arrays::asList(expectedOrder), Arrays::asList(actualOrder));
}

shared_ptr<Document> TestDateSort::createDocument(const wstring &text,
                                                  int64_t time)
{
  shared_ptr<Document> document = make_shared<Document>();

  // Add the text field.
  shared_ptr<Field> textField =
      newTextField(TEXT_FIELD, text, Field::Store::YES);
  document->push_back(textField);

  // Add the date/time field.
  wstring dateTimeString =
      DateTools::timeToString(time, DateTools::Resolution::SECOND);
  shared_ptr<Field> dateTimeField =
      newStringField(DATE_TIME_FIELD, dateTimeString, Field::Store::YES);
  document->push_back(dateTimeField);
  document->push_back(make_shared<SortedDocValuesField>(
      DATE_TIME_FIELD, make_shared<BytesRef>(dateTimeString)));

  return document;
}
} // namespace org::apache::lucene::search