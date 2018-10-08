using namespace std;

#include "TestUnifiedHighlighter.h"

namespace org::apache::lucene::search::uhighlight
{
using com::carrotsearch::randomizedtesting::annotations::ParametersFactory;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using HighlightFlag =
    org::apache::lucene::search::uhighlight::UnifiedHighlighter::HighlightFlag;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ParametersFactory public static Iterable<Object[]>
// parameters()
deque<std::deque<any>> TestUnifiedHighlighter::parameters()
{
  return UHTestHelper::parametersFactoryList();
}

TestUnifiedHighlighter::TestUnifiedHighlighter(shared_ptr<FieldType> fieldType)
    : fieldType(fieldType)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
void TestUnifiedHighlighter::doBefore() 
{
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE,
                                true); // whitespace, punctuation, lowercase
  dir = newDirectory();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
void TestUnifiedHighlighter::doAfter()  { delete dir; }

shared_ptr<UnifiedHighlighter> TestUnifiedHighlighter::randomUnifiedHighlighter(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer)
{
  return randomUnifiedHighlighter(searcher, indexAnalyzer,
                                  EnumSet::noneOf(HighlightFlag::typeid));
}

shared_ptr<UnifiedHighlighter> TestUnifiedHighlighter::randomUnifiedHighlighter(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer,
    shared_ptr<EnumSet<HighlightFlag>> mandatoryFlags)
{
  if (random()->nextBoolean()) {
    return make_shared<UnifiedHighlighter>(searcher, indexAnalyzer);
  } else {
    shared_ptr<UnifiedHighlighter> *const uh =
        make_shared<UnifiedHighlighterAnonymousInnerClass>(
            searcher, indexAnalyzer, mandatoryFlags);
    uh->setCacheFieldValCharsThreshold(random()->nextInt(100));
    if (random()->nextBoolean()) {
      uh->setFieldMatcher([&](any f) { true; }); // requireFieldMatch==false
    }
    return uh;
  }
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass::
    UnifiedHighlighterAnonymousInnerClass(
        shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer,
        shared_ptr<EnumSet<HighlightFlag>> mandatoryFlags)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->mandatoryFlags = mandatoryFlags;
}

shared_ptr<Set<HighlightFlag>>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass::getFlags(
    const wstring &field)
{
  shared_ptr<EnumSet<HighlightFlag>> *const result =
      EnumSet::copyOf(mandatoryFlags);
  int r = LuceneTestCase::random()->nextInt();
  for (HighlightFlag highlightFlag : HighlightFlag::values()) {
    if (((1 << static_cast<int>(highlightFlag)) & r) == 0) {
      result->add(highlightFlag);
    }
  }
  return result;
}

void TestUnifiedHighlighter::testBasics() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  iw->addDocument(doc);
  body->setStringValue(L"Highlighting the first term. Hope it works.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"Just a test <b>highlighting</b> from postings. ", snippets[0]);
  assertEquals(L"<b>Highlighting</b> the first term. ", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighter::testFormatWithMatchExceedingContentLength2() throw(
    runtime_error)
{

  wstring bodyText = L"123 TEST 01234 TEST";

  std::deque<wstring> snippets =
      formatWithMatchExceedingContentLength(bodyText);

  assertEquals(1, snippets.size());
  assertEquals(L"123 <b>TEST</b> 01234 TE", snippets[0]);
}

void TestUnifiedHighlighter::testFormatWithMatchExceedingContentLength3() throw(
    runtime_error)
{

  wstring bodyText = L"123 5678 01234 TEST TEST";

  std::deque<wstring> snippets =
      formatWithMatchExceedingContentLength(bodyText);

  assertEquals(1, snippets.size());
  assertEquals(L"123 5678 01234 TE", snippets[0]);
}

void TestUnifiedHighlighter::testFormatWithMatchExceedingContentLength() throw(
    runtime_error)
{

  wstring bodyText = L"123 5678 01234 TEST";

  std::deque<wstring> snippets =
      formatWithMatchExceedingContentLength(bodyText);

  assertEquals(1, snippets.size());
  // LUCENE-5166: no snippet
  assertEquals(L"123 5678 01234 TE", snippets[0]);
}

std::deque<wstring>
TestUnifiedHighlighter::formatWithMatchExceedingContentLength(
    const wstring &bodyText) 
{

  int maxLength = 17;

  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> *const body =
      make_shared<Field>(L"body", bodyText, fieldType);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);

  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxLength(maxLength);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);

  delete ir;
  return snippets;
}

void TestUnifiedHighlighter::testHighlightLastWord() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(1, snippets.size());
  assertEquals(L"This is a <b>test</b>", snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testOneSentence() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighter::testMaxLengthWithMultivalue() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();

  const wstring value = L"This is a multivalued field. Sentencetwo field.";
  doc->push_back(make_shared<Field>(L"body", value, fieldType));
  doc->push_back(make_shared<Field>(L"body", value, fieldType));
  doc->push_back(make_shared<Field>(L"body", value, fieldType));

  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxLength(value.length() * 2 + 1);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"field"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 10);
  assertEquals(1, snippets.size());
  wstring highlightedValue =
      L"This is a multivalued <b>field</b>. Sentencetwo <b>field</b>.";
  assertEquals(highlightedValue + L"... " + highlightedValue, snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testMultipleFields() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Field> title = make_shared<Field>(
      L"title", L"", UHTestHelper::randomFieldType(random()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);
  doc->push_back(title);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  title->setStringValue(L"I am hoping for the best.");
  iw->addDocument(doc);
  body->setStringValue(L"Highlighting the first term. Hope it works.");
  title->setStringValue(L"But best may not be good enough.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"body", L"highlighting")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"best")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  unordered_map<wstring, std::deque<wstring>> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body", L"title"},
                                   query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"Just a test <b>highlighting</b> from postings. ",
               snippets[L"body"][0]);
  assertEquals(L"<b>Highlighting</b> the first term. ", snippets[L"body"][1]);
  assertEquals(L"I am hoping for the <b>best</b>.", snippets[L"title"][0]);
  assertEquals(L"But <b>best</b> may not be good enough.",
               snippets[L"title"][1]);
  delete ir;
}

void TestUnifiedHighlighter::testMultipleTerms() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  iw->addDocument(doc);
  body->setStringValue(L"Highlighting the first term. Hope it works.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"body", L"highlighting")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"just")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"first")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"<b>Just</b> a test <b>highlighting</b> from postings. ",
               snippets[0]);
  assertEquals(L"<b>Highlighting</b> the <b>first</b> term. ", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighter::testMultiplePassages() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  iw->addDocument(doc);
  body->setStringValue(
      L"This test is another test. Not a good sentence. Test test test test.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(2, snippets.size());
  assertEquals(
      L"This is a <b>test</b>. Just a <b>test</b> highlighting from postings. ",
      snippets[0]);
  assertEquals(L"This <b>test</b> is another <b>test</b>. ... <b>Test</b> "
               L"<b>test</b> <b>test</b> <b>test</b>.",
               snippets[1]);

  delete ir;
}

void TestUnifiedHighlighter::testBuddhism() 
{
  wstring text =
      wstring(L"This eight-volume set brings together seminal papers in "
              L"Buddhist studies from a vast ") +
      L"range of academic disciplines published over the last forty years. "
      L"With a new introduction " +
      L"by the editor, this collection is a unique and unrivalled research "
      L"resource for both " +
      L"student and scholar. Coverage includes: - Buddhist origins; early "
      L"history of Buddhism in " +
      L"South and Southeast Asia - early Buddhist Schools and Doctrinal "
      L"History; Theravada Doctrine " +
      L"- the Origins and nature of Mahayana Buddhism; some Mahayana religious "
      L"topics - Abhidharma " +
      L"and Madhyamaka - Yogacara, the Epistemological tradition, and "
      L"Tathagatagarbha - Tantric " +
      L"Buddhism (Including China and Japan); Buddhism in Nepal and Tibet - "
      L"Buddhism in South and " +
      L"Southeast Asia, and - Buddhism in China, East Asia, and Japan.";
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", text, fieldType);
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(body);
  iw->addDocument(document);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<PhraseQuery> query =
      (make_shared<PhraseQuery::Builder>())
          ->add(make_shared<Term>(L"body", L"buddhist"))
          ->add(make_shared<Term>(L"body", L"origins"))
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10);
  assertEquals(1, topDocs->totalHits);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setHighlightPhrasesStrictly(false);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertTrue(snippets[0].find(L"<b>Buddhist</b> <b>origins</b>") !=
             wstring::npos);
  delete ir;
}

void TestUnifiedHighlighter::testCuriousGeorge() 
{
  wstring text = wstring(L"It’s the formula for success for "
                         L"preschoolers—Curious George and fire trucks! ") +
                 L"Curious George and the Firefighters is a story based on H. "
                 L"A. and Margret Rey’s " +
                 L"popular primate and painted in the original watercolor and "
                 L"charcoal style. " +
                 L"Firefighters are a famously brave lot, but can they "
                 L"withstand a visit from one curious monkey?";
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", text, fieldType);
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(body);
  iw->addDocument(document);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<PhraseQuery> query =
      (make_shared<PhraseQuery::Builder>())
          ->add(make_shared<Term>(L"body", L"curious"))
          ->add(make_shared<Term>(L"body", L"george"))
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10);
  assertEquals(1, topDocs->totalHits);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setHighlightPhrasesStrictly(false);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertFalse(snippets[0].find(L"<b>Curious</b>Curious") != wstring::npos);
  delete ir;
}

void TestUnifiedHighlighter::testCambridgeMA() 
{
  shared_ptr<BufferedReader> r =
      make_shared<BufferedReader>(make_shared<InputStreamReader>(
          this->getClass().getResourceAsStream(L"CambridgeMA.utf8"),
          StandardCharsets::UTF_8));
  wstring text = r->readLine();
  r->close();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
  shared_ptr<Field> body = make_shared<Field>(L"body", text, fieldType);
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(body);
  iw->addDocument(document);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"porter")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"square")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"body", L"massachusetts")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10);
  assertEquals(1, topDocs->totalHits);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxLength(numeric_limits<int>::max() - 1);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertTrue(snippets[0].find(L"<b>Square</b>") != wstring::npos);
  assertTrue(snippets[0].find(L"<b>Porter</b>") != wstring::npos);
  delete ir;
}

void TestUnifiedHighlighter::testPassageRanking() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(
      L"This is a test.  Just highlighting from postings. This is also a much "
      L"sillier test.  Feel free to test test test test test test test.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertEquals(
      L"This is a <b>test</b>.  ... Feel free to <b>test</b> <b>test</b> "
      L"<b>test</b> <b>test</b> <b>test</b> <b>test</b> <b>test</b>.",
      snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testBooleanMustNot() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(
      L"body", L"This sentence has both terms.  This sentence has only terms.",
      fieldType);
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(body);
  iw->addDocument(document);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<BooleanQuery> query2 =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"both")),
                BooleanClause::Occur::MUST_NOT)
          ->build();

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"terms")),
                BooleanClause::Occur::SHOULD)
          ->add(query2, BooleanClause::Occur::SHOULD)
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(query, 10);
  assertEquals(1, topDocs->totalHits);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxLength(numeric_limits<int>::max() - 1);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertFalse(snippets[0].find(L"<b>both</b>") != wstring::npos);
  delete ir;
}

void TestUnifiedHighlighter::testHighlightAllText() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(
      L"This is a test.  Just highlighting from postings. This is also a much "
      L"sillier test.  Feel free to test test test test test test test.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass2>(
          shared_from_this(), searcher, indexAnalyzer);
  highlighter->setMaxLength(10000);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertEquals(
      L"This is a <b>test</b>.  Just highlighting from postings. This is also "
      L"a much sillier <b>test</b>.  Feel free to <b>test</b> <b>test</b> "
      L"<b>test</b> <b>test</b> <b>test</b> <b>test</b> <b>test</b>.",
      snippets[0]);

  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass2::
    UnifiedHighlighterAnonymousInnerClass2(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<BreakIterator>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass2::
    getBreakIterator(const wstring &field)
{
  return make_shared<WholeBreakIterator>();
}

void TestUnifiedHighlighter::testSpecificDocIDs() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  iw->addDocument(doc);
  body->setStringValue(L"Highlighting the first term. Hope it works.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<std::shared_ptr<ScoreDoc>> hits = topDocs->scoreDocs;
  std::deque<int> docIDs(2);
  docIDs[0] = hits[0]->doc;
  docIDs[1] = hits[1]->doc;
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{1})[L"body"];
  assertEquals(2, snippets.size());
  assertEquals(L"Just a test <b>highlighting</b> from postings. ", snippets[0]);
  assertEquals(L"<b>Highlighting</b> the first term. ", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighter::testCustomFieldValueSource() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();

  const wstring text =
      L"This is a test.  Just highlighting from postings. This is also a much "
      L"sillier test.  Feel free to test test test test test test test.";
  shared_ptr<Field> body = make_shared<Field>(L"body", text, fieldType);
  doc->push_back(body);
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass3>(
          shared_from_this(), searcher, indexAnalyzer, text);

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 2);
  assertEquals(1, snippets.size());
  assertEquals(
      L"This is a <b>test</b>.  Just highlighting from postings. This is also "
      L"a much sillier <b>test</b>.  Feel free to <b>test</b> <b>test</b> "
      L"<b>test</b> <b>test</b> <b>test</b> <b>test</b> <b>test</b>.",
      snippets[0]);

  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass3::
    UnifiedHighlighterAnonymousInnerClass3(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer, const wstring &text)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
  this->text = text;
}

deque<std::deque<std::shared_ptr<std::wstring>>>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass3::loadFieldValues(
    std::deque<wstring> &fields, shared_ptr<DocIdSetIterator> docIter,
    int cacheCharsThreshold) 
{
  assert(fields.size() == 1);
  assert(docIter->cost() == 1);
  docIter->nextDoc();
  return Collections::singletonList(
      std::deque<std::shared_ptr<std::wstring>>{text});
}

shared_ptr<BreakIterator>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass3::
    getBreakIterator(const wstring &field)
{
  return make_shared<WholeBreakIterator>();
}

void TestUnifiedHighlighter::testEmptyHighlights() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<Field> body =
      make_shared<Field>(L"body",
                         L"test this is.  another sentence this test has.  far "
                         L"away is that planet.",
                         fieldType);
  doc->push_back(body);
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  std::deque<int> docIDs = {0};
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(L"test this is.  another sentence this test has.  ",
               snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testNothingAnalyzes() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(
      L"body", L" ", fieldType)); // just a space! (thus not empty)
  doc->push_back(newTextField(L"id", L"id", Field::Store::YES));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"something", fieldType));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  int docID =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
          ->scoreDocs[0]
          ->doc;

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  std::deque<int> docIDs(1);
  docIDs[0] = docID;
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(L" ", snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testCustomEmptyHighlights() 
{
  indexAnalyzer->setPositionIncrementGap(10);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<Field> body =
      make_shared<Field>(L"body",
                         L"test this is.  another sentence this test has.  far "
                         L"away is that planet.",
                         fieldType);
  doc->push_back(body);
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxNoHighlightPassages(0); // don't want any default summary
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  std::deque<int> docIDs = {0};
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertNull(snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testEmptyHighlightsWhole() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<Field> body =
      make_shared<Field>(L"body",
                         L"test this is.  another sentence this test has.  far "
                         L"away is that planet.",
                         fieldType);
  doc->push_back(body);
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass4>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  std::deque<int> docIDs = {0};
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(L"test this is.  another sentence this test has.  far away is "
               L"that planet.",
               snippets[0]);

  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass4::
    UnifiedHighlighterAnonymousInnerClass4(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<BreakIterator>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass4::
    getBreakIterator(const wstring &field)
{
  return make_shared<WholeBreakIterator>();
}

void TestUnifiedHighlighter::testFieldIsMissing() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<Field> body =
      make_shared<Field>(L"body",
                         L"test this is.  another sentence this test has.  far "
                         L"away is that planet.",
                         fieldType);
  doc->push_back(body);
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"bogus", L"highlighting"));
  std::deque<int> docIDs = {0};
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"bogus"}, query,
                                   docIDs, std::deque<int>{2})[L"bogus"];
  assertEquals(1, snippets.size());
  assertNull(snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testFieldIsJustSpace() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"   ", fieldType));
  doc->push_back(newTextField(L"id", L"id", Field::Store::YES));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"something", fieldType));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  int docID =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
          ->scoreDocs[0]
          ->doc;

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  std::deque<int> docIDs(1);
  docIDs[0] = docID;
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(L"   ", snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testFieldIsEmptyString() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"", fieldType));
  doc->push_back(newTextField(L"id", L"id", Field::Store::YES));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"something", fieldType));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  int docID =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
          ->scoreDocs[0]
          ->doc;

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  std::deque<int> docIDs(1);
  docIDs[0] = docID;
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertNull(snippets[0]);

  delete ir;
}

void TestUnifiedHighlighter::testMultipleDocs() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring content = L"the answer is " + to_wstring(i);
    if ((i & 1) == 0) {
      content += L" some more terms";
    }
    doc->push_back(make_shared<Field>(L"body", content, fieldType));
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    iw->addDocument(doc);

    if (random()->nextInt(10) == 2) {
      iw->commit();
    }
  }

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setCacheFieldValCharsThreshold(random()->nextInt(10) *
                                              10); // 0 thru 90 intervals of 10
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"answer"));
  shared_ptr<TopDocs> hits = searcher->search(query, numDocs);
  assertEquals(numDocs, hits->totalHits);

  std::deque<wstring> snippets = highlighter->highlight(L"body", query, hits);
  assertEquals(numDocs, snippets.size());
  for (int hit = 0; hit < numDocs; hit++) {
    shared_ptr<Document> doc = searcher->doc(hits->scoreDocs[hit]->doc);
    int id = static_cast<Integer>(doc[L"id"]);
    wstring expected = L"the <b>answer</b> is " + to_wstring(id);
    if ((id & 1) == 0) {
      expected += L" some more terms";
    }
    assertEquals(expected, snippets[hit]);
  }

  delete ir;
}

void TestUnifiedHighlighter::testMultipleSnippetSizes() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Field> title = make_shared<Field>(
      L"title", L"", UHTestHelper::randomFieldType(random()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);
  doc->push_back(title);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  title->setStringValue(L"This is a test. Just a test highlighting from "
                        L"postings. Feel free to ignore.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"test")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"test")),
                BooleanClause::Occur::SHOULD)
          ->build();
  unordered_map<wstring, std::deque<wstring>> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"title", L"body"},
                                   query, std::deque<int>{0},
                                   std::deque<int>{1, 2});
  wstring titleHighlight = snippets[L"title"][0];
  wstring bodyHighlight = snippets[L"body"][0];
  assertEquals(L"This is a <b>test</b>. ", titleHighlight);
  assertEquals(
      L"This is a <b>test</b>. Just a <b>test</b> highlighting from postings. ",
      bodyHighlight);
  delete ir;
}

void TestUnifiedHighlighter::testEncode() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"<i>postings</i>. Feel free to ignore.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass5>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(1, snippets.size());
  assertEquals(L"Just&#32;a&#32;test&#32;<b>highlighting</"
               L"b>&#32;from&#32;&lt;i&gt;postings&lt;&#x2F;i&gt;&#46;&#32;",
               snippets[0]);

  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass5::
    UnifiedHighlighterAnonymousInnerClass5(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PassageFormatter>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass5::getFormatter(
    const wstring &field)
{
  return make_shared<DefaultPassageFormatter>(L"<b>", L"</b>", L"... ", true);
}

void TestUnifiedHighlighter::testObjectFormatter() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test. Just a test highlighting from "
                       L"postings. Feel free to ignore.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass6>(
          shared_from_this(), searcher, indexAnalyzer);

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"highlighting"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<int> docIDs(1);
  docIDs[0] = topDocs->scoreDocs[0]->doc;
  unordered_map<wstring, std::deque<any>> snippets =
      highlighter->highlightFieldsAsObjects(std::deque<wstring>{L"body"},
                                            query, docIDs, std::deque<int>{1});
  std::deque<any> bodySnippets = snippets[L"body"];
  assertEquals(1, bodySnippets.size());
  assertTrue(Arrays::equals(
      std::deque<wstring>{L"blah blah",
                           L"Just a test <b>highlighting</b> from postings. "},
      any_cast<std::deque<wstring>>(bodySnippets[0])));

  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass6::
    UnifiedHighlighterAnonymousInnerClass6(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PassageFormatter>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass6::getFormatter(
    const wstring &field)
{
  return make_shared<PassageFormatterAnonymousInnerClass>(shared_from_this());
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass6::
    PassageFormatterAnonymousInnerClass::PassageFormatterAnonymousInnerClass(
        shared_ptr<UnifiedHighlighterAnonymousInnerClass6> outerInstance)
{
  this->outerInstance = outerInstance;
  defaultFormatter = make_shared<DefaultPassageFormatter>();
}

std::deque<wstring>
TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass6::
    PassageFormatterAnonymousInnerClass::format(
        std::deque<std::shared_ptr<Passage>> &passages, const wstring &content)
{
  // Just turns the std::wstring snippet into a length 2
  // array of std::wstring
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return std::deque<wstring>{
      L"blah blah", defaultFormatter::format(passages, content)->toString()};
}

shared_ptr<IndexReader>
TestUnifiedHighlighter::indexSomeFields() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setIndexOptions(IndexOptions::NONE);
  ft->setTokenized(false);
  ft->setStored(true);
  ft->freeze();

  shared_ptr<Field> title = make_shared<Field>(L"title", L"", fieldType);
  shared_ptr<Field> text = make_shared<Field>(L"text", L"", fieldType);
  shared_ptr<Field> category = make_shared<Field>(L"category", L"", fieldType);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(title);
  doc->push_back(text);
  doc->push_back(category);
  title->setStringValue(L"This is the title field.");
  text->setStringValue(
      L"This is the text field. You can put some text if you want.");
  category->setStringValue(L"This is the category field.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  return ir;
}

void TestUnifiedHighlighter::testFieldMatcherTermQuery() 
{
  shared_ptr<IndexReader> ir = indexSomeFields();
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighterNoFieldMatch =
      make_shared<UnifiedHighlighterAnonymousInnerClass7>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<UnifiedHighlighter> highlighterFieldMatch =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighterFieldMatch->setFieldMatcher(nullptr); // default
  shared_ptr<BooleanQuery::Builder> queryBuilder =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"text", L"some")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"text", L"field")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"text", L"this")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"is")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"this")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"category", L"this")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"category", L"some")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(L"category", L"category")),
                BooleanClause::Occur::SHOULD);
  shared_ptr<Query> query = queryBuilder->build();

  {
    // title
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the title <b>field</b>.", snippets[0]);

    snippets = highlighterFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the title field.", snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"text" == fq; });
    snippets = highlighterFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> is the title <b>field</b>.", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }

  {
    // text
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the text <b>field</b>. You can put "
                 L"<b>some</b> text if you want.",
                 snippets[0]);

    snippets = highlighterFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> is the text <b>field</b>. You can put "
                 L"<b>some</b> text if you want.",
                 snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"title" == fq; });
    snippets = highlighterFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the text field. ", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }

  {
    // category
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the <b>category</b> <b>field</b>.",
                 snippets[0]);

    snippets =
        highlighterFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> is the <b>category</b> field.", snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"title" == fq; });
    snippets =
        highlighterFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the category field.", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }
  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass7::
    UnifiedHighlighterAnonymousInnerClass7(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<protected> function <
    TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass7::bool(
        shared_ptr<const> std)
{
  // requireFieldMatch=false
  return [&](qf) { true; };
}

void TestUnifiedHighlighter::testFieldMatcherMultiTermQuery() throw(
    runtime_error)
{
  shared_ptr<IndexReader> ir = indexSomeFields();
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighterNoFieldMatch =
      make_shared<UnifiedHighlighterAnonymousInnerClass8>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<UnifiedHighlighter> highlighterFieldMatch =
      randomUnifiedHighlighter(searcher, indexAnalyzer,
                               EnumSet::of(HighlightFlag::MULTI_TERM_QUERY));
  highlighterFieldMatch->setFieldMatcher(nullptr); // default
  shared_ptr<BooleanQuery::Builder> queryBuilder =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<FuzzyQuery>(make_shared<Term>(L"text", L"sime"), 1),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"text", L"fie")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"text", L"thi")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"title", L"is")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"title", L"thi")),
                BooleanClause::Occur::SHOULD)
          ->add(
              make_shared<PrefixQuery>(make_shared<Term>(L"category", L"thi")),
              BooleanClause::Occur::SHOULD)
          ->add(make_shared<FuzzyQuery>(make_shared<Term>(L"category", L"sime"),
                                        1),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PrefixQuery>(
                    make_shared<Term>(L"category", L"categ")),
                BooleanClause::Occur::SHOULD);
  shared_ptr<Query> query = queryBuilder->build();

  {
    // title
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the title <b>field</b>.", snippets[0]);

    snippets = highlighterFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the title field.", snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"text" == fq; });
    snippets = highlighterFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> is the title <b>field</b>.", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }

  {
    // text
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the text <b>field</b>. You can put "
                 L"<b>some</b> text if you want.",
                 snippets[0]);

    snippets = highlighterFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> is the text <b>field</b>. You can put "
                 L"<b>some</b> text if you want.",
                 snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"title" == fq; });
    snippets = highlighterFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the text field. ", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }

  {
    // category
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the <b>category</b> <b>field</b>.",
                 snippets[0]);

    snippets =
        highlighterFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> is the <b>category</b> field.", snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"title" == fq; });
    snippets =
        highlighterFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the category field.", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }
  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass8::
    UnifiedHighlighterAnonymousInnerClass8(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<protected> function <
    TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass8::bool(
        shared_ptr<const> std)
{
  // requireFieldMatch=false
  return [&](qf) { true; };
}

void TestUnifiedHighlighter::testFieldMatcherPhraseQuery() 
{
  shared_ptr<IndexReader> ir = indexSomeFields();
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighterNoFieldMatch =
      make_shared<UnifiedHighlighterAnonymousInnerClass9>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<UnifiedHighlighter> highlighterFieldMatch =
      randomUnifiedHighlighter(searcher, indexAnalyzer,
                               EnumSet::of(HighlightFlag::PHRASES));
  highlighterFieldMatch->setFieldMatcher(nullptr); // default
  shared_ptr<BooleanQuery::Builder> queryBuilder =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<PhraseQuery>(L"title", L"this", L"is", L"the",
                                         L"title"),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PhraseQuery>(2, L"category", L"this", L"is", L"the",
                                         L"field"),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PhraseQuery>(L"text", L"this", L"is"),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PhraseQuery>(L"category", L"this", L"is"),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<PhraseQuery>(1, L"text", L"you", L"can", L"put",
                                         L"text"),
                BooleanClause::Occur::SHOULD);
  shared_ptr<Query> query = queryBuilder->build();

  {
    // title
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> <b>the</b> <b>title</b> <b>field</b>.",
                 snippets[0]);

    snippets = highlighterFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> <b>the</b> <b>title</b> field.",
                 snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"text" == fq; });
    snippets = highlighterFieldMatch->highlight(L"title", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the title field.", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }

  {
    // text
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(
        L"<b>This</b> <b>is</b> <b>the</b> <b>text</b> <b>field</b>. "
        L"<b>You</b> <b>can</b> <b>put</b> some <b>text</b> if you want.",
        snippets[0]);

    snippets = highlighterFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the <b>text</b> field. <b>You</b> "
                 L"<b>can</b> <b>put</b> some <b>text</b> if you want.",
                 snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"title" == fq; });
    snippets = highlighterFieldMatch->highlight(L"text", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"This is the text field. You can put some text if you want.",
                 snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }

  {
    // category
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
    assertEquals(1, topDocs->totalHits);
    std::deque<wstring> snippets =
        highlighterNoFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> <b>the</b> category <b>field</b>.",
                 snippets[0]);

    snippets =
        highlighterFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> <b>the</b> category <b>field</b>.",
                 snippets[0]);

    highlighterFieldMatch->setFieldMatcher([&](fq) { L"text" == fq; });
    snippets =
        highlighterFieldMatch->highlight(L"category", query, topDocs, 10);
    assertEquals(1, snippets.size());
    assertEquals(L"<b>This</b> <b>is</b> the category field.", snippets[0]);
    highlighterFieldMatch->setFieldMatcher(nullptr);
  }
  delete ir;
}

TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass9::
    UnifiedHighlighterAnonymousInnerClass9(
        shared_ptr<TestUnifiedHighlighter> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<MockAnalyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<protected> function <
    TestUnifiedHighlighter::UnifiedHighlighterAnonymousInnerClass9::bool(
        shared_ptr<const> std)
{
  // requireFieldMatch=false
  return [&](qf) { true; };
}
} // namespace org::apache::lucene::search::uhighlight