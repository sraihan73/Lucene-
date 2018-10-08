using namespace std;

#include "TestComplexPhraseQuery.h"

namespace org::apache::lucene::queryparser::complexPhrase
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockSynonymAnalyzer = org::apache::lucene::analysis::MockSynonymAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestComplexPhraseQuery::testComplexPhrases() 
{
  checkMatches(L"\"john smith\"", L"1");    // Simple multi-term still works
  checkMatches(L"\"j*   smyth~\"", L"1,2"); // wildcards and fuzzies are OK in
  // phrases
  checkMatches(L"\"(jo* -john)  smith\"", L"2");  // bool logic works
  checkMatches(L"\"jo*  smith\"~2", L"1,2,3");    // position logic works.
  checkMatches(L"\"jo* [sma TO smZ]\" ", L"1,2"); // range queries supported
  checkMatches(L"\"john\"", L"1,3"); // Simple single-term still works
  checkMatches(L"\"(john OR johathon)  smith\"", L"1,2"); // bool logic with
  // brackets works.
  checkMatches(L"\"(john OR nosuchword*)  smith\"", L"1"); // bool logic with
  // brackets works when one of the terms in BooleanQuery does not exist
  // (LUCENE-8305).
  checkMatches(L"\"(jo* -john) smyth~\"", L"2"); // bool logic with
  // brackets works.

  // checkMatches("\"john -percival\"", "1"); // not logic doesn't work
  // currently :(.

  checkMatches(L"\"john  nosuchword*\"", L""); // phrases with clauses producing
  // empty sets

  checkBadQuery(L"\"jo*  id:1 smith\""); // mixing fields in a phrase is bad
  checkBadQuery(L"\"jo* \"smith\" \"");  // phrases inside phrases is bad
}

void TestComplexPhraseQuery::testSingleTermPhrase() 
{
  checkMatches(L"\"joh*\"", L"1,2,3,5");
  checkMatches(L"\"joh~\"", L"1,3,5");
  checkMatches(L"\"joh*\" \"tom\"", L"1,2,3,4,5");
  checkMatches(L"+\"j*\" +\"tom\"", L"4");
  checkMatches(L"\"jo*\" \"[sma TO smZ]\" ", L"1,2,3,5,8");
  checkMatches(L"+\"j*hn\" +\"sm*h\"", L"1,3");
}

void TestComplexPhraseQuery::testSynonyms() 
{
  checkMatches(L"\"dogs\"", L"8");
  shared_ptr<MockSynonymAnalyzer> synonym = make_shared<MockSynonymAnalyzer>();
  checkMatches(L"\"dogs\"", L"7,8", synonym);
  // synonym is unidirectional
  checkMatches(L"\"dog\"", L"7", synonym);
  checkMatches(L"\"dogs cigar*\"", L"");
  checkMatches(L"\"dog cigar*\"", L"7");
  checkMatches(L"\"dogs cigar*\"", L"7", synonym);
  checkMatches(L"\"dog cigar*\"", L"7", synonym);
  checkMatches(L"\"dogs cigar*\"~2", L"7,8", synonym);
  // synonym is unidirectional
  checkMatches(L"\"dog cigar*\"~2", L"7", synonym);
}

void TestComplexPhraseQuery::testUnOrderedProximitySearches() throw(
    runtime_error)
{

  inOrder = true;
  checkMatches(L"\"smith jo*\"~2", L""); // ordered proximity produces empty set

  inOrder = false;
  checkMatches(L"\"smith jo*\"~2", L"1,2,3"); // un-ordered proximity
}

void TestComplexPhraseQuery::checkBadQuery(const wstring &qString)
{
  shared_ptr<ComplexPhraseQueryParser> qp =
      make_shared<ComplexPhraseQueryParser>(defaultFieldName, analyzer);
  qp->setInOrder(inOrder);
  expectThrows(runtime_error::typeid, [&]() { qp->parse(qString); });
}

void TestComplexPhraseQuery::checkMatches(
    const wstring &qString, const wstring &expectedVals) 
{
  checkMatches(qString, expectedVals, analyzer);
}

void TestComplexPhraseQuery::checkMatches(
    const wstring &qString, const wstring &expectedVals,
    shared_ptr<Analyzer> anAnalyzer) 
{
  shared_ptr<ComplexPhraseQueryParser> qp =
      make_shared<ComplexPhraseQueryParser>(defaultFieldName, anAnalyzer);
  qp->setInOrder(inOrder);
  qp->setFuzzyPrefixLength(1); // usually a good idea

  shared_ptr<Query> q = qp->parse(qString);

  unordered_set<wstring> expecteds = unordered_set<wstring>();
  std::deque<wstring> vals = expectedVals.split(L",");
  for (int i = 0; i < vals.size(); i++) {
    if (vals[i].length() > 0) {
      expecteds.insert(vals[i]);
    }
  }

  shared_ptr<TopDocs> td = searcher->search(q, 10);
  std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
  for (int i = 0; i < sd.size(); i++) {
    shared_ptr<Document> doc = searcher->doc(sd[i]->doc);
    wstring id = doc[L"id"];
    assertTrue(qString + L"matched doc#" + id + L" not expected",
               find(expecteds.begin(), expecteds.end(), id) != expecteds.end());
    expecteds.remove(id);
  }

  assertEquals(qString + L" missing some matches ", 0, expecteds.size());
}

void TestComplexPhraseQuery::testFieldedQuery() 
{
  checkMatches(L"name:\"john smith\"", L"1");
  checkMatches(L"name:\"j*   smyth~\"", L"1,2");
  checkMatches(L"role:\"developer\"", L"1,2");
  checkMatches(L"role:\"p* manager\"", L"4");
  checkMatches(L"role:de*", L"1,2,3");
  checkMatches(L"name:\"j* smyth~\"~5", L"1,2,3");
  checkMatches(L"role:\"p* manager\" AND name:jack*", L"4");
  checkMatches(L"+role:developer +name:jack*", L"");
  checkMatches(L"name:\"john smith\"~2 AND role:designer AND id:3", L"3");
}

void TestComplexPhraseQuery::testToStringContainsSlop() 
{
  shared_ptr<ComplexPhraseQueryParser> qp =
      make_shared<ComplexPhraseQueryParser>(defaultFieldName, analyzer);
  int slop = random()->nextInt(31) + 1;

  wstring qString = L"name:\"j* smyth~\"~" + to_wstring(slop);
  shared_ptr<Query> query = qp->parse(qString);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(
      L"Slop is not shown in toString()",
      StringHelper::endsWith(query->toString(), L"~" + to_wstring(slop)));

  wstring string = L"\"j* smyth~\"";
  shared_ptr<Query> q = qp->parse(string);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Don't show implicit slop of zero", q->toString(), string);
}

void TestComplexPhraseQuery::testHashcodeEquals() 
{
  shared_ptr<ComplexPhraseQueryParser> qp =
      make_shared<ComplexPhraseQueryParser>(defaultFieldName, analyzer);
  qp->setInOrder(true);
  qp->setFuzzyPrefixLength(1);

  wstring qString = L"\"aaa* bbb*\"";

  shared_ptr<Query> q = qp->parse(qString);
  shared_ptr<Query> q2 = qp->parse(qString);

  assertEquals(q->hashCode(), q2->hashCode());
  assertEquals(q, q2);

  qp->setInOrder(false); // SOLR-6011

  q2 = qp->parse(qString);

  // although the general contract of hashCode can't guarantee different values,
  // if we only change one thing about a single query, it normally should result
  // in a different value (and will with the current implementation in
  // ComplexPhraseQuery)
  assertTrue(q->hashCode() != q2->hashCode());
  assertTrue(!q->equals(q2));
  assertTrue(!q2->equals(q));
}

void TestComplexPhraseQuery::setUp() 
{
  LuceneTestCase::setUp();

  analyzer = make_shared<MockAnalyzer>(random());
  rd = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(rd, newIndexWriterConfig(analyzer));
  for (int i = 0; i < docsContent.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"name", docsContent[i]->name, Field::Store::YES));
    doc->push_back(newTextField(L"id", docsContent[i]->id, Field::Store::YES));
    doc->push_back(
        newTextField(L"role", docsContent[i]->role, Field::Store::YES));
    w->addDocument(doc);
  }
  delete w;
  reader = DirectoryReader::open(rd);
  searcher = newSearcher(reader);
}

void TestComplexPhraseQuery::tearDown() 
{
  delete reader;
  delete rd;
  LuceneTestCase::tearDown();
}

// C++ TODO: No base class can be determined:
TestComplexPhraseQuery::DocData::DocData(const wstring &name, const wstring &id,
                                         const wstring &role)
{
  this->name = name;
  this->id = id;
  this->role = role;
}
} // namespace org::apache::lucene::queryparser::complexPhrase