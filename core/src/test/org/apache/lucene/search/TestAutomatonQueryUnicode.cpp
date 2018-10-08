using namespace std;

#include "TestAutomatonQueryUnicode.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
const wstring TestAutomatonQueryUnicode::FN = L"field";

void TestAutomatonQueryUnicode::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> titleField =
      newTextField(L"title", L"some title", Field::Store::NO);
  shared_ptr<Field> field = newTextField(FN, L"", Field::Store::NO);
  shared_ptr<Field> footerField =
      newTextField(L"footer", L"a footer", Field::Store::NO);
  doc->push_back(titleField);
  doc->push_back(field);
  doc->push_back(footerField);
  field->setStringValue(L"\uD866\uDF05abcdef");
  writer->addDocument(doc);
  field->setStringValue(L"\uD866\uDF06ghijkl");
  writer->addDocument(doc);
  // this sorts before the previous two in UTF-8/UTF-32, but after in UTF-16!!!
  field->setStringValue(L"\uFB94mnopqr");
  writer->addDocument(doc);
  field->setStringValue(L"\uFB95stuvwx"); // this one too.
  writer->addDocument(doc);
  field->setStringValue(L"a\uFFFCbc");
  writer->addDocument(doc);
  field->setStringValue(L"a\uFFFDbc");
  writer->addDocument(doc);
  field->setStringValue(L"a\uFFFEbc");
  writer->addDocument(doc);
  field->setStringValue(L"a\uFB94bc");
  writer->addDocument(doc);
  field->setStringValue(L"bacadaba");
  writer->addDocument(doc);
  field->setStringValue(L"\uFFFD");
  writer->addDocument(doc);
  field->setStringValue(L"\uFFFD\uD866\uDF05");
  writer->addDocument(doc);
  field->setStringValue(L"\uFFFD\uFFFD");
  writer->addDocument(doc);
  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

void TestAutomatonQueryUnicode::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

shared_ptr<Term> TestAutomatonQueryUnicode::newTerm(const wstring &value)
{
  return make_shared<Term>(FN, value);
}

int64_t TestAutomatonQueryUnicode::automatonQueryNrHits(
    shared_ptr<AutomatonQuery> query) 
{
  return searcher->search(query, 5)->totalHits;
}

void TestAutomatonQueryUnicode::assertAutomatonHits(
    int expected, shared_ptr<Automaton> automaton) 
{
  shared_ptr<AutomatonQuery> query =
      make_shared<AutomatonQuery>(newTerm(L"bogus"), automaton);

  query->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  assertEquals(expected, automatonQueryNrHits(query));

  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  assertEquals(expected, automatonQueryNrHits(query));

  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE);
  assertEquals(expected, automatonQueryNrHits(query));
}

void TestAutomatonQueryUnicode::testSortOrder() 
{
  shared_ptr<Automaton> a =
      (make_shared<RegExp>(L"((\uD866\uDF05)|\uFB94).*"))->toAutomaton();
  assertAutomatonHits(2, a);
}
} // namespace org::apache::lucene::search