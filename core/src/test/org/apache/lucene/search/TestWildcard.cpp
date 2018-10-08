using namespace std;

#include "TestWildcard.h"

namespace org::apache::lucene::search
{
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;

void TestWildcard::testEquals()
{
  shared_ptr<WildcardQuery> wq1 =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"b*a"));
  shared_ptr<WildcardQuery> wq2 =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"b*a"));
  shared_ptr<WildcardQuery> wq3 =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"b*a"));

  // reflexive?
  assertEquals(wq1, wq2);
  assertEquals(wq2, wq1);

  // transitive?
  assertEquals(wq2, wq3);
  assertEquals(wq1, wq3);

  assertFalse(wq1->equals(nullptr));

  shared_ptr<FuzzyQuery> fq =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"b*a"));
  assertFalse(wq1->equals(fq));
  assertFalse(fq->equals(wq1));
}

void TestWildcard::testTermWithoutWildcard() 
{
  shared_ptr<Directory> indexStore = getIndexStore(
      L"field", std::deque<wstring>{L"nowildcard", L"nowildcardx"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(indexStore);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<MultiTermQuery> wq =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"nowildcard"));
  assertMatches(searcher, wq, 1);

  wq->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  shared_ptr<Query> q = searcher->rewrite(wq);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(q) != nullptr);

  wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  q = searcher->rewrite(wq);
  assertTrue(std::dynamic_pointer_cast<MultiTermQueryConstantScoreWrapper>(q) !=
             nullptr);

  wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE);
  q = searcher->rewrite(wq);
  assertTrue(std::dynamic_pointer_cast<ConstantScoreQuery>(q) != nullptr);
  delete reader;
  delete indexStore;
}

void TestWildcard::testEmptyTerm() 
{
  shared_ptr<Directory> indexStore = getIndexStore(
      L"field", std::deque<wstring>{L"nowildcard", L"nowildcardx"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(indexStore);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<MultiTermQuery> wq =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L""));
  wq->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  assertMatches(searcher, wq, 0);
  shared_ptr<Query> q = searcher->rewrite(wq);
  assertTrue(std::dynamic_pointer_cast<MatchNoDocsQuery>(q) != nullptr);
  delete reader;
  delete indexStore;
}

void TestWildcard::testPrefixTerm() 
{
  shared_ptr<Directory> indexStore =
      getIndexStore(L"field", std::deque<wstring>{L"prefix", L"prefixx"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(indexStore);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<MultiTermQuery> wq =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"prefix*"));
  assertMatches(searcher, wq, 2);

  wq = make_shared<WildcardQuery>(make_shared<Term>(L"field", L"*"));
  assertMatches(searcher, wq, 2);
  shared_ptr<Terms> terms =
      MultiFields::getTerms(searcher->getIndexReader(), L"field");
  assertFalse(wq->getTermsEnum(terms)->getClass().getSimpleName()->contains(
      L"AutomatonTermsEnum"));
  delete reader;
  delete indexStore;
}

void TestWildcard::testAsterisk() 
{
  shared_ptr<Directory> indexStore =
      getIndexStore(L"body", std::deque<wstring>{L"metal", L"metals"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(indexStore);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<Query> query1 =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"metal"));
  shared_ptr<Query> query2 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"metal*"));
  shared_ptr<Query> query3 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"m*tal"));
  shared_ptr<Query> query4 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"m*tal*"));
  shared_ptr<Query> query5 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"m*tals"));

  shared_ptr<BooleanQuery::Builder> query6 =
      make_shared<BooleanQuery::Builder>();
  query6->add(query5, BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> query7 =
      make_shared<BooleanQuery::Builder>();
  query7->add(query3, BooleanClause::Occur::SHOULD);
  query7->add(query5, BooleanClause::Occur::SHOULD);

  // Queries do not automatically lower-case search terms:
  shared_ptr<Query> query8 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"M*tal*"));

  assertMatches(searcher, query1, 1);
  assertMatches(searcher, query2, 2);
  assertMatches(searcher, query3, 1);
  assertMatches(searcher, query4, 2);
  assertMatches(searcher, query5, 1);
  assertMatches(searcher, query6->build(), 1);
  assertMatches(searcher, query7->build(), 2);
  assertMatches(searcher, query8, 0);
  assertMatches(
      searcher,
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"*tall")), 0);
  assertMatches(searcher,
                make_shared<WildcardQuery>(make_shared<Term>(L"body", L"*tal")),
                1);
  assertMatches(
      searcher,
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"*tal*")), 2);
  delete reader;
  delete indexStore;
}

void TestWildcard::testQuestionmark() 
{
  shared_ptr<Directory> indexStore = getIndexStore(
      L"body", std::deque<wstring>{L"metal", L"metals", L"mXtals", L"mXtXls"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(indexStore);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<Query> query1 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"m?tal"));
  shared_ptr<Query> query2 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"metal?"));
  shared_ptr<Query> query3 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"metals?"));
  shared_ptr<Query> query4 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"m?t?ls"));
  shared_ptr<Query> query5 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"M?t?ls"));
  shared_ptr<Query> query6 =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"meta??"));

  assertMatches(searcher, query1, 1);
  assertMatches(searcher, query2, 1);
  assertMatches(searcher, query3, 0);
  assertMatches(searcher, query4, 3);
  assertMatches(searcher, query5, 0);
  assertMatches(searcher, query6,
                1); // Query: 'meta??' matches 'metals' not 'metal'
  delete reader;
  delete indexStore;
}

void TestWildcard::testEscapes() 
{
  shared_ptr<Directory> indexStore = getIndexStore(
      L"field", std::deque<wstring>{L"foo*bar", L"foo??bar", L"fooCDbar",
                                     L"fooSOMETHINGbar", L"foo\\"});
  shared_ptr<IndexReader> reader = DirectoryReader::open(indexStore);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // without escape: matches foo??bar, fooCDbar, foo*bar, and fooSOMETHINGbar
  shared_ptr<WildcardQuery> unescaped =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo*bar"));
  assertMatches(searcher, unescaped, 4);

  // with escape: only matches foo*bar
  shared_ptr<WildcardQuery> escaped =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo\\*bar"));
  assertMatches(searcher, escaped, 1);

  // without escape: matches foo??bar and fooCDbar
  unescaped =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo??bar"));
  assertMatches(searcher, unescaped, 2);

  // with escape: matches foo??bar only
  escaped =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo\\?\\?bar"));
  assertMatches(searcher, escaped, 1);

  // check escaping at end: lenient parse yields "foo\"
  shared_ptr<WildcardQuery> atEnd =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo\\"));
  assertMatches(searcher, atEnd, 1);

  delete reader;
  delete indexStore;
}

shared_ptr<Directory>
TestWildcard::getIndexStore(const wstring &field,
                            std::deque<wstring> &contents) 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  for (int i = 0; i < contents.size(); ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(field, contents[i], Field::Store::YES));
    writer->addDocument(doc);
  }
  delete writer;

  return indexStore;
}

void TestWildcard::assertMatches(shared_ptr<IndexSearcher> searcher,
                                 shared_ptr<Query> q,
                                 int expectedMatches) 
{
  std::deque<std::shared_ptr<ScoreDoc>> result =
      searcher->search(q, 1000)->scoreDocs;
  assertEquals(expectedMatches, result.size());
}

void TestWildcard::testParsingAndSearching() 
{
  wstring field = L"content";
  std::deque<wstring> docs = {L"\\ abcdefg1", L"\\79 hijklmn1",
                               L"\\\\ opqrstu1"};

  // queries that should find all docs
  std::deque<std::shared_ptr<Query>> matchAll = {
      make_shared<WildcardQuery>(make_shared<Term>(field, L"*")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"*1")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"**1")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"*?")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"*?1")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"?*1")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"**")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"***")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"\\\\*"))};

  // queries that should find no docs
  std::deque<std::shared_ptr<Query>> matchNone = {
      make_shared<WildcardQuery>(make_shared<Term>(field, L"a*h")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"a?h")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"*a*h")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"?a")),
      make_shared<WildcardQuery>(make_shared<Term>(field, L"a?"))};

  std::deque<std::deque<std::shared_ptr<PrefixQuery>>> matchOneDocPrefix = {
      std::deque<std::shared_ptr<PrefixQuery>>{
          make_shared<PrefixQuery>(make_shared<Term>(field, L"a")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"ab")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"abc"))},
      std::deque<std::shared_ptr<PrefixQuery>>{
          make_shared<PrefixQuery>(make_shared<Term>(field, L"h")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"hi")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"hij")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"\\7"))},
      std::deque<std::shared_ptr<PrefixQuery>>{
          make_shared<PrefixQuery>(make_shared<Term>(field, L"o")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"op")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"opq")),
          make_shared<PrefixQuery>(make_shared<Term>(field, L"\\\\"))}};

  std::deque<std::deque<std::shared_ptr<WildcardQuery>>> matchOneDocWild = {
      std::deque<std::shared_ptr<WildcardQuery>>{
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*a*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*ab*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*abc**")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"ab*e*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*g?")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*f?1"))},
      std::deque<std::shared_ptr<WildcardQuery>>{
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*h*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*hi*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*hij**")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"hi*k*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*n?")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*m?1")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"hij**"))},
      std::deque<std::shared_ptr<WildcardQuery>>{
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*o*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*op*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*opq**")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"op*q*")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*u?")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"*t?1")),
          make_shared<WildcardQuery>(make_shared<Term>(field, L"opq**"))}};

  // prepare the index
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  for (int i = 0; i < docs.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(field, docs[i], Field::Store::NO));
    iw->addDocument(doc);
  }
  delete iw;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // test queries that must find all
  for (auto q : matchAll) {
    if (VERBOSE) {
      wcout << L"matchAll: q=" << q << L" " << q->getClassName() << endl;
    }
    std::deque<std::shared_ptr<ScoreDoc>> hits =
        searcher->search(q, 1000)->scoreDocs;
    assertEquals(docs.size(), hits.size());
  }

  // test queries that must find none
  for (auto q : matchNone) {
    if (VERBOSE) {
      wcout << L"matchNone: q=" << q << L" " << q->getClassName() << endl;
    }
    std::deque<std::shared_ptr<ScoreDoc>> hits =
        searcher->search(q, 1000)->scoreDocs;
    assertEquals(0, hits.size());
  }

  // thest the prefi queries find only one doc
  for (int i = 0; i < matchOneDocPrefix.size(); i++) {
    for (int j = 0; j < matchOneDocPrefix[i].size(); j++) {
      shared_ptr<Query> q = matchOneDocPrefix[i][j];
      if (VERBOSE) {
        wcout << L"match 1 prefix: doc=" << docs[i] << L" q=" << q << L" "
              << q->getClassName() << endl;
      }
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          searcher->search(q, 1000)->scoreDocs;
      assertEquals(1, hits.size());
      assertEquals(i, hits[0]->doc);
    }
  }

  // test the wildcard queries find only one doc
  for (int i = 0; i < matchOneDocWild.size(); i++) {
    for (int j = 0; j < matchOneDocWild[i].size(); j++) {
      shared_ptr<Query> q = matchOneDocWild[i][j];
      if (VERBOSE) {
        wcout << L"match 1 wild: doc=" << docs[i] << L" q=" << q << L" "
              << q->getClassName() << endl;
      }
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          searcher->search(q, 1000)->scoreDocs;
      assertEquals(1, hits.size());
      assertEquals(i, hits[0]->doc);
    }
  }

  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::search