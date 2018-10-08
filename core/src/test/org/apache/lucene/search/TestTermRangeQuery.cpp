using namespace std;

#include "TestTermRangeQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestTermRangeQuery::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
}

void TestTermRangeQuery::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}

void TestTermRangeQuery::testExclusive() 
{
  shared_ptr<Query> query =
      TermRangeQuery::newStringRange(L"content", L"A", L"C", false, false);
  initializeIndex(std::deque<wstring>{L"A", L"B", L"C", L"D"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"A,B,C,D, only B in range", 1, hits.size());
  delete reader;

  initializeIndex(std::deque<wstring>{L"A", L"B", L"D"});
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"A,B,D, only B in range", 1, hits.size());
  delete reader;

  addDoc(L"C");
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"C added, still only B in range", 1, hits.size());
  delete reader;
}

void TestTermRangeQuery::testInclusive() 
{
  shared_ptr<Query> query =
      TermRangeQuery::newStringRange(L"content", L"A", L"C", true, true);

  initializeIndex(std::deque<wstring>{L"A", L"B", L"C", L"D"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"A,B,C,D - A,B,C in range", 3, hits.size());
  delete reader;

  initializeIndex(std::deque<wstring>{L"A", L"B", L"D"});
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"A,B,D - A and B in range", 2, hits.size());
  delete reader;

  addDoc(L"C");
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"C added - A, B, C in range", 3, hits.size());
  delete reader;
}

void TestTermRangeQuery::testAllDocs() 
{
  initializeIndex(std::deque<wstring>{L"A", L"B", L"C", L"D"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<TermRangeQuery> query =
      make_shared<TermRangeQuery>(L"content", nullptr, nullptr, true, true);
  assertEquals(4, searcher->search(query, 1000)->scoreDocs.size());

  query = TermRangeQuery::newStringRange(L"content", L"", L"", true, true);
  assertEquals(4, searcher->search(query, 1000)->scoreDocs.size());

  query = TermRangeQuery::newStringRange(L"content", L"", L"", true, false);
  assertEquals(4, searcher->search(query, 1000)->scoreDocs.size());

  // and now another one
  query = TermRangeQuery::newStringRange(L"content", L"B", L"", true, true);
  assertEquals(3, searcher->search(query, 1000)->scoreDocs.size());
  delete reader;
}

void TestTermRangeQuery::testTopTermsRewrite() 
{
  initializeIndex(std::deque<wstring>{L"A", L"B", L"C", L"D", L"E", L"F", L"G",
                                       L"H", L"I", L"J", L"K"});

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TermRangeQuery> query =
      TermRangeQuery::newStringRange(L"content", L"B", L"J", true, true);
  checkBooleanTerms(searcher, query,
                    {L"B", L"C", L"D", L"E", L"F", L"G", L"H", L"I", L"J"});

  constexpr int savedClauseCount = BooleanQuery::getMaxClauseCount();
  try {
    BooleanQuery::setMaxClauseCount(3);
    checkBooleanTerms(searcher, query, {L"B", L"C", L"D"});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    BooleanQuery::setMaxClauseCount(savedClauseCount);
  }
  delete reader;
}

void TestTermRangeQuery::checkBooleanTerms(
    shared_ptr<IndexSearcher> searcher, shared_ptr<TermRangeQuery> query,
    deque<wstring> &terms) 
{
  query->setRewriteMethod(
      make_shared<MultiTermQuery::TopTermsScoringBooleanQueryRewrite>(50));
  shared_ptr<BooleanQuery> *const bq =
      std::static_pointer_cast<BooleanQuery>(searcher->rewrite(query));
  shared_ptr<Set<wstring>> *const allowedTerms = asSet({terms});
  assertEquals(allowedTerms->size(), bq->clauses().size());
  for (auto c : bq->clauses()) {
    assertTrue(std::dynamic_pointer_cast<TermQuery>(c->getQuery()) != nullptr);
    shared_ptr<TermQuery> *const tq =
        std::static_pointer_cast<TermQuery>(c->getQuery());
    const wstring term = tq->getTerm()->text();
    assertTrue(L"invalid term: " + term, allowedTerms->contains(term));
    allowedTerms->remove(term); // remove to fail on double terms
  }
  assertEquals(0, allowedTerms->size());
}

void TestTermRangeQuery::testEqualsHashcode()
{
  shared_ptr<Query> query =
      TermRangeQuery::newStringRange(L"content", L"A", L"C", true, true);

  shared_ptr<Query> other =
      TermRangeQuery::newStringRange(L"content", L"A", L"C", true, true);

  assertEquals(L"query equals itself is true", query, query);
  assertEquals(L"equivalent queries are equal", query, other);
  assertEquals(L"hashcode must return same value when equals is true",
               query->hashCode(), other->hashCode());

  other = TermRangeQuery::newStringRange(L"notcontent", L"A", L"C", true, true);
  assertFalse(L"Different fields are not equal", query->equals(other));

  other = TermRangeQuery::newStringRange(L"content", L"X", L"C", true, true);
  assertFalse(L"Different lower terms are not equal", query->equals(other));

  other = TermRangeQuery::newStringRange(L"content", L"A", L"Z", true, true);
  assertFalse(L"Different upper terms are not equal", query->equals(other));

  query = TermRangeQuery::newStringRange(L"content", L"", L"C", true, true);
  other = TermRangeQuery::newStringRange(L"content", L"", L"C", true, true);
  assertEquals(L"equivalent queries with null lowerterms are equal()", query,
               other);
  assertEquals(L"hashcode must return same value when equals is true",
               query->hashCode(), other->hashCode());

  query = TermRangeQuery::newStringRange(L"content", L"C", L"", true, true);
  other = TermRangeQuery::newStringRange(L"content", L"C", L"", true, true);
  assertEquals(L"equivalent queries with null upperterms are equal()", query,
               other);
  assertEquals(L"hashcode returns same value", query->hashCode(),
               other->hashCode());

  query = TermRangeQuery::newStringRange(L"content", L"", L"C", true, true);
  other = TermRangeQuery::newStringRange(L"content", L"C", L"", true, true);
  assertFalse(L"queries with different upper and lower terms are not equal",
              query->equals(other));

  query = TermRangeQuery::newStringRange(L"content", L"A", L"C", false, false);
  other = TermRangeQuery::newStringRange(L"content", L"A", L"C", true, true);
  assertFalse(L"queries with different inclusive are not equal",
              query->equals(other));
}

TestTermRangeQuery::SingleCharAnalyzer::SingleCharTokenizer::
    SingleCharTokenizer()
    : org::apache::lucene::analysis::Tokenizer()
{
  termAtt = addAttribute(CharTermAttribute::typeid);
}

bool TestTermRangeQuery::SingleCharAnalyzer::SingleCharTokenizer::
    incrementToken() 
{
  if (done) {
    return false;
  } else {
    int count = input->read(buffer);
    clearAttributes();
    done = true;
    if (count == 1) {
      termAtt->copyBuffer(buffer, 0, 1);
    }
    return true;
  }
}

void TestTermRangeQuery::SingleCharAnalyzer::SingleCharTokenizer::reset() throw(
    IOException)
{
  Tokenizer::reset();
  done = false;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestTermRangeQuery::SingleCharAnalyzer::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<SingleCharTokenizer>());
}

void TestTermRangeQuery::initializeIndex(std::deque<wstring> &values) throw(
    IOException)
{
  initializeIndex(values, make_shared<MockAnalyzer>(
                              random(), MockTokenizer::WHITESPACE, false));
}

void TestTermRangeQuery::initializeIndex(
    std::deque<wstring> &values,
    shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)->setOpenMode(OpenMode::CREATE));
  for (int i = 0; i < values.size(); i++) {
    insertDoc(writer, values[i]);
  }
  delete writer;
}

void TestTermRangeQuery::addDoc(const wstring &content) 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setOpenMode(OpenMode::APPEND));
  insertDoc(writer, content);
  delete writer;
}

void TestTermRangeQuery::insertDoc(shared_ptr<IndexWriter> writer,
                                   const wstring &content) 
{
  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(
      newStringField(L"id", L"id" + to_wstring(docCount), Field::Store::YES));
  doc->push_back(newTextField(L"content", content, Field::Store::NO));

  writer->addDocument(doc);
  docCount++;
}

void TestTermRangeQuery::testExclusiveLowerNull() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SingleCharAnalyzer>();
  // http://issues.apache.org/jira/browse/LUCENE-38
  shared_ptr<Query> query =
      TermRangeQuery::newStringRange(L"content", L"", L"C", false, false);
  initializeIndex(std::deque<wstring>{L"A", L"B", L"", L"C", L"D"}, analyzer);
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  int64_t numHits = searcher->search(query, 1000)->totalHits;
  // When Lucene-38 is fixed, use the assert on the next line:
  assertEquals(L"A,B,<empty string>,C,D => A, B & <empty string> are in range",
               3, numHits);
  // until Lucene-38 is fixed, use this assert:
  // assertEquals("A,B,<empty string>,C,D => A, B & <empty string> are in
  // range", 2, hits.length());

  delete reader;
  initializeIndex(std::deque<wstring>{L"A", L"B", L"", L"D"}, analyzer);
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  numHits = searcher->search(query, 1000)->totalHits;
  // When Lucene-38 is fixed, use the assert on the next line:
  assertEquals(L"A,B,<empty string>,D => A, B & <empty string> are in range", 3,
               numHits);
  // until Lucene-38 is fixed, use this assert:
  // assertEquals("A,B,<empty string>,D => A, B & <empty string> are in range",
  // 2, hits.length());
  delete reader;
  addDoc(L"C");
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  numHits = searcher->search(query, 1000)->totalHits;
  // When Lucene-38 is fixed, use the assert on the next line:
  assertEquals(L"C added, still A, B & <empty string> are in range", 3,
               numHits);
  // until Lucene-38 is fixed, use this assert
  // assertEquals("C added, still A, B & <empty string> are in range", 2,
  // hits.length());
  delete reader;
}

void TestTermRangeQuery::testInclusiveLowerNull() 
{
  // http://issues.apache.org/jira/browse/LUCENE-38
  shared_ptr<Analyzer> analyzer = make_shared<SingleCharAnalyzer>();
  shared_ptr<Query> query =
      TermRangeQuery::newStringRange(L"content", L"", L"C", true, true);
  initializeIndex(std::deque<wstring>{L"A", L"B", L"", L"C", L"D"}, analyzer);
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  int64_t numHits = searcher->search(query, 1000)->totalHits;
  // When Lucene-38 is fixed, use the assert on the next line:
  assertEquals(L"A,B,<empty string>,C,D => A,B,<empty string>,C in range", 4,
               numHits);
  // until Lucene-38 is fixed, use this assert
  // assertEquals("A,B,<empty string>,C,D => A,B,<empty string>,C in range", 3,
  // hits.length());
  delete reader;
  initializeIndex(std::deque<wstring>{L"A", L"B", L"", L"D"}, analyzer);
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  numHits = searcher->search(query, 1000)->totalHits;
  // When Lucene-38 is fixed, use the assert on the next line:
  assertEquals(L"A,B,<empty string>,D - A, B and <empty string> in range", 3,
               numHits);
  // until Lucene-38 is fixed, use this assert
  // assertEquals("A,B,<empty string>,D => A, B and <empty string> in range", 2,
  // hits.length());
  delete reader;
  addDoc(L"C");
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  numHits = searcher->search(query, 1000)->totalHits;
  // When Lucene-38 is fixed, use the assert on the next line:
  assertEquals(L"C added => A,B,<empty string>,C in range", 4, numHits);
  // until Lucene-38 is fixed, use this assert
  // assertEquals("C added => A,B,<empty string>,C in range", 3, hits.length());
  delete reader;
}
} // namespace org::apache::lucene::search