using namespace std;

#include "TestUnifiedHighlighterStrictPhrases.h"

namespace org::apache::lucene::search::uhighlight
{
using com::carrotsearch::randomizedtesting::annotations::ParametersFactory;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Weight = org::apache::lucene::search::Weight;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using QueryBuilder = org::apache::lucene::util::QueryBuilder;
using org::junit::After;
using org::junit::Before;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ParametersFactory public static Iterable<Object[]>
// parameters()
deque<std::deque<any>> TestUnifiedHighlighterStrictPhrases::parameters()
{
  return UHTestHelper::parametersFactoryList();
}

TestUnifiedHighlighterStrictPhrases::TestUnifiedHighlighterStrictPhrases(
    shared_ptr<FieldType> fieldType)
    : fieldType(fieldType)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
void TestUnifiedHighlighterStrictPhrases::doBefore() 
{
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE,
                                true); // whitespace, punctuation, lowercase
  indexAnalyzer->setPositionIncrementGap(3); // more than default
  dir = newDirectory();
  indexWriter = make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
void TestUnifiedHighlighterStrictPhrases::doAfter() 
{
  IOUtils::close({indexReader, indexWriter, dir});
}

shared_ptr<Document>
TestUnifiedHighlighterStrictPhrases::newDoc(deque<wstring> &bodyVals)
{
  shared_ptr<Document> doc = make_shared<Document>();
  for (wstring bodyVal : bodyVals) {
    doc->push_back(make_shared<Field>(L"body", bodyVal, fieldType));
  }
  return doc;
}

void TestUnifiedHighlighterStrictPhrases::initReaderSearcherHighlighter() throw(
    IOException)
{
  indexReader = indexWriter->getReader();
  searcher = newSearcher(indexReader);
  highlighter = make_shared<UnifiedHighlighter>(searcher, indexAnalyzer);
  highlighter->setHighlightPhrasesStrictly(true);
}

shared_ptr<PhraseQuery>
TestUnifiedHighlighterStrictPhrases::newPhraseQuery(const wstring &field,
                                                    const wstring &phrase)
{
  return std::static_pointer_cast<PhraseQuery>(
      (make_shared<QueryBuilder>(indexAnalyzer))
          ->createPhraseQuery(field, phrase));
}

shared_ptr<PhraseQuery>
TestUnifiedHighlighterStrictPhrases::setSlop(shared_ptr<PhraseQuery> query,
                                             int slop)
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  std::deque<std::shared_ptr<Term>> terms = query->getTerms();
  std::deque<int> positions = query->getPositions();
  for (int i = 0; i < terms.size(); i++) {
    builder->add(terms[i], positions[i]);
  }
  builder->setSlop(slop);
  return builder->build();
}

void TestUnifiedHighlighterStrictPhrases::testBasics() 
{
  indexWriter->addDocument(
      newDoc({L"Yin yang, filter"})); // filter out. test getTermToSpanLists
                                      // reader 1-doc filter
  indexWriter->addDocument(newDoc({L"yin alone, Yin yang, yin gap yang"}));
  initReaderSearcherHighlighter();

  // query:  -filter +"yin yang"
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"filter")),
                BooleanClause::Occur::MUST_NOT)
          ->add(newPhraseQuery(L"body", L"yin yang"),
                BooleanClause::Occur::MUST)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{L"yin alone, <b>Yin</b> <b>yang</b>, yin gap yang"},
      snippets);
}

void TestUnifiedHighlighterStrictPhrases::testWithSameTermQuery() throw(
    IOException)
{
  indexWriter->addDocument(newDoc({L"Yin yang, yin gap yang"}));
  initReaderSearcherHighlighter();

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"yin")),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"yin yang"),
                BooleanClause::Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"yang")),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{L"<b>Yin</b> <b>yang</b>, <b>yin</b> gap yang"},
      snippets);

  // test the Passage only has 3 matches.  We don't want duplicates from "Yin"
  // being in TermQuery & PhraseQuery.
  highlighter->setFormatter(
      make_shared<PassageFormatterAnonymousInnerClass>(shared_from_this()));
  assertArrayEquals(
      std::deque<wstring>{
          L"[Passage[0-22]{yin[0-3],yang[4-8],yin[10-13]}score=2.4964213]"},
      highlighter->highlight(L"body", query, topDocs));
}

TestUnifiedHighlighterStrictPhrases::PassageFormatterAnonymousInnerClass::
    PassageFormatterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance)
{
  this->outerInstance = outerInstance;
}

any TestUnifiedHighlighterStrictPhrases::PassageFormatterAnonymousInnerClass::
    format(std::deque<std::shared_ptr<Passage>> &passages,
           const wstring &content)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Arrays->toString(passages);
}

void TestUnifiedHighlighterStrictPhrases::testPhraseNotInDoc() throw(
    IOException)
{
  indexWriter->addDocument(
      newDoc({L"Whatever yin"})); // query matches this; highlight it
  indexWriter->addDocument(
      newDoc({L"nextdoc yin"})); // query does NOT match this, only the SHOULD
                                 // clause does
  initReaderSearcherHighlighter();

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"whatever")),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"nextdoc yin"),
                BooleanClause::Occur::SHOULD)
          ->add(newPhraseQuery(L"body", L"nonexistent yin"),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(std::deque<wstring>{L"<b>Whatever</b> yin"}, snippets);
}

void TestUnifiedHighlighterStrictPhrases::testSubPhrases() 
{
  indexWriter->addDocument(
      newDoc({L"alpha bravo charlie - charlie bravo alpha"}));
  initReaderSearcherHighlighter();

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(newPhraseQuery(L"body", L"alpha bravo charlie"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"alpha bravo"),
                BooleanClause::Occur::MUST)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>alpha</b> <b>bravo</b> <b>charlie</b> - charlie bravo alpha"},
      snippets);

  // test the Passage only has 3 matches.  We don't want duplicates from both
  // PhraseQuery
  highlighter->setFormatter(
      make_shared<PassageFormatterAnonymousInnerClass2>(shared_from_this()));
  assertArrayEquals(std::deque<wstring>{L"[Passage[0-41]{alpha[0-5],bravo[6-"
                                         L"11],charlie[12-19]}score=3.931102]"},
                    highlighter->highlight(L"body", query, topDocs));
}

TestUnifiedHighlighterStrictPhrases::PassageFormatterAnonymousInnerClass2::
    PassageFormatterAnonymousInnerClass2(
        shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance)
{
  this->outerInstance = outerInstance;
}

any TestUnifiedHighlighterStrictPhrases::PassageFormatterAnonymousInnerClass2::
    format(std::deque<std::shared_ptr<Passage>> &passages,
           const wstring &content)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Arrays->toString(passages);
}

void TestUnifiedHighlighterStrictPhrases::testSynonyms() 
{
  indexWriter->addDocument(newDoc({L"mother father w mom father w dad"}));
  initReaderSearcherHighlighter();

  shared_ptr<MultiPhraseQuery> query =
      (make_shared<MultiPhraseQuery::Builder>())
          ->add(std::deque<std::shared_ptr<Term>>{
              make_shared<Term>(L"body", L"mom"),
              make_shared<Term>(L"body", L"mother")})
          ->add(std::deque<std::shared_ptr<Term>>{
              make_shared<Term>(L"body", L"dad"),
              make_shared<Term>(L"body", L"father")})
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>mother</b> <b>father</b> w <b>mom</b> <b>father</b> w dad"},
      snippets);
}

void TestUnifiedHighlighterStrictPhrases::testRewriteAndMtq() 
{
  indexWriter->addDocument(
      newDoc({L"alpha bravo charlie - charlie bravo alpha"}));
  initReaderSearcherHighlighter();

  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(L"body", L"bravo")),
          make_shared<SpanMultiTermQueryWrapper<>>(
              make_shared<PrefixQuery>(make_shared<Term>(L"body", L"ch")))},
      0, true);

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(snq, BooleanClause::Occur::MUST)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"body", L"al")),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"alpha bravo"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"title", L"bravo alpha"),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{L"<b>alpha</b> <b>bravo</b> <b>charlie</b> - "
                           L"charlie bravo <b>alpha</b>"},
      snippets);

  // do again, this time with MTQ disabled.  We should only find "alpha bravo".
  highlighter->setHandleMultiTermQuery(
      false); // disable but leave phrase processing enabled

  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  snippets = highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>alpha</b> <b>bravo</b> charlie - charlie bravo alpha"},
      snippets);
}

void TestUnifiedHighlighterStrictPhrases::testRewrite() 
{
  indexWriter->addDocument(
      newDoc({L"alpha bravo charlie - charlie bravo alpha"}));
  initReaderSearcherHighlighter();

  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(L"body", L"bravo")),
          make_shared<SpanMultiTermQueryWrapper<>>(
              make_shared<PrefixQuery>(make_shared<Term>(L"body", L"ch")))},
      0, true);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(snq, BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"alpha bravo"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"title", L"bravo alpha"),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>alpha</b> <b>bravo</b> <b>charlie</b> - charlie bravo alpha"},
      snippets);

  // do again, this time with MTQ disabled.  We should only find "alpha bravo".
  highlighter->setHandleMultiTermQuery(
      false); // disable but leave phrase processing enabled

  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  snippets = highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>alpha</b> <b>bravo</b> charlie - charlie bravo alpha"},
      snippets);
}

void TestUnifiedHighlighterStrictPhrases::testMtq() 
{
  indexWriter->addDocument(
      newDoc({L"alpha bravo charlie - charlie bravo alpha"}));
  initReaderSearcherHighlighter();

  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(L"body", L"bravo")),
          make_shared<SpanTermQuery>(make_shared<Term>(L"body", L"charlie"))},
      0, true);

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(snq, BooleanClause::Occur::MUST)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"body", L"al")),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"alpha bravo"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"title", L"bravo alpha"),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{L"<b>alpha</b> <b>bravo</b> <b>charlie</b> - "
                           L"charlie bravo <b>alpha</b>"},
      snippets);

  // do again, this time with MTQ disabled.
  highlighter->setHandleMultiTermQuery(
      false); // disable but leave phrase processing enabled

  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  snippets = highlighter->highlight(L"body", query, topDocs);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>alpha</b> <b>bravo</b> <b>charlie</b> - charlie bravo alpha"},
      snippets);
}

void TestUnifiedHighlighterStrictPhrases::testMultiValued() 
{
  indexWriter->addDocument(newDoc({L"one bravo three", L"four bravo six"}));
  initReaderSearcherHighlighter();

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(newPhraseQuery(L"body", L"one bravo"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"four bravo"),
                BooleanClause::Occur::MUST)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"body", L"br")),
                BooleanClause::Occur::MUST)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);

  assertArrayEquals(
      std::deque<wstring>{
          L"<b>one</b> <b>bravo</b> three... <b>four</b> <b>bravo</b> six"},
      snippets);

  // now test phraseQuery won't span across values
  assert(indexAnalyzer->getPositionIncrementGap(L"body") > 0);

  shared_ptr<PhraseQuery> phraseQuery = newPhraseQuery(L"body", L"three four");
  // 1 too little; won't span
  phraseQuery =
      setSlop(phraseQuery, indexAnalyzer->getPositionIncrementGap(L"body") - 1);

  query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"bravo")),
                BooleanClause::Occur::MUST)
          ->add(phraseQuery, BooleanClause::Occur::SHOULD)
          ->build();

  topDocs = searcher->search(query, 10);
  snippets = highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(L"one <b>bravo</b> three... four <b>bravo</b> six", snippets[0]);

  // and add just enough slop to cross the values:
  phraseQuery = newPhraseQuery(L"body", L"three four");
  phraseQuery = setSlop(phraseQuery, indexAnalyzer->getPositionIncrementGap(
                                         L"body")); // just enough to span
  query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"bravo")),
                BooleanClause::Occur::MUST)
          ->add(phraseQuery, BooleanClause::Occur::MUST)
          ->build();
  topDocs = searcher->search(query, 10);
  assertEquals(1, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(L"one <b>bravo</b> <b>three</b>... <b>four</b> <b>bravo</b> six",
               snippets[0]);
}

void TestUnifiedHighlighterStrictPhrases::testMaxLen() 
{
  indexWriter->addDocument(newDoc(
      {L"alpha bravo charlie - gap alpha bravo"})); // hyphen is at char 21
  initReaderSearcherHighlighter();
  highlighter->setMaxLength(21);

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(newPhraseQuery(L"body", L"alpha bravo"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"gap alpha"),
                BooleanClause::Occur::MUST)
          ->add(newPhraseQuery(L"body", L"charlie gap"),
                BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  if (fieldType == UHTestHelper::reanalysisType) {
    assertArrayEquals(
        std::deque<wstring>{L"<b>alpha</b> <b>bravo</b> charlie -"}, snippets);
  } else {
    assertArrayEquals(
        std::deque<wstring>{L"<b>alpha</b> <b>bravo</b> <b>charlie</b> -"},
        snippets);
  }
}

void TestUnifiedHighlighterStrictPhrases::testFilteredOutSpan() throw(
    IOException)
{
  indexWriter->addDocument(
      newDoc({L"freezing cold stuff like stuff freedom of speech"}));
  initReaderSearcherHighlighter();

  shared_ptr<WildcardQuery> wildcardQuery =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"free*"));
  shared_ptr<SpanMultiTermQueryWrapper<std::shared_ptr<WildcardQuery>>>
      wildcardSpanQuery = make_shared<
          SpanMultiTermQueryWrapper<std::shared_ptr<WildcardQuery>>>(
          wildcardQuery);
  shared_ptr<SpanTermQuery> termQuery =
      make_shared<SpanTermQuery>(make_shared<Term>(L"body", L"speech"));
  shared_ptr<SpanQuery> spanQuery = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{wildcardSpanQuery, termQuery}, 3,
      false);

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(spanQuery, BooleanClause::Occur::MUST)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertArrayEquals(
      std::deque<wstring>{
          L"freezing cold stuff like stuff <b>freedom</b> of <b>speech</b>"},
      snippets);
}

void TestUnifiedHighlighterStrictPhrases::testMatchNoDocsQuery() throw(
    IOException)
{
  highlighter = make_shared<UnifiedHighlighter>(nullptr, indexAnalyzer);
  highlighter->setHighlightPhrasesStrictly(true);
  wstring content = L"whatever";
  any o = highlighter->highlightWithoutSearcher(
      L"body", make_shared<MatchNoDocsQuery>(), content, 1);
  assertEquals(content, o);
}

void TestUnifiedHighlighterStrictPhrases::testPreSpanQueryRewrite() throw(
    IOException)
{
  indexWriter->addDocument(
      newDoc({L"There is no accord and satisfaction with this - Consideration "
              L"of the accord is arbitrary."}));
  initReaderSearcherHighlighter();

  highlighter = make_shared<UnifiedHighlighterAnonymousInnerClass>(
      shared_from_this(), searcher, indexAnalyzer);
  highlighter->setHighlightPhrasesStrictly(true);

  shared_ptr<BooleanQuery::Builder> bqBuilder =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<Query> phraseQuery = make_shared<BoostQuery>(
      make_shared<PhraseQuery>(L"body", L"accord", L"and", L"satisfaction"),
      2.0f);
  shared_ptr<Query> oredTerms =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(2)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"accord")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"body", L"satisfaction")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"body", L"consideration")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<Query> proximityBoostingQuery = make_shared<MyQuery>(oredTerms);
  shared_ptr<Query> totalQuery =
      bqBuilder->add(phraseQuery, BooleanClause::Occur::SHOULD)
          ->add(proximityBoostingQuery, BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs =
      searcher->search(totalQuery, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", totalQuery, topDocs);
  assertArrayEquals(
      std::deque<wstring>{
          L"There is no <b>accord</b> <b>and</b> <b>satisfaction</b> with this "
          L"- <b>Consideration</b> of the <b>accord</b> is arbitrary."},
      snippets);
}

TestUnifiedHighlighterStrictPhrases::UnifiedHighlighterAnonymousInnerClass::
    UnifiedHighlighterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<deque<std::shared_ptr<Query>>>
TestUnifiedHighlighterStrictPhrases::UnifiedHighlighterAnonymousInnerClass::
    preSpanQueryRewrite(shared_ptr<Query> query)
{
  if (std::dynamic_pointer_cast<MyQuery>(query) != nullptr) {
    return Collections::singletonList(
        (std::static_pointer_cast<MyQuery>(query))->wrapped);
  }
  return nullptr;
}

TestUnifiedHighlighterStrictPhrases::MyQuery::MyQuery(shared_ptr<Query> wrapped)
    : wrapped(wrapped)
{
}

shared_ptr<Weight> TestUnifiedHighlighterStrictPhrases::MyQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return wrapped->createWeight(searcher, needsScores, boost);
}

shared_ptr<Query> TestUnifiedHighlighterStrictPhrases::MyQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> newWrapped = wrapped->rewrite(reader);
  if (newWrapped != wrapped) {
    return make_shared<MyQuery>(newWrapped);
  }
  return shared_from_this();
}

wstring
TestUnifiedHighlighterStrictPhrases::MyQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"[[[" + wrapped->toString(field) + L"]]]";
}

bool TestUnifiedHighlighterStrictPhrases::MyQuery::equals(any obj)
{
  return obj != nullptr && obj.type() == getClass() &&
         wrapped->equals((std::static_pointer_cast<MyQuery>(wrapped))->wrapped);
}

int TestUnifiedHighlighterStrictPhrases::MyQuery::hashCode()
{
  return wrapped->hashCode();
}

void TestUnifiedHighlighterStrictPhrases::testNestedSpanQueryHighlight() throw(
    runtime_error)
{
  // For a long time, the highlighters used to assume all query terms within the
  // SpanQuery were valid at the Spans'
  //   position range.  This would highlight occurrences of terms that were
  //   actually not matched by the query. But now using the SpanCollector API we
  //   don't make this kind of mistake.
  const wstring FIELD_NAME = L"body";
  const wstring indexedText = L"x y z x z x a";
  indexWriter->addDocument(newDoc({indexedText}));
  initReaderSearcherHighlighter();
  shared_ptr<TopDocs> topDocs = make_shared<TopDocs>(
      1, std::deque<std::shared_ptr<ScoreDoc>>{make_shared<ScoreDoc>(0, 1.0f)},
      1.0f);

  wstring expected = L"<b>x</b> <b>y</b> <b>z</b> x z x <b>a</b>";
  shared_ptr<Query> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanNearQuery>(
              std::deque<std::shared_ptr<SpanQuery>>{
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(FIELD_NAME, L"x")),
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(FIELD_NAME, L"y")),
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(FIELD_NAME, L"z"))},
              0, true),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"a"))},
      10, false);
  wstring observed = highlighter->highlight(FIELD_NAME, q, topDocs)[0];
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Nested SpanNear query not properly highlighted.", expected,
               observed);

  expected = L"x <b>y</b> <b>z</b> <b>x</b> <b>z</b> x <b>a</b>";
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanOrQuery>(
              make_shared<SpanNearQuery>(
                  std::deque<std::shared_ptr<SpanQuery>>{
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"x")),
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"z"))},
                  0, true),
              make_shared<SpanNearQuery>(
                  std::deque<std::shared_ptr<SpanQuery>>{
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"y")),
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"z"))},
                  0, true)),
          make_shared<SpanOrQuery>(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"a")),
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"b")))},
      10, false);
  observed = highlighter->highlight(FIELD_NAME, q, topDocs)[0];
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Nested SpanNear query within SpanOr not properly highlighted.",
               expected, observed);

  expected = L"x <b>y</b> <b>z</b> <b>x</b> <b>z</b> x <b>a</b>";
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanNearQuery>(
              std::deque<std::shared_ptr<SpanQuery>>{
                  make_shared<SpanMultiTermQueryWrapper<>>(
                      make_shared<WildcardQuery>(
                          make_shared<Term>(FIELD_NAME, L"*"))),
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(FIELD_NAME, L"z"))},
              0, true),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"a"))},
      10, false);
  observed = highlighter->highlight(FIELD_NAME, q, topDocs)[0];
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Nested SpanNear query with wildcard not properly highlighted.",
               expected, observed);

  expected = L"<b>x</b> <b>y</b> z x z x <b>a</b>";
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanOrQuery>(
              make_shared<SpanNearQuery>(
                  std::deque<std::shared_ptr<SpanQuery>>{
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"x")),
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"y"))},
                  0, true),
              make_shared<SpanNearQuery>(
                  std::deque<std::shared_ptr<SpanQuery>>{
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"z")),
                      make_shared<SpanTermQuery>(
                          make_shared<Term>(FIELD_NAME, L"a"))},
                  0, true)),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"a"))},
      10, false);
  observed = highlighter->highlight(FIELD_NAME, q, topDocs)[0];
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Nested SpanNear query within SpanOr not properly highlighted.",
               expected, observed);
}
} // namespace org::apache::lucene::search::uhighlight