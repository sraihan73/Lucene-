using namespace std;

#include "HighlighterTest.h"

namespace org::apache::lucene::search::highlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockPayloadAnalyzer = org::apache::lucene::analysis::MockPayloadAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using NGramTokenizer = org::apache::lucene::analysis::ngram::NGramTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Term = org::apache::lucene::index::Term;
using CommonTermsQuery = org::apache::lucene::queries::CommonTermsQuery;
using CustomScoreQuery = org::apache::lucene::queries::CustomScoreQuery;
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using SpanPayloadCheckQuery =
    org::apache::lucene::queries::payloads::SpanPayloadCheckQuery;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Builder = org::apache::lucene::search::PhraseQuery::Builder;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using TestHighlightRunner = org::apache::lucene::search::highlight::
    SynonymTokenizer::TestHighlightRunner;
using BitSetProducer = org::apache::lucene::search::join::BitSetProducer;
using QueryBitSetProducer =
    org::apache::lucene::search::join::QueryBitSetProducer;
using ScoreMode = org::apache::lucene::search::join::ScoreMode;
using ToChildBlockJoinQuery =
    org::apache::lucene::search::join::ToChildBlockJoinQuery;
using ToParentBlockJoinQuery =
    org::apache::lucene::search::join::ToParentBlockJoinQuery;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using org::w3c::dom::Element;
using org::w3c::dom::NodeList;
const wstring HighlighterTest::FIELD_NAME = L"contents";
const wstring HighlighterTest::NUMERIC_FIELD_NAME = L"nfield";

shared_ptr<TokenStream>
HighlighterTest::getAnyTokenStream(const wstring &fieldName,
                                   int docId) 
{
  return TokenSources::getTokenStream(
      fieldName, searcher->getIndexReader()->getTermVectors(docId),
      searcher->doc(docId)[fieldName], analyzer, -1);
}

void HighlighterTest::testCustomScoreQueryHighlight() 
{
  shared_ptr<TermQuery> termQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"very"));
  shared_ptr<CustomScoreQuery> query = make_shared<CustomScoreQuery>(termQuery);

  searcher = newSearcher(reader);
  shared_ptr<TopDocs> hits = searcher->search(
      query, 10,
      make_shared<Sort>(SortField::FIELD_DOC, SortField::FIELD_SCORE));
  assertEquals(2, hits->totalHits);
  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  constexpr int docId0 = hits->scoreDocs[0]->doc;
  shared_ptr<Document> doc = searcher->doc(docId0);
  wstring storedField = doc[FIELD_NAME];

  shared_ptr<TokenStream> stream = getAnyTokenStream(FIELD_NAME, docId0);
  shared_ptr<Fragmenter> fragmenter = make_shared<SimpleSpanFragmenter>(scorer);
  highlighter->setTextFragmenter(fragmenter);
  wstring fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"Hello this is a piece of text that is <B>very</B> long and "
               L"contains too much preamble and the meat is really here which "
               L"says kennedy has been shot",
               fragment);
}

void HighlighterTest::testFunctionScoreQuery() 
{
  shared_ptr<TermQuery> termQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"very"));
  shared_ptr<FunctionScoreQuery> query = make_shared<FunctionScoreQuery>(
      termQuery, DoubleValuesSource::constant(1));

  searcher = newSearcher(reader);
  shared_ptr<TopDocs> hits = searcher->search(
      query, 10,
      make_shared<Sort>(SortField::FIELD_DOC, SortField::FIELD_SCORE));
  assertEquals(2, hits->totalHits);
  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  constexpr int docId0 = hits->scoreDocs[0]->doc;
  shared_ptr<Document> doc = searcher->doc(docId0);
  wstring storedField = doc[FIELD_NAME];

  shared_ptr<TokenStream> stream = getAnyTokenStream(FIELD_NAME, docId0);
  shared_ptr<Fragmenter> fragmenter = make_shared<SimpleSpanFragmenter>(scorer);
  highlighter->setTextFragmenter(fragmenter);
  wstring fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"Hello this is a piece of text that is <B>very</B> long and "
               L"contains too much preamble and the meat is really here which "
               L"says kennedy has been shot",
               fragment);
}

void HighlighterTest::testQueryScorerHits() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(FIELD_NAME, L"very", L"long");

  query = phraseQuery;
  searcher = newSearcher(reader);
  shared_ptr<TopDocs> hits = searcher->search(query, 10);

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> doc = searcher->doc(docId);
    wstring storedField = doc[FIELD_NAME];

    shared_ptr<TokenStream> stream = getAnyTokenStream(FIELD_NAME, docId);

    shared_ptr<Fragmenter> fragmenter =
        make_shared<SimpleSpanFragmenter>(scorer);

    highlighter->setTextFragmenter(fragmenter);

    wstring fragment = highlighter->getBestFragment(stream, storedField);

    if (VERBOSE) {
      wcout << fragment << endl;
    }
  }
}

void HighlighterTest::testHighlightingCommonTermsQuery() 
{
  shared_ptr<CommonTermsQuery> query =
      make_shared<CommonTermsQuery>(Occur::MUST, Occur::SHOULD, 3);
  query->add(make_shared<Term>(FIELD_NAME, L"this")); // stop-word
  query->add(make_shared<Term>(FIELD_NAME, L"long"));
  query->add(make_shared<Term>(FIELD_NAME, L"very"));

  searcher = newSearcher(reader);
  shared_ptr<TopDocs> hits = searcher->search(
      query, 10,
      make_shared<Sort>(SortField::FIELD_DOC, SortField::FIELD_SCORE));
  assertEquals(2, hits->totalHits);
  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  constexpr int docId0 = hits->scoreDocs[0]->doc;
  shared_ptr<Document> doc = searcher->doc(docId0);
  wstring storedField = doc[FIELD_NAME];

  shared_ptr<TokenStream> stream = getAnyTokenStream(FIELD_NAME, docId0);
  shared_ptr<Fragmenter> fragmenter = make_shared<SimpleSpanFragmenter>(scorer);
  highlighter->setTextFragmenter(fragmenter);
  wstring fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"Hello this is a piece of text that is <B>very</B> <B>long</B> "
               L"and contains too much preamble and the meat is really here "
               L"which says kennedy has been shot",
               fragment);

  constexpr int docId1 = hits->scoreDocs[1]->doc;
  doc = searcher->doc(docId1);
  storedField = doc[FIELD_NAME];

  stream = getAnyTokenStream(FIELD_NAME, docId1);
  highlighter->setTextFragmenter(make_shared<SimpleSpanFragmenter>(scorer));
  fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"This piece of text refers to Kennedy at the beginning then "
               L"has a longer piece of text that is <B>very</B>",
               fragment);
}

void HighlighterTest::testHighlightingSynonymQuery() 
{
  searcher = newSearcher(reader);
  shared_ptr<Query> query =
      make_shared<SynonymQuery>(make_shared<Term>(FIELD_NAME, L"jfk"),
                                make_shared<Term>(FIELD_NAME, L"kennedy"));
  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);
  shared_ptr<TokenStream> stream = getAnyTokenStream(FIELD_NAME, 2);
  shared_ptr<Fragmenter> fragmenter = make_shared<SimpleSpanFragmenter>(scorer);
  highlighter->setTextFragmenter(fragmenter);
  wstring storedField = searcher->doc(2)[FIELD_NAME];
  wstring fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"<B>JFK</B> has been shot", fragment);

  stream = getAnyTokenStream(FIELD_NAME, 3);
  storedField = searcher->doc(3)[FIELD_NAME];
  fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"John <B>Kennedy</B> has been shot", fragment);
}

void HighlighterTest::testHighlightUnknownQueryAfterRewrite() throw(
    IOException, InvalidTokenOffsetsException)
{
  shared_ptr<Query> query =
      make_shared<QueryAnonymousInnerClass>(shared_from_this());

  searcher = newSearcher(reader);
  shared_ptr<TopDocs> hits = searcher->search(
      query, 10,
      make_shared<Sort>(SortField::FIELD_DOC, SortField::FIELD_SCORE));
  assertEquals(2, hits->totalHits);
  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  constexpr int docId0 = hits->scoreDocs[0]->doc;
  shared_ptr<Document> doc = searcher->doc(docId0);
  wstring storedField = doc[FIELD_NAME];

  shared_ptr<TokenStream> stream = getAnyTokenStream(FIELD_NAME, docId0);
  shared_ptr<Fragmenter> fragmenter = make_shared<SimpleSpanFragmenter>(scorer);
  highlighter->setTextFragmenter(fragmenter);
  wstring fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"Hello this is a piece of text that is <B>very</B> <B>long</B> "
               L"and contains too much preamble and the meat is really here "
               L"which says kennedy has been shot",
               fragment);

  constexpr int docId1 = hits->scoreDocs[1]->doc;
  doc = searcher->doc(docId1);
  storedField = doc[FIELD_NAME];

  stream = getAnyTokenStream(FIELD_NAME, docId1);
  highlighter->setTextFragmenter(make_shared<SimpleSpanFragmenter>(scorer));
  fragment = highlighter->getBestFragment(stream, storedField);
  assertEquals(L"This piece of text refers to Kennedy at the beginning then "
               L"has a longer piece of text that is <B>very</B>",
               fragment);
}

HighlighterTest::QueryAnonymousInnerClass::QueryAnonymousInnerClass(
    shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Query> HighlighterTest::QueryAnonymousInnerClass::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<CommonTermsQuery> query =
      make_shared<CommonTermsQuery>(Occur::MUST, Occur::SHOULD, 3);
  query->add(make_shared<Term>(FIELD_NAME, L"this")); // stop-word
  query->add(make_shared<Term>(FIELD_NAME, L"long"));
  query->add(make_shared<Term>(FIELD_NAME, L"very"));
  return query;
}

// C++ TODO: There is no native C++ equivalent to 'toString':
wstring
HighlighterTest::QueryAnonymousInnerClass::BaseTokenStreamTestCase::toString(
    const wstring &field)
{
  return L"";
}

int HighlighterTest::QueryAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HighlighterTest::QueryAnonymousInnerClass::equals(any obj)
{
  return obj == shared_from_this();
}

void HighlighterTest::testHighlightingWithDefaultField() 
{

  wstring s1 = L"I call our world Flatland, not because we call it so,";

  // Verify that a query against the default field results in text being
  // highlighted
  // regardless of the field name.

  shared_ptr<PhraseQuery> q =
      make_shared<PhraseQuery>(3, FIELD_NAME, L"world", L"flatland");

  wstring expected =
      L"I call our <B>world</B> <B>Flatland</B>, not because we call it so,";
  wstring observed = highlightField(q, L"SOME_FIELD_NAME", s1);
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Query in the default field results in text for *ANY* field "
               L"being highlighted",
               expected, observed);

  // Verify that a query against a named field does not result in any
  // highlighting
  // when the query field name differs from the name of the field being
  // highlighted,
  // which in this example happens to be the default field name.
  q = make_shared<PhraseQuery>(3, L"text", L"world", L"flatland");

  expected = s1;
  observed = highlightField(q, FIELD_NAME, s1);
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Query in a named field does not result in highlighting when "
               L"that field isn't in the query",
               s1, highlightField(q, FIELD_NAME, s1));
}

wstring HighlighterTest::highlightField(
    shared_ptr<Query> query, const wstring &fieldName,
    const wstring &text) 
{
  shared_ptr<TokenStream> tokenStream = analyzer->tokenStream(fieldName, text);
  // Assuming "<B>", "</B>" used to highlight
  shared_ptr<SimpleHTMLFormatter> formatter =
      make_shared<SimpleHTMLFormatter>();
  shared_ptr<QueryScorer> scorer =
      make_shared<QueryScorer>(query, fieldName, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(formatter, scorer);
  highlighter->setTextFragmenter(
      make_shared<SimpleFragmenter>(numeric_limits<int>::max()));

  wstring rv = highlighter->getBestFragments(tokenStream, text, 1,
                                             L"(FIELD TEXT TRUNCATED)");
  return rv.length() == 0 ? text : rv;
}

void HighlighterTest::testSimpleSpanHighlighter() 
{
  doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")));

  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    wstring text = searcher->doc(hits->scoreDocs[i]->doc)[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        analyzer->tokenStream(FIELD_NAME, text);
    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  // Not sure we can assert anything here - just running to check we dont
  // throw any exceptions
}

void HighlighterTest::
    testSimpleSpanHighlighterWithStopWordsStraddlingFragmentBoundaries() throw(
        runtime_error)
{
  doSearching(make_shared<PhraseQuery>(FIELD_NAME, L"all", L"tokens"));

  int maxNumFragmentsRequired = 1;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(scorer);

  assertEquals(L"Must have one hit", 1, hits->totalHits);
  for (int i = 0; i < hits->totalHits; i++) {
    wstring text = searcher->doc(hits->scoreDocs[i]->doc)[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        analyzer->tokenStream(FIELD_NAME, text);

    highlighter->setTextFragmenter(
        make_shared<SimpleSpanFragmenter>(scorer, 36));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }

    assertTrue(L"Fragment must be less than 60 characters long",
               result.length() < 60);
  }
}

void HighlighterTest::testRepeatingTermsInMultBooleans() 
{
  wstring content = L"x y z a b c d e f g b c g";
  wstring f1 = L"f1";
  wstring f2 = L"f2";

  shared_ptr<PhraseQuery> f1ph1 =
      make_shared<PhraseQuery>(f1, L"a", L"b", L"c", L"d");

  shared_ptr<PhraseQuery> f2ph1 =
      make_shared<PhraseQuery>(f2, L"a", L"b", L"c", L"d");

  shared_ptr<PhraseQuery> f1ph2 =
      make_shared<PhraseQuery>(f1, L"b", L"c", L"g");

  shared_ptr<PhraseQuery> f2ph2 =
      make_shared<PhraseQuery>(f2, L"b", L"c", L"g");

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> leftChild =
      make_shared<BooleanQuery::Builder>();
  leftChild->add(f1ph1, Occur::SHOULD);
  leftChild->add(f2ph1, Occur::SHOULD);
  booleanQuery->add(leftChild->build(), Occur::MUST);

  shared_ptr<BooleanQuery::Builder> rightChild =
      make_shared<BooleanQuery::Builder>();
  rightChild->add(f1ph2, Occur::SHOULD);
  rightChild->add(f2ph2, Occur::SHOULD);
  booleanQuery->add(rightChild->build(), Occur::MUST);

  shared_ptr<QueryScorer> scorer =
      make_shared<QueryScorer>(booleanQuery->build(), f1);
  scorer->setExpandMultiTermQuery(false);

  shared_ptr<Highlighter> h =
      make_shared<Highlighter>(shared_from_this(), scorer);

  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);

  h->getBestFragment(analyzer, f1, content);

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 7);
}

void HighlighterTest::testSimpleQueryScorerPhraseHighlighting() throw(
    runtime_error)
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(FIELD_NAME, L"very"), 0);
  builder->add(make_shared<Term>(FIELD_NAME, L"long"), 1);
  builder->add(make_shared<Term>(FIELD_NAME, L"contains"), 3);
  shared_ptr<PhraseQuery> phraseQuery = builder->build();
  doSearching(phraseQuery);

  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 3);

  numHighlights = 0;

  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(FIELD_NAME, L"piece"), 1);
  builder->add(make_shared<Term>(FIELD_NAME, L"text"), 3);
  builder->add(make_shared<Term>(FIELD_NAME, L"refers"), 4);
  builder->add(make_shared<Term>(FIELD_NAME, L"kennedy"), 6);
  phraseQuery = builder->build();

  doSearching(phraseQuery);

  maxNumFragmentsRequired = 2;

  scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  highlighter = make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 4);

  numHighlights = 0;

  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(FIELD_NAME, L"lets"), 0);
  builder->add(make_shared<Term>(FIELD_NAME, L"lets"), 4);
  builder->add(make_shared<Term>(FIELD_NAME, L"lets"), 8);
  builder->add(make_shared<Term>(FIELD_NAME, L"lets"), 12);
  phraseQuery = builder->build();

  doSearching(phraseQuery);

  maxNumFragmentsRequired = 2;

  scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  highlighter = make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 4);
}

void HighlighterTest::testSpanRegexQuery() 
{
  query = make_shared<SpanOrQuery>(make_shared<SpanMultiTermQueryWrapper<>>(
      make_shared<RegexpQuery>(make_shared<Term>(FIELD_NAME, L"ken.*"))));
  searcher = newSearcher(reader);
  hits = searcher->search(query, 100);
  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 5);
}

void HighlighterTest::testRegexQuery() 
{
  query = make_shared<RegexpQuery>(make_shared<Term>(FIELD_NAME, L"ken.*"));
  searcher = newSearcher(reader);
  hits = searcher->search(query, 100);
  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 5);
}

void HighlighterTest::testExternalReader() 
{
  query = make_shared<RegexpQuery>(make_shared<Term>(FIELD_NAME, L"ken.*"));
  searcher = newSearcher(reader);
  hits = searcher->search(query, 100);
  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer =
      make_shared<QueryScorer>(query, reader, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(reader->docFreq(make_shared<Term>(FIELD_NAME, L"hello")) > 0);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 5);
}

void HighlighterTest::testDimensionalRangeQuery() 
{
  // doesn't currently highlight, but make sure it doesn't cause exception
  // either
  query = IntPoint::newRangeQuery(NUMERIC_FIELD_NAME, 2, 6);
  searcher = newSearcher(reader);
  hits = searcher->search(query, 100);
  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring text = searcher->doc(hits->scoreDocs[i]->doc)
                       ->getField(NUMERIC_FIELD_NAME)
                       ->numericValue()
                       ->toString();
    shared_ptr<TokenStream> tokenStream =
        analyzer->tokenStream(FIELD_NAME, text);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    //      std::wstring result =
    highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired,
                                  L"...");
    // if (VERBOSE) System.out.println("\t" + result);
  }
}

void HighlighterTest::testToParentBlockJoinQuery() 
{
  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"parent")));

  query = make_shared<ToParentBlockJoinQuery>(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"child")),
      parentFilter, ScoreMode::None);
  searcher = newSearcher(reader);
  hits = searcher->search(query, 100);
  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    wstring text = L"child document";
    shared_ptr<TokenStream> tokenStream =
        analyzer->tokenStream(FIELD_NAME, text);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));
    highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired,
                                  L"...");
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 1);
}

void HighlighterTest::testToChildBlockJoinQuery() 
{
  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"parent")));

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(
      make_shared<ToChildBlockJoinQuery>(
          make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"parent")),
          parentFilter),
      Occur::MUST);
  booleanQuery->add(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"child")),
      Occur::MUST);
  query = booleanQuery->build();

  searcher = newSearcher(reader);
  hits = searcher->search(query, 100);
  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    wstring text = L"parent document";
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));
    highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired,
                                  L"...");
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 1);
}

void HighlighterTest::testSimpleQueryScorerPhraseHighlighting2() throw(
    runtime_error)
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(5, FIELD_NAME, L"text", L"piece", L"long");
  doSearching(phraseQuery);

  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);
  highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 6);
}

void HighlighterTest::testSimpleQueryScorerPhraseHighlighting3() throw(
    runtime_error)
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(FIELD_NAME, L"x", L"y", L"z");
  doSearching(phraseQuery);

  int maxNumFragmentsRequired = 2;

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);
    shared_ptr<QueryScorer> scorer =
        make_shared<QueryScorer>(query, FIELD_NAME);
    shared_ptr<Highlighter> highlighter =
        make_shared<Highlighter>(shared_from_this(), scorer);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }

    assertTrue(L"Failed to find correct number of highlights " +
                   to_wstring(numHighlights) + L" found",
               numHighlights == 3);
  }
}

void HighlighterTest::testSimpleSpanFragmenter() 
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(FIELD_NAME, L"piece"), 0);
  builder->add(make_shared<Term>(FIELD_NAME, L"text"), 2);
  builder->add(make_shared<Term>(FIELD_NAME, L"very"), 5);
  builder->add(make_shared<Term>(FIELD_NAME, L"long"), 6);
  shared_ptr<PhraseQuery> phraseQuery = builder->build();
  doSearching(phraseQuery);

  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(
        make_shared<SimpleSpanFragmenter>(scorer, 5));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }

  phraseQuery = make_shared<PhraseQuery>(FIELD_NAME, L"been", L"shot");

  doSearching(query);

  maxNumFragmentsRequired = 2;

  scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  highlighter = make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    wstring text = searcher->doc(hits->scoreDocs[i]->doc)[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        analyzer->tokenStream(FIELD_NAME, text);

    highlighter->setTextFragmenter(
        make_shared<SimpleSpanFragmenter>(scorer, 20));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
}

void HighlighterTest::testPosTermStdTerm() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"y")),
                    Occur::SHOULD);

  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(FIELD_NAME, L"x", L"y", L"z");
  booleanQuery->add(phraseQuery, Occur::SHOULD);

  doSearching(booleanQuery->build());

  int maxNumFragmentsRequired = 2;

  shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(shared_from_this(), scorer);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }

    assertTrue(L"Failed to find correct number of highlights " +
                   to_wstring(numHighlights) + L" found",
               numHighlights == 4);
  }
}

void HighlighterTest::testQueryScorerMultiPhraseQueryHighlighting() throw(
    runtime_error)
{
  shared_ptr<MultiPhraseQuery::Builder> mpqb =
      make_shared<MultiPhraseQuery::Builder>();

  mpqb->add(std::deque<std::shared_ptr<Term>>{
      make_shared<Term>(FIELD_NAME, L"wordx"),
      make_shared<Term>(FIELD_NAME, L"wordb")});
  mpqb->add(make_shared<Term>(FIELD_NAME, L"wordy"));

  doSearching(mpqb->build());

  constexpr int maxNumFragmentsRequired = 2;
  assertExpectedHighlightCount(maxNumFragmentsRequired, 6);
}

void HighlighterTest::
    testQueryScorerMultiPhraseQueryHighlightingWithGap() 
{
  shared_ptr<MultiPhraseQuery::Builder> mpqb =
      make_shared<MultiPhraseQuery::Builder>();

  /*
   * The toString of MultiPhraseQuery doesn't work so well with these
   * out-of-order additions, but the Query itself seems to match accurately.
   */

  mpqb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(FIELD_NAME,
                                                                 L"wordz")},
            2);
  mpqb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(FIELD_NAME,
                                                                 L"wordx")},
            0);

  doSearching(mpqb->build());

  constexpr int maxNumFragmentsRequired = 1;
  constexpr int expectedHighlights = 2;

  assertExpectedHighlightCount(maxNumFragmentsRequired, expectedHighlights);
}

void HighlighterTest::testNearSpanSimpleQuery() 
{
  doSearching(make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(
              make_shared<Term>(FIELD_NAME, L"beginning")),
          make_shared<SpanTermQuery>(
              make_shared<Term>(FIELD_NAME, L"kennedy"))},
      3, false));

  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass>(shared_from_this());

  helper->run();

  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 2);
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass::
    TestHighlightRunnerAnonymousInnerClass(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass::run() throw(
    runtime_error)
{
  mode = TestHighlightRunner::QUERY;
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
}

void HighlighterTest::testSimpleQueryTermScorerHighlighter() throw(
    runtime_error)
{
  doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")));
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(make_shared<QueryTermScorer>(query));
  highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));
  int maxNumFragmentsRequired = 2;
  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
  // Not sure we can assert anything here - just running to check we dont
  // throw any exceptions
}

void HighlighterTest::testSpanHighlighting() 
{
  shared_ptr<Query> query1 = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"wordx")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"wordy"))},
      1, false);
  shared_ptr<Query> query2 = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"wordy")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"wordc"))},
      1, false);
  shared_ptr<BooleanQuery::Builder> bquery =
      make_shared<BooleanQuery::Builder>();
  bquery->add(query1, Occur::SHOULD);
  bquery->add(query2, Occur::SHOULD);
  doSearching(bquery->build());
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass2>(shared_from_this());

  helper->run();
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 7);
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass2::
    TestHighlightRunnerAnonymousInnerClass2(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass2::run() throw(
    runtime_error)
{
  mode = TestHighlightRunner::QUERY;
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
}

void HighlighterTest::testNotSpanSimpleQuery() 
{
  doSearching(make_shared<SpanNotQuery>(
      make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              make_shared<SpanTermQuery>(
                  make_shared<Term>(FIELD_NAME, L"shot")),
              make_shared<SpanTermQuery>(
                  make_shared<Term>(FIELD_NAME, L"kennedy"))},
          3, false),
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"john"))));
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass3>(shared_from_this());

  helper->run();
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 4);
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass3::
    TestHighlightRunnerAnonymousInnerClass3(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass3::run() throw(
    runtime_error)
{
  mode = TestHighlightRunner::QUERY;
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
}

void HighlighterTest::testGetBestFragmentsSimpleQuery() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass4>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass4::
    TestHighlightRunnerAnonymousInnerClass4(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass4::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  outerInstance->doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")));
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);
}

void HighlighterTest::testGetBestFragmentsConstantScore() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass5>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass5::
    TestHighlightRunnerAnonymousInnerClass5(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass5::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  if (LuceneTestCase::random()->nextBoolean()) {
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    bq->add(make_shared<ConstantScoreQuery>(make_shared<TermQuery>(
                make_shared<Term>(FIELD_NAME, L"kennedy"))),
            Occur::MUST);
    bq->add(make_shared<ConstantScoreQuery>(make_shared<TermQuery>(
                make_shared<Term>(FIELD_NAME, L"kennedy"))),
            Occur::MUST);
    outerInstance->doSearching(bq->build());
  } else {
    outerInstance->doSearching(make_shared<ConstantScoreQuery>(
        make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy"))));
  }
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);
}

void HighlighterTest::testGetFuzzyFragments() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass6>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass6::
    TestHighlightRunnerAnonymousInnerClass6(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass6::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  shared_ptr<FuzzyQuery> fuzzyQuery =
      make_shared<FuzzyQuery>(make_shared<Term>(FIELD_NAME, L"kinnedy"), 2);
  fuzzyQuery->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  outerInstance->doSearching(fuzzyQuery);
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query, outerInstance,
                       true);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);
}

void HighlighterTest::testGetWildCardFragments() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass7>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass7::
    TestHighlightRunnerAnonymousInnerClass7(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass7::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  shared_ptr<WildcardQuery> wildcardQuery =
      make_shared<WildcardQuery>(make_shared<Term>(FIELD_NAME, L"k?nnedy"));
  wildcardQuery->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  outerInstance->doSearching(wildcardQuery);
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);
}

void HighlighterTest::testGetMidWildCardFragments() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass8>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass8::
    TestHighlightRunnerAnonymousInnerClass8(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass8::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  shared_ptr<WildcardQuery> wildcardQuery =
      make_shared<WildcardQuery>(make_shared<Term>(FIELD_NAME, L"k*dy"));
  wildcardQuery->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  outerInstance->doSearching(wildcardQuery);
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 5);
}

void HighlighterTest::testGetRangeFragments() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass9>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass9::
    TestHighlightRunnerAnonymousInnerClass9(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass9::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;

  // Need to explicitly set the QueryParser property to use TermRangeQuery
  // rather
  // than RangeFilters

  shared_ptr<TermRangeQuery> rangeQuery = make_shared<TermRangeQuery>(
      FIELD_NAME, make_shared<BytesRef>(L"kannedy"),
      make_shared<BytesRef>(L"kznnedy"), true, true);
  rangeQuery->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);

  outerInstance->query = rangeQuery;
  outerInstance->doSearching(outerInstance->query);

  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 5);
}

void HighlighterTest::testConstantScoreMultiTermQuery() 
{

  numHighlights = 0;

  query = make_shared<WildcardQuery>(make_shared<Term>(FIELD_NAME, L"ken*"));
  (std::static_pointer_cast<WildcardQuery>(query))
      ->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  searcher = newSearcher(reader);
  // can't rewrite ConstantScore if you want to highlight it -
  // it rewrites to ConstantScoreQuery which cannot be highlighted
  // query = unReWrittenQuery.rewrite(reader);
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Searching for: " << query->toString(FIELD_NAME) << endl;
  }
  hits = searcher->search(query, 1000);

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);
    int maxNumFragmentsRequired = 2;
    wstring fragmentSeparator = L"...";
    shared_ptr<QueryScorer> scorer =
        make_shared<QueryScorer>(query, HighlighterTest::FIELD_NAME);

    shared_ptr<Highlighter> highlighter =
        make_shared<Highlighter>(shared_from_this(), scorer);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(20));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, fragmentSeparator);
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 5);

  // try null field

  hits = searcher->search(query, 1000);

  numHighlights = 0;

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);
    int maxNumFragmentsRequired = 2;
    wstring fragmentSeparator = L"...";
    shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(query, nullptr);

    shared_ptr<Highlighter> highlighter =
        make_shared<Highlighter>(shared_from_this(), scorer);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(20));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, fragmentSeparator);
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 5);

  // try default field

  hits = searcher->search(query, 1000);

  numHighlights = 0;

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream = getAnyTokenStream(FIELD_NAME, docId);
    int maxNumFragmentsRequired = 2;
    wstring fragmentSeparator = L"...";
    shared_ptr<QueryScorer> scorer = make_shared<QueryScorer>(
        query, L"random_field", HighlighterTest::FIELD_NAME);

    shared_ptr<Highlighter> highlighter =
        make_shared<Highlighter>(shared_from_this(), scorer);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(20));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, fragmentSeparator);
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(numHighlights) + L" found",
             numHighlights == 5);
}

void HighlighterTest::testGetBestFragmentsPhrase() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass10>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass10::
    TestHighlightRunnerAnonymousInnerClass10(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass10::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(FIELD_NAME, L"john", L"kennedy");
  outerInstance->doSearching(phraseQuery);
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  // Currently highlights "John" and "Kennedy" separately
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 2);
}

void HighlighterTest::testGetBestFragmentsQueryScorer() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass11>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass11::
    TestHighlightRunnerAnonymousInnerClass11(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass11::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  std::deque<std::shared_ptr<SpanQuery>> clauses = {
      make_shared<SpanTermQuery>(make_shared<Term>(L"contents", L"john")),
      make_shared<SpanTermQuery>(make_shared<Term>(L"contents", L"kennedy"))};

  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(clauses, 1, true);
  outerInstance->doSearching(snq);
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  // Currently highlights "John" and "Kennedy" separately
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 2);
}

void HighlighterTest::testOffByOne() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass12>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass12::
    TestHighlightRunnerAnonymousInnerClass12(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass12::run() throw(
    runtime_error)
{
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(L"data", L"help"));
  shared_ptr<Highlighter> hg = make_shared<Highlighter>(
      make_shared<SimpleHTMLFormatter>(), make_shared<QueryTermScorer>(query));
  hg->setTextFragmenter(make_shared<NullFragmenter>());

  wstring match =
      hg->getBestFragment(outerInstance->analyzer, L"data", L"help me [54-65]");
  assertEquals(L"<B>help</B> me [54-65]", match);
}

void HighlighterTest::testGetBestFragmentsFilteredQuery() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass13>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass13::
    TestHighlightRunnerAnonymousInnerClass13(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass13::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  std::deque<std::shared_ptr<SpanQuery>> clauses = {
      make_shared<SpanTermQuery>(make_shared<Term>(L"contents", L"john")),
      make_shared<SpanTermQuery>(make_shared<Term>(L"contents", L"kennedy"))};
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(clauses, 1, true);
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(snq, Occur::MUST);
  bq->add(
      TermRangeQuery::newStringRange(L"contents", L"john", L"john", true, true),
      Occur::FILTER);

  outerInstance->doSearching(bq->build());
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  // Currently highlights "John" and "Kennedy" separately
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 2);
}

void HighlighterTest::testGetBestFragmentsFilteredPhraseQuery() throw(
    runtime_error)
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass14>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass14::
    TestHighlightRunnerAnonymousInnerClass14(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass14::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"contents", L"john", L"kennedy");
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(pq, Occur::MUST);
  bq->add(
      TermRangeQuery::newStringRange(L"contents", L"john", L"john", true, true),
      Occur::FILTER);

  outerInstance->doSearching(bq->build());
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  // Currently highlights "John" and "Kennedy" separately
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 2);
}

void HighlighterTest::testGetBestFragmentsMultiTerm() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass15>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass15::
    TestHighlightRunnerAnonymousInnerClass15(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass15::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"john")),
      Occur::SHOULD);
  shared_ptr<PrefixQuery> prefixQuery =
      make_shared<PrefixQuery>(make_shared<Term>(FIELD_NAME, L"kenn"));
  prefixQuery->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  booleanQuery->add(prefixQuery, Occur::SHOULD);

  outerInstance->doSearching(booleanQuery->build());
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, outerInstance->query,
                       outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 5);
}

void HighlighterTest::testGetBestFragmentsWithOr() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass16>(shared_from_this());
  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass16::
    TestHighlightRunnerAnonymousInnerClass16(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass16::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"jfk")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")),
             Occur::SHOULD);

  outerInstance->doSearching(query->build());
  doStandardHighlights(outerInstance->analyzer, outerInstance->searcher,
                       outerInstance->hits, query->build(), outerInstance);
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 5);
}

void HighlighterTest::testGetBestSingleFragment() 
{

  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass17>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass17::
    TestHighlightRunnerAnonymousInnerClass17(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass17::run() throw(
    runtime_error)
{
  outerInstance->doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")));
  outerInstance->numHighlights = 0;
  for (int i = 0; i < outerInstance->hits->totalHits; i++) {
    constexpr int docId = outerInstance->hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = outerInstance->searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        outerInstance->getAnyTokenStream(FIELD_NAME, docId);

    shared_ptr<Highlighter> highlighter =
        getHighlighter(outerInstance->query, FIELD_NAME, outerInstance);
    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));
    wstring result = highlighter->getBestFragment(tokenStream, text);
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);

  outerInstance->numHighlights = 0;
  for (int i = 0; i < outerInstance->hits->totalHits; i++) {
    constexpr int docId = outerInstance->hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = outerInstance->searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        outerInstance->getAnyTokenStream(FIELD_NAME, docId);
    shared_ptr<Highlighter> highlighter =
        getHighlighter(outerInstance->query, FIELD_NAME, outerInstance);
    highlighter->getBestFragment(tokenStream, text);
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);

  outerInstance->numHighlights = 0;
  for (int i = 0; i < outerInstance->hits->totalHits; i++) {
    constexpr int docId = outerInstance->hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = outerInstance->searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        outerInstance->getAnyTokenStream(FIELD_NAME, docId);

    shared_ptr<Highlighter> highlighter =
        getHighlighter(outerInstance->query, FIELD_NAME, outerInstance);
    highlighter->getBestFragments(tokenStream, text, 10);
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);
}

void HighlighterTest::testNotRewriteMultiTermQuery() 
{
  // field "bar": (not the field we ultimately want to extract)
  shared_ptr<MultiTermQuery> mtq =
      make_shared<TermRangeQuery>(L"bar", make_shared<BytesRef>(L"aa"),
                                  make_shared<BytesRef>(L"zz"), true, true);
  shared_ptr<WeightedSpanTermExtractor> extractor =
      make_shared<WeightedSpanTermExtractorAnonymousInnerClass>(
          shared_from_this(), mtq);
  extractor->setExpandMultiTermQuery(true);
  extractor->setMaxDocCharsToAnalyze(51200);
  extractor->getWeightedSpanTerms(
      mtq, 3,
      make_shared<CannedTokenStream>(make_shared<Token>(L"aa", 0, 2),
                                     make_shared<Token>(L"bb", 2, 4)),
      L"foo"); // field "foo"
}

HighlighterTest::WeightedSpanTermExtractorAnonymousInnerClass::
    WeightedSpanTermExtractorAnonymousInnerClass(
        shared_ptr<HighlighterTest> outerInstance,
        shared_ptr<MultiTermQuery> mtq)
{
  this->outerInstance = outerInstance;
  this->mtq = mtq;
}

void HighlighterTest::WeightedSpanTermExtractorAnonymousInnerClass::extract(
    shared_ptr<Query> query, float boost,
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
        &terms) 
{
  assertEquals(mtq, query);
  outerInstance->super->extract(query, boost, terms);
}

void HighlighterTest::testGetBestSingleFragmentWithWeights() throw(
    runtime_error)
{

  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass18>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass18::
    TestHighlightRunnerAnonymousInnerClass18(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass18::run() throw(
    runtime_error)
{
  std::deque<std::shared_ptr<WeightedSpanTerm>> wTerms(2);
  wTerms[0] = make_shared<WeightedSpanTerm>(10.0f, L"hello");

  deque<std::shared_ptr<PositionSpan>> positionSpans =
      deque<std::shared_ptr<PositionSpan>>();
  positionSpans.push_back(make_shared<PositionSpan>(0, 0));
  wTerms[0]->addPositionSpans(positionSpans);

  wTerms[1] = make_shared<WeightedSpanTerm>(1.0f, L"kennedy");
  positionSpans = deque<>();
  positionSpans.push_back(make_shared<PositionSpan>(14, 14));
  wTerms[1]->addPositionSpans(positionSpans);

  shared_ptr<Highlighter> highlighter =
      getHighlighter(wTerms, outerInstance); // new
  // Highlighter(new
  // QueryTermScorer(wTerms));
  shared_ptr<TokenStream> tokenStream =
      outerInstance->analyzer->tokenStream(FIELD_NAME, outerInstance->texts[0]);
  highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(2));

  wstring result = StringHelper::trim(
      highlighter->getBestFragment(tokenStream, outerInstance->texts[0]));
  assertTrue(L"Failed to find best section using weighted terms. Found: [" +
                 result + L"]",
             L"<B>Hello</B>" == result);

  // readjust weights
  wTerms[1]->setWeight(50.0f);
  tokenStream =
      outerInstance->analyzer->tokenStream(FIELD_NAME, outerInstance->texts[0]);
  highlighter = getHighlighter(wTerms, outerInstance);
  highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(2));

  result = StringHelper::trim(
      highlighter->getBestFragment(tokenStream, outerInstance->texts[0]));
  assertTrue(L"Failed to find best section using weighted terms. Found: " +
                 result,
             L"<B>kennedy</B>" == result);
}

void HighlighterTest::testOverlapAnalyzer() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass19>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass19::
    TestHighlightRunnerAnonymousInnerClass19(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass19::run() throw(
    runtime_error)
{
  unordered_map<wstring, wstring> synonyms = unordered_map<wstring, wstring>();
  synonyms.emplace(L"football", L"soccer,footie");
  shared_ptr<Analyzer> analyzer = make_shared<SynonymAnalyzer>(synonyms);

  wstring s = L"football-soccer in the euro 2004 footie competition";

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(L"bookid", L"football")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"bookid", L"soccer")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"bookid", L"footie")),
             Occur::SHOULD);

  shared_ptr<Highlighter> highlighter =
      getHighlighter(query->build(), nullptr, outerInstance);

  // Get 3 best fragments and separate with a "..."
  shared_ptr<TokenStream> tokenStream = analyzer->tokenStream(L"", s);

  wstring result = highlighter->getBestFragments(tokenStream, s, 3, L"...");
  wstring expectedResult = L"<B>football</B>-<B>soccer</B> in the euro 2004 "
                           L"<B>footie</B> competition";
  assertTrue(L"overlapping analyzer should handle highlights OK, expected:" +
                 expectedResult + L" actual:" + result,
             expectedResult == result);
}

void HighlighterTest::testGetSimpleHighlight() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass20>(shared_from_this());
  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass20::
    TestHighlightRunnerAnonymousInnerClass20(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass20::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  outerInstance->doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")));
  // new Highlighter(HighlighterTest.this, new QueryTermScorer(query));

  for (int i = 0; i < outerInstance->hits->totalHits; i++) {
    wstring text = outerInstance->searcher->doc(
        outerInstance->hits->scoreDocs[i]->doc)[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        outerInstance->analyzer->tokenStream(FIELD_NAME, text);
    shared_ptr<Highlighter> highlighter =
        getHighlighter(outerInstance->query, FIELD_NAME, outerInstance);
    wstring result = highlighter->getBestFragment(tokenStream, text);
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 4);
}

void HighlighterTest::testGetTextFragments() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass21>(shared_from_this());
  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass21::
    TestHighlightRunnerAnonymousInnerClass21(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass21::run() throw(
    runtime_error)
{

  outerInstance->doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"kennedy")));

  for (int i = 0; i < outerInstance->hits->totalHits; i++) {
    constexpr int docId = outerInstance->hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = outerInstance->searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        outerInstance->getAnyTokenStream(FIELD_NAME, docId);

    shared_ptr<Highlighter> highlighter =
        getHighlighter(outerInstance->query, FIELD_NAME,
                       outerInstance); // new Highlighter(this, new
    // QueryTermScorer(query));
    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(20));
    std::deque<wstring> stringResults =
        highlighter->getBestFragments(tokenStream, text, 10);

    tokenStream = outerInstance->analyzer->tokenStream(FIELD_NAME, text);
    std::deque<std::shared_ptr<TextFragment>> fragmentResults =
        highlighter->getBestTextFragments(tokenStream, text, true, 10);

    assertTrue(L"Failed to find correct number of text Fragments: " +
                   fragmentResults.size() + L" vs " + stringResults.size(),
               fragmentResults.size() == stringResults.size());
    for (int j = 0; j < stringResults.size(); j++) {
      if (VERBOSE) {
        wcout << fragmentResults[j] << endl;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertTrue(L"Failed to find same text Fragments: " + fragmentResults[j] +
                     L" found",
                 fragmentResults[j]->toString() == stringResults[j]);
    }
  }
}

void HighlighterTest::testMaxSizeHighlight() 
{
  // we disable MockTokenizer checks because we will forcefully limit the
  // tokenstream and call end() before incrementToken() returns false.
  // But we first need to clear the re-used tokenstream components that have
  // enableChecks.
  analyzer->getReuseStrategy()->setReusableComponents(analyzer, FIELD_NAME,
                                                      nullptr);
  analyzer->setEnableChecks(false);
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass22>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass22::
    TestHighlightRunnerAnonymousInnerClass22(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass22::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  outerInstance->doSearching(
      make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"meat")));
  shared_ptr<TokenStream> tokenStream =
      outerInstance->analyzer->tokenStream(FIELD_NAME, outerInstance->texts[0]);
  shared_ptr<Highlighter> highlighter =
      getHighlighter(outerInstance->query, FIELD_NAME,
                     outerInstance); // new Highlighter(this, new
  // QueryTermScorer(query));
  highlighter->setMaxDocCharsToAnalyze(30);

  highlighter->getBestFragment(tokenStream, outerInstance->texts[0]);
  assertTrue(wstring(L"Setting MaxDocBytesToAnalyze should have prevented ") +
                 L"us from finding matches for this record: " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 0);
}

void HighlighterTest::testMaxSizeHighlightTruncates() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass23>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass23::
    TestHighlightRunnerAnonymousInnerClass23(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass23::run() throw(
    runtime_error)
{
  wstring goodWord = L"goodtoken";
  shared_ptr<CharacterRunAutomaton> stopWords =
      make_shared<CharacterRunAutomaton>(Automata::makeString(L"stoppedtoken"));
  // we disable MockTokenizer checks because we will forcefully limit the
  // tokenstream and call end() before incrementToken() returns false.
  shared_ptr<MockAnalyzer> *const analyzer = make_shared<MockAnalyzer>(
      LuceneTestCase::random(), MockTokenizer::SIMPLE, true, stopWords);
  analyzer->setEnableChecks(false);
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(L"data", goodWord));

  wstring match;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(goodWord);
  for (int i = 0; i < 10000; i++) {
    sb->append(L" ");
    // only one stopword
    sb->append(L"stoppedtoken");
  }
  shared_ptr<SimpleHTMLFormatter> fm = make_shared<SimpleHTMLFormatter>();
  shared_ptr<Highlighter> hg =
      getHighlighter(query, L"data", fm); // new Highlighter(fm,
  // new
  // QueryTermScorer(query));
  hg->setTextFragmenter(make_shared<NullFragmenter>());
  hg->setMaxDocCharsToAnalyze(100);
  match = hg->getBestFragment(analyzer, L"data", sb->toString());
  assertTrue(L"Matched text should be no more than 100 chars in length ",
             match.length() < hg->getMaxDocCharsToAnalyze());

  // add another tokenized word to the overrall length - but set way
  // beyond
  // the length of text under consideration (after a large slug of stop
  // words
  // + whitespace)
  sb->append(L" ");
  sb->append(goodWord);
  match = hg->getBestFragment(analyzer, L"data", sb->toString());
  assertTrue(L"Matched text should be no more than 100 chars in length ",
             match.length() < hg->getMaxDocCharsToAnalyze());
}

void HighlighterTest::testMaxSizeEndHighlight() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass24>(shared_from_this());
  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass24::
    TestHighlightRunnerAnonymousInnerClass24(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass24::run() throw(
    runtime_error)
{
  shared_ptr<CharacterRunAutomaton> stopWords =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"i[nt]"))->toAutomaton());
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(L"text", L"searchterm"));

  wstring text = L"this is a text with searchterm in it";
  shared_ptr<SimpleHTMLFormatter> fm = make_shared<SimpleHTMLFormatter>();
  shared_ptr<Highlighter> hg = getHighlighter(query, L"text", fm);
  hg->setTextFragmenter(make_shared<NullFragmenter>());
  hg->setMaxDocCharsToAnalyze(36);
  wstring match = hg->getBestFragment(
      make_shared<MockAnalyzer>(LuceneTestCase::random(), MockTokenizer::SIMPLE,
                                true, stopWords),
      L"text", text);
  assertTrue(
      L"Matched text should contain remainder of text after highlighted query ",
      StringHelper::endsWith(match, L"in it"));
}

void HighlighterTest::testHighlighterWithPhraseQuery() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring fieldName = L"substring";

  shared_ptr<PhraseQuery> *const query = make_shared<PhraseQuery>(
      fieldName,
      std::deque<std::shared_ptr<BytesRef>>{make_shared<BytesRef>(L"uchu")});

  assertHighlighting(query, make_shared<SimpleHTMLFormatter>(L"<b>", L"</b>"),
                     L"Buchung", L"B<b>uchu</b>ng", fieldName);
}

void HighlighterTest::testHighlighterWithMultiPhraseQuery() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring fieldName = L"substring";

  shared_ptr<MultiPhraseQuery> *const mpq =
      (make_shared<MultiPhraseQuery::Builder>())
          ->add(make_shared<Term>(fieldName, L"uchu"))
          ->build();

  assertHighlighting(mpq, make_shared<SimpleHTMLFormatter>(L"<b>", L"</b>"),
                     L"Buchung", L"B<b>uchu</b>ng", fieldName);
}

void HighlighterTest::assertHighlighting(
    shared_ptr<Query> query, shared_ptr<Formatter> formatter,
    const wstring &text, const wstring &expected,
    const wstring &fieldName) 
{
  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), fieldName);

  shared_ptr<QueryScorer> *const fragmentScorer =
      make_shared<QueryScorer>(query, fieldName);

  shared_ptr<Highlighter> *const highlighter =
      make_shared<Highlighter>(formatter, fragmentScorer);
  highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(100));
  const wstring fragment =
      highlighter->getBestFragment(analyzer, fieldName, text);

  assertEquals(expected, fragment);
}

HighlighterTest::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<HighlighterTest> outerInstance, const wstring &fieldName)
{
  this->outerInstance = outerInstance;
  this->fieldName = fieldName;
}

shared_ptr<Analyzer::TokenStreamComponents>
HighlighterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<NGramTokenizer>(4, 4));
}

void HighlighterTest::testUnRewrittenQuery() 
{
  shared_ptr<TestHighlightRunner> *const helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass25>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass25::
    TestHighlightRunnerAnonymousInnerClass25(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass25::run() throw(
    runtime_error)
{
  outerInstance->numHighlights = 0;
  // test to show how rewritten query can still be used
  outerInstance->searcher = LuceneTestCase::newSearcher(outerInstance->reader);

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<WildcardQuery>(make_shared<Term>(FIELD_NAME, L"jf?")),
             Occur::SHOULD);
  query->add(
      make_shared<WildcardQuery>(make_shared<Term>(FIELD_NAME, L"kenned*")),
      Occur::SHOULD);

  if (VERBOSE) {
    wcout << L"Searching with primitive query" << endl;
  }
  // forget to set this and...
  // query=query.rewrite(reader);
  shared_ptr<TopDocs> hits =
      outerInstance->searcher->search(query->build(), 1000);

  // create an instance of the highlighter with the tags used to surround
  // highlighted text
  // QueryHighlightExtractor highlighter = new
  // QueryHighlightExtractor(this,
  // query, new StandardAnalyzer(TEST_VERSION));

  int maxNumFragmentsRequired = 3;

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = outerInstance->searcher->doc(docId);
    wstring text = doc[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        outerInstance->getAnyTokenStream(FIELD_NAME, docId);
    shared_ptr<Highlighter> highlighter =
        getHighlighter(query->build(), FIELD_NAME, outerInstance, false);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring highlightedText = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");

    if (VERBOSE) {
      wcout << highlightedText << endl;
    }
  }
  // We expect to have zero highlights if the query is multi-terms and is
  // not
  // rewritten!
  assertTrue(L"Failed to find correct number of highlights " +
                 to_wstring(outerInstance->numHighlights) + L" found",
             outerInstance->numHighlights == 0);
}

void HighlighterTest::testNoFragments() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass26>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass26::
    TestHighlightRunnerAnonymousInnerClass26(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass26::run() throw(
    runtime_error)
{
  outerInstance->doSearching(make_shared<TermQuery>(make_shared<Term>(
      FIELD_NAME, L"aninvalidquerywhichshouldyieldnoresults")));

  for (auto text : outerInstance->texts) {
    shared_ptr<TokenStream> tokenStream =
        outerInstance->analyzer->tokenStream(FIELD_NAME, text);
    shared_ptr<Highlighter> highlighter =
        getHighlighter(outerInstance->query, FIELD_NAME, outerInstance);
    wstring result = highlighter->getBestFragment(tokenStream, text);
    assertNull(
        L"The highlight result should be null for text with no query terms",
        result);
  }
}

void HighlighterTest::testEncoding() 
{

  wstring rawDocContent = L"\"Smith & sons' prices < 3 and >4\" claims article";
  // run the highlighter on the raw content (scorer does not score any tokens
  // for
  // highlighting but scores a single fragment for selection
  shared_ptr<Highlighter> highlighter = make_shared<Highlighter>(
      shared_from_this(), make_shared<SimpleHTMLEncoder>(),
      make_shared<ScorerAnonymousInnerClass>(shared_from_this()));
  highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(2000));
  shared_ptr<TokenStream> tokenStream =
      analyzer->tokenStream(FIELD_NAME, rawDocContent);

  wstring encodedSnippet =
      highlighter->getBestFragments(tokenStream, rawDocContent, 1, L"");
  // An ugly bit of XML creation:
  wstring xhtml = wstring(L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") +
                  L"<html xmlns=\"http://www.w3.org/1999/xhtml\" "
                  L"xml:lang=\"en\" lang=\"en\">\n" +
                  L"<head>\n" + L"<title>My Test HTML Document</title>\n" +
                  L"</head>\n" + L"<body>\n" + L"<h2>" + encodedSnippet +
                  L"</h2>\n" + L"</body>\n" + L"</html>";
  // now an ugly built of XML parsing to test the snippet is encoded OK
  shared_ptr<DocumentBuilderFactory> dbf =
      DocumentBuilderFactory::newInstance();
  shared_ptr<DocumentBuilder> db = dbf->newDocumentBuilder();
  shared_ptr<org::w3c::dom::Document> doc =
      db->parse(make_shared<ByteArrayInputStream>(
          xhtml.getBytes(StandardCharsets::UTF_8)));
  shared_ptr<Element> root = doc->getDocumentElement();
  shared_ptr<NodeList> nodes = root->getElementsByTagName(L"body");
  shared_ptr<Element> body = std::static_pointer_cast<Element>(nodes->item(0));
  nodes = body->getElementsByTagName(L"h2");
  shared_ptr<Element> h2 = std::static_pointer_cast<Element>(nodes->item(0));
  wstring decodedSnippet = h2->getFirstChild().getNodeValue();
  assertEquals(L"XHTML Encoding should have worked:", rawDocContent,
               decodedSnippet);
}

HighlighterTest::ScorerAnonymousInnerClass::ScorerAnonymousInnerClass(
    shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::ScorerAnonymousInnerClass::startFragment(
    shared_ptr<TextFragment> newFragment)
{
}

float HighlighterTest::ScorerAnonymousInnerClass::getTokenScore() { return 0; }

float HighlighterTest::ScorerAnonymousInnerClass::getFragmentScore()
{
  return 1;
}

shared_ptr<TokenStream> HighlighterTest::ScorerAnonymousInnerClass::init(
    shared_ptr<TokenStream> tokenStream)
{
  return nullptr;
}

void HighlighterTest::testFieldSpecificHighlighting() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass27>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass27::
    TestHighlightRunnerAnonymousInnerClass27(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass27::run() throw(
    runtime_error)
{
  wstring docMainText = L"fred is one of the people";

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD_NAME, L"fred")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"category", L"people")),
             Occur::SHOULD);

  // highlighting respects fieldnames used in query

  shared_ptr<Scorer> fieldSpecificScorer = nullptr;
  if (mode == TestHighlightRunner::QUERY) {
    fieldSpecificScorer = make_shared<QueryScorer>(query->build(), FIELD_NAME);
  } else if (mode == TestHighlightRunner::QUERY_TERM) {
    fieldSpecificScorer =
        make_shared<QueryTermScorer>(query->build(), L"contents");
  }
  shared_ptr<Highlighter> fieldSpecificHighlighter = make_shared<Highlighter>(
      make_shared<SimpleHTMLFormatter>(), fieldSpecificScorer);
  fieldSpecificHighlighter->setTextFragmenter(make_shared<NullFragmenter>());
  wstring result = fieldSpecificHighlighter->getBestFragment(
      outerInstance->analyzer, FIELD_NAME, docMainText);
  assertEquals(L"Should match", result, L"<B>fred</B> is one of the people");

  // highlighting does not respect fieldnames used in query
  shared_ptr<Scorer> fieldInSpecificScorer = nullptr;
  if (mode == TestHighlightRunner::QUERY) {
    fieldInSpecificScorer = make_shared<QueryScorer>(query->build(), nullptr);
  } else if (mode == TestHighlightRunner::QUERY_TERM) {
    fieldInSpecificScorer = make_shared<QueryTermScorer>(query->build());
  }

  shared_ptr<Highlighter> fieldInSpecificHighlighter = make_shared<Highlighter>(
      make_shared<SimpleHTMLFormatter>(), fieldInSpecificScorer);
  fieldInSpecificHighlighter->setTextFragmenter(make_shared<NullFragmenter>());
  result = fieldInSpecificHighlighter->getBestFragment(outerInstance->analyzer,
                                                       FIELD_NAME, docMainText);
  assertEquals(L"Should match", result,
               L"<B>fred</B> is one of the <B>people</B>");

  delete outerInstance->reader;
}

shared_ptr<TokenStream> HighlighterTest::getTS2()
{
  // std::wstring s = "Hi-Speed10 foo";
  return make_shared<TokenStreamAnonymousInnerClass>(shared_from_this());
}

HighlighterTest::TokenStreamAnonymousInnerClass::TokenStreamAnonymousInnerClass(
    shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);

  lst = deque<>();
  shared_ptr<Token> t;
  t = createToken(L"hi", 0, 2);
  t->setPositionIncrement(1);
  lst->add(t);
  t = createToken(L"hispeed", 0, 8);
  t->setPositionIncrement(1);
  lst->add(t);
  t = createToken(L"speed", 3, 8);
  t->setPositionIncrement(0);
  lst->add(t);
  t = createToken(L"10", 8, 10);
  t->setPositionIncrement(1);
  lst->add(t);
  t = createToken(L"foo", 11, 14);
  t->setPositionIncrement(1);
  lst->add(t);
  iter = lst::begin();
}

bool HighlighterTest::TokenStreamAnonymousInnerClass::incrementToken()
{
  if (iter->hasNext()) {
    shared_ptr<Token> token = iter->next();
    clearAttributes();
    termAtt::setEmpty()->append(token);
    posIncrAtt::setPositionIncrement(token->getPositionIncrement());
    offsetAtt::setOffset(token->startOffset(), token->endOffset());
    return true;
  }
  return false;
}

void HighlighterTest::TokenStreamAnonymousInnerClass::reset() 
{
  outerInstance->super->reset();
  iter = lst::begin();
}

shared_ptr<TokenStream> HighlighterTest::getTS2a()
{
  // std::wstring s = "Hi-Speed10 foo";
  return make_shared<TokenStreamAnonymousInnerClass2>(shared_from_this());
}

HighlighterTest::TokenStreamAnonymousInnerClass2::
    TokenStreamAnonymousInnerClass2(shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);

  lst = deque<>();
  shared_ptr<Token> t;
  t = createToken(L"hispeed", 0, 8);
  t->setPositionIncrement(1);
  lst->add(t);
  t = createToken(L"hi", 0, 2);
  t->setPositionIncrement(0);
  lst->add(t);
  t = createToken(L"speed", 3, 8);
  t->setPositionIncrement(1);
  lst->add(t);
  t = createToken(L"10", 8, 10);
  t->setPositionIncrement(1);
  lst->add(t);
  t = createToken(L"foo", 11, 14);
  t->setPositionIncrement(1);
  lst->add(t);
  iter = lst::begin();
}

bool HighlighterTest::TokenStreamAnonymousInnerClass2::incrementToken()
{
  if (iter->hasNext()) {
    shared_ptr<Token> token = iter->next();
    clearAttributes();
    termAtt::setEmpty()->append(token);
    posIncrAtt::setPositionIncrement(token->getPositionIncrement());
    offsetAtt::setOffset(token->startOffset(), token->endOffset());
    return true;
  }
  return false;
}

void HighlighterTest::TokenStreamAnonymousInnerClass2::reset() throw(
    IOException)
{
  outerInstance->super->reset();
  iter = lst::begin();
}

void HighlighterTest::testOverlapAnalyzer2() 
{
  shared_ptr<TestHighlightRunner> helper =
      make_shared<TestHighlightRunnerAnonymousInnerClass28>(shared_from_this());

  helper->start();
}

HighlighterTest::TestHighlightRunnerAnonymousInnerClass28::
    TestHighlightRunnerAnonymousInnerClass28(
        shared_ptr<HighlighterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlighterTest::TestHighlightRunnerAnonymousInnerClass28::run() throw(
    runtime_error)
{
  wstring s = L"Hi-Speed10 foo";

  shared_ptr<Query> query;
  shared_ptr<Highlighter> highlighter;
  wstring result;

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"foo"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result = highlighter->getBestFragments(outerInstance->getTS2(), s, 3, L"...");
  assertEquals(L"Hi-Speed10 <B>foo</B>", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"10"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result = highlighter->getBestFragments(outerInstance->getTS2(), s, 3, L"...");
  assertEquals(L"Hi-Speed<B>10</B> foo", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"hi"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result = highlighter->getBestFragments(outerInstance->getTS2(), s, 3, L"...");
  assertEquals(L"<B>Hi</B>-Speed10 foo", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"speed"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result = highlighter->getBestFragments(outerInstance->getTS2(), s, 3, L"...");
  assertEquals(L"Hi-<B>Speed</B>10 foo", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"hispeed"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result = highlighter->getBestFragments(outerInstance->getTS2(), s, 3, L"...");
  assertEquals(L"<B>Hi-Speed</B>10 foo", result);

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(L"text", L"hi")),
                    Occur::SHOULD);
  booleanQuery->add(
      make_shared<TermQuery>(make_shared<Term>(L"text", L"speed")),
      Occur::SHOULD);

  query = booleanQuery->build();
  highlighter = getHighlighter(query, L"text", outerInstance);
  result = highlighter->getBestFragments(outerInstance->getTS2(), s, 3, L"...");
  assertEquals(L"<B>Hi-Speed</B>10 foo", result);

  // ///////////////// same tests, just put the bigger overlapping token
  // first
  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"foo"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result =
      highlighter->getBestFragments(outerInstance->getTS2a(), s, 3, L"...");
  assertEquals(L"Hi-Speed10 <B>foo</B>", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"10"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result =
      highlighter->getBestFragments(outerInstance->getTS2a(), s, 3, L"...");
  assertEquals(L"Hi-Speed<B>10</B> foo", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"hi"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result =
      highlighter->getBestFragments(outerInstance->getTS2a(), s, 3, L"...");
  assertEquals(L"<B>Hi</B>-Speed10 foo", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"speed"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result =
      highlighter->getBestFragments(outerInstance->getTS2a(), s, 3, L"...");
  assertEquals(L"Hi-<B>Speed</B>10 foo", result);

  query = make_shared<TermQuery>(make_shared<Term>(L"text", L"hispeed"));
  highlighter = getHighlighter(query, L"text", outerInstance);
  result =
      highlighter->getBestFragments(outerInstance->getTS2a(), s, 3, L"...");
  assertEquals(L"<B>Hi-Speed</B>10 foo", result);

  query = booleanQuery->build();
  highlighter = getHighlighter(query, L"text", outerInstance);
  result =
      highlighter->getBestFragments(outerInstance->getTS2a(), s, 3, L"...");
  assertEquals(L"<B>Hi-Speed</B>10 foo", result);
}

void HighlighterTest::testWeightedTermsWithDeletes() throw(
    IOException, InvalidTokenOffsetsException)
{
  makeIndex();
  deleteDocument();
  searchIndex();
}

void HighlighterTest::makeIndex() 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(
               random(), MockTokenizer::WHITESPACE, false)));
  writer->addDocument(
      doc(L"t_text1", L"random words for highlighting tests del"));
  writer->addDocument(
      doc(L"t_text1", L"more random words for second field del"));
  writer->addDocument(
      doc(L"t_text1", L"random words for highlighting tests del"));
  writer->addDocument(doc(L"t_text1", L"more random words for second field"));
  writer->forceMerge(1);
  delete writer;
}

void HighlighterTest::deleteDocument() 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(
                random(), MockTokenizer::WHITESPACE, false)))
               ->setOpenMode(IndexWriterConfig::OpenMode::APPEND));
  writer->deleteDocuments({make_shared<Term>(L"t_text1", L"del")});
  // To see negative idf, keep comment the following line
  // writer.forceMerge(1);
  delete writer;
}

void HighlighterTest::searchIndex() throw(IOException,
                                          InvalidTokenOffsetsException)
{
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"t_text1", L"random"));
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  // This scorer can return negative idf -> null fragment
  shared_ptr<Scorer> scorer = make_shared<QueryTermScorer>(
      query, searcher->getIndexReader(), L"t_text1");
  // This scorer doesn't use idf (patch version)
  // Scorer scorer = new QueryTermScorer( query, "t_text1" );
  shared_ptr<Highlighter> h = make_shared<Highlighter>(scorer);

  shared_ptr<TopDocs> hits = searcher->search(query, 10);
  for (int i = 0; i < hits->totalHits; i++) {
    shared_ptr<Document> doc = searcher->doc(hits->scoreDocs[i]->doc);
    wstring result = h->getBestFragment(a, L"t_text1", doc[L"t_text1"]);
    if (VERBOSE) {
      wcout << L"result:" << result << endl;
    }
    assertEquals(L"more <B>random</B> words for second field", result);
  }
  delete reader;
}

void HighlighterTest::testPayloadQuery() throw(IOException,
                                               InvalidTokenOffsetsException)
{
  const wstring text = L"random words and words"; //"words" at positions 1 & 4

  shared_ptr<Analyzer> analyzer =
      make_shared<MockPayloadAnalyzer>(); // sets payload to "pos: X" (where X
                                          // is position #)
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexWriter writer =
  // new org.apache.lucene.index.IndexWriter(dir, new
  // org.apache.lucene.index.IndexWriterConfig(analyzer)))
  {
    org::apache::lucene::index::IndexWriter writer =
        org::apache::lucene::index::IndexWriter(
            dir, make_shared<org::apache::lucene::index::IndexWriterConfig>(
                     analyzer));
    writer->deleteAll();
    shared_ptr<Document> doc = make_shared<Document>();

    doc->push_back(make_shared<Field>(FIELD_NAME, text, fieldType));
    writer->addDocument(doc);
    writer->commit();
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader reader =
  // org.apache.lucene.index.DirectoryReader.open(dir))
  {
    org::apache::lucene::index::IndexReader reader =
        org::apache::lucene::index::DirectoryReader::open(dir);
    shared_ptr<Query> query = make_shared<SpanPayloadCheckQuery>(
        make_shared<SpanTermQuery>(make_shared<Term>(FIELD_NAME, L"words")),
        Collections::singletonList(make_shared<BytesRef>(
            L"pos: 1"))); // just match the first "word" occurrence
    shared_ptr<IndexSearcher> searcher = newSearcher(reader);
    shared_ptr<QueryScorer> scorer =
        make_shared<QueryScorer>(query, searcher->getIndexReader(), FIELD_NAME);
    scorer->setUsePayloads(true);
    shared_ptr<Highlighter> h = make_shared<Highlighter>(scorer);

    shared_ptr<TopDocs> hits = searcher->search(query, 10);
    assertEquals(1, hits->scoreDocs.size());
    shared_ptr<TokenStream> stream = TokenSources::getAnyTokenStream(
        searcher->getIndexReader(), 0, FIELD_NAME, analyzer);
    if (random()->nextBoolean()) {
      stream = make_shared<CachingTokenFilter>(
          stream); // conceals detection of TokenStreamFromTermVector
    }
    wstring result = h->getBestFragment(stream, text);
    assertEquals(L"random <B>words</B> and words",
                 result); // only highlight first "word"
  }
}

wstring HighlighterTest::highlightTerm(const wstring &originalText,
                                       shared_ptr<TokenGroup> group)
{
  if (group->getTotalScore() <= 0) {
    return originalText;
  }
  numHighlights++; // update stats used in assertions
  return L"<B>" + originalText + L"</B>";
}

void HighlighterTest::doSearching(shared_ptr<Query> unReWrittenQuery) throw(
    runtime_error)
{
  searcher = newSearcher(reader);
  // for any multi-term queries to work (prefix, wildcard, range,fuzzy etc)
  // you must use a rewritten query!
  query = unReWrittenQuery->rewrite(reader);
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Searching for: " << query->toString(FIELD_NAME) << endl;
  }
  hits = searcher->search(query, 1000);
}

void HighlighterTest::assertExpectedHighlightCount(
    int const maxNumFragmentsRequired,
    int const expectedHighlights) 
{
  for (int i = 0; i < hits->totalHits; i++) {
    wstring text = searcher->doc(hits->scoreDocs[i]->doc)[FIELD_NAME];
    shared_ptr<TokenStream> tokenStream =
        analyzer->tokenStream(FIELD_NAME, text);
    shared_ptr<QueryScorer> scorer =
        make_shared<QueryScorer>(query, FIELD_NAME);
    shared_ptr<Highlighter> highlighter =
        make_shared<Highlighter>(shared_from_this(), scorer);

    highlighter->setTextFragmenter(make_shared<SimpleFragmenter>(40));

    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, L"...");
    if (VERBOSE) {
      wcout << L"\t" << result << endl;
    }

    assertTrue(L"Failed to find correct number of highlights " +
                   to_wstring(numHighlights) + L" found",
               numHighlights == expectedHighlights);
  }
}

void HighlighterTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();

  // Not many use this setup:
  a = make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  dir = newDirectory();

  // Most tests use this setup:
  analyzer = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                       MockTokenFilter::ENGLISH_STOPSET);
  ramDir = newDirectory();
  fieldType = random()->nextBoolean() ? FIELD_TYPE_TV : TextField::TYPE_STORED;
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ramDir,
      newIndexWriterConfig(analyzer)->setMergePolicy(newLogMergePolicy()));

  for (auto text : texts) {
    writer->addDocument(doc(FIELD_NAME, text));
  }

  // a few tests need other docs...:
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(NUMERIC_FIELD_NAME, 1));
  doc->push_back(make_shared<StoredField>(NUMERIC_FIELD_NAME, 1));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(NUMERIC_FIELD_NAME, 3));
  doc->push_back(make_shared<StoredField>(NUMERIC_FIELD_NAME, 3));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(NUMERIC_FIELD_NAME, 5));
  doc->push_back(make_shared<StoredField>(NUMERIC_FIELD_NAME, 5));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(NUMERIC_FIELD_NAME, 7));
  doc->push_back(make_shared<StoredField>(NUMERIC_FIELD_NAME, 7));
  writer->addDocument(doc);

  shared_ptr<Document> childDoc = this->doc(FIELD_NAME, L"child document");
  shared_ptr<Document> parentDoc = this->doc(FIELD_NAME, L"parent document");
  writer->addDocuments(Arrays::asList(childDoc, parentDoc));

  writer->forceMerge(1);
  delete writer;
  reader = DirectoryReader::open(ramDir);

  // Misc:
  numHighlights = 0;
}

void HighlighterTest::tearDown() 
{
  delete reader;
  delete dir;
  delete ramDir;
  BaseTokenStreamTestCase::tearDown();
}

shared_ptr<Document> HighlighterTest::doc(const wstring &name,
                                          const wstring &value)
{
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(make_shared<Field>(
      name, value,
      fieldType)); // fieldType is randomly chosen for term vectors in setUp
  return d;
}

shared_ptr<Token> HighlighterTest::createToken(const wstring &term, int start,
                                               int offset)
{
  return make_shared<Token>(term, start, offset);
}

void HighlighterTest::InitializeInstanceFields()
{
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  fieldType->setStoreTermVectors(true);
  fieldType->setStoreTermVectorPositions(true);
  fieldType->setStoreTermVectorPayloads(true);
  fieldType->setStoreTermVectorOffsets(true);
  fieldType->freeze();
  FIELD_TYPE_TV = fieldType;
}

SynonymAnalyzer::SynonymAnalyzer(unordered_map<wstring, wstring> &synonyms)
{
  this->synonyms = synonyms;
}

shared_ptr<Analyzer::TokenStreamComponents>
SynonymAnalyzer::createComponents(const wstring &arg0)
{
  shared_ptr<Tokenizer> stream =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  stream->addAttribute(CharTermAttribute::typeid);
  stream->addAttribute(PositionIncrementAttribute::typeid);
  stream->addAttribute(OffsetAttribute::typeid);
  return make_shared<Analyzer::TokenStreamComponents>(
      stream, make_shared<SynonymTokenizer>(stream, synonyms));
}

SynonymTokenizer::SynonymTokenizer(shared_ptr<TokenStream> realStream,
                                   unordered_map<wstring, wstring> &synonyms)
    : realStream(realStream), synonyms(synonyms),
      realTermAtt(realStream->addAttribute(CharTermAttribute::typeid)),
      realPosIncrAtt(
          realStream->addAttribute(PositionIncrementAttribute::typeid)),
      realOffsetAtt(realStream->addAttribute(OffsetAttribute::typeid)),
      termAtt(addAttribute(CharTermAttribute::typeid)),
      posIncrAtt(addAttribute(PositionIncrementAttribute::typeid)),
      offsetAtt(addAttribute(OffsetAttribute::typeid))
{
}

bool SynonymTokenizer::incrementToken() 
{

  if (currentRealToken == nullptr) {
    bool next = realStream->incrementToken();
    if (!next) {
      return false;
    }
    // Token nextRealToken = new Token(, offsetAtt.startOffset(),
    // offsetAtt.endOffset());
    clearAttributes();
    termAtt->copyBuffer(realTermAtt->buffer(), 0, realTermAtt->length());
    offsetAtt->setOffset(realOffsetAtt->startOffset(),
                         realOffsetAtt->endOffset());
    posIncrAtt->setPositionIncrement(realPosIncrAtt->getPositionIncrement());

    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring expansions = synonyms[realTermAtt->toString()];
    if (expansions == L"") {
      return true;
    }
    st = make_shared<StringTokenizer>(expansions, L",");
    if (st->hasMoreTokens()) {
      currentRealToken = make_shared<Token>();
      currentRealToken->setOffset(realOffsetAtt->startOffset(),
                                  realOffsetAtt->endOffset());
      currentRealToken->copyBuffer(realTermAtt->buffer(), 0,
                                   realTermAtt->length());
    }

    return true;
  } else {
    wstring tok = st->nextToken();
    clearAttributes();
    termAtt->setEmpty()->append(tok);
    offsetAtt->setOffset(currentRealToken->startOffset(),
                         currentRealToken->endOffset());
    posIncrAtt->setPositionIncrement(0);
    if (!st->hasMoreTokens()) {
      currentRealToken.reset();
      st.reset();
    }
    return true;
  }
}

void SynonymTokenizer::reset() 
{
  TokenStream::reset();
  this->realStream->reset();
  this->currentRealToken.reset();
  this->st.reset();
}

void SynonymTokenizer::end() 
{
  TokenStream::end();
  this->realStream->end();
}

SynonymTokenizer::~SynonymTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  delete this->realStream;
}

shared_ptr<Highlighter> SynonymTokenizer::TestHighlightRunner::getHighlighter(
    shared_ptr<Query> query, const wstring &fieldName,
    shared_ptr<Formatter> formatter)
{
  return getHighlighter(query, fieldName, formatter, true);
}

shared_ptr<Highlighter> SynonymTokenizer::TestHighlightRunner::getHighlighter(
    shared_ptr<Query> query, const wstring &fieldName,
    shared_ptr<Formatter> formatter, bool expanMultiTerm)
{
  shared_ptr<Scorer> scorer;
  if (mode == QUERY) {
    scorer = make_shared<QueryScorer>(query, fieldName);
    if (!expanMultiTerm) {
      (std::static_pointer_cast<QueryScorer>(scorer))
          ->setExpandMultiTermQuery(false);
    }
  } else if (mode == QUERY_TERM) {
    scorer = make_shared<QueryTermScorer>(query);
  } else {
    throw runtime_error(L"Unknown highlight mode");
  }

  return make_shared<Highlighter>(formatter, scorer);
}

shared_ptr<Highlighter> SynonymTokenizer::TestHighlightRunner::getHighlighter(
    std::deque<std::shared_ptr<WeightedTerm>> &weightedTerms,
    shared_ptr<Formatter> formatter)
{
  if (mode == QUERY) {
    return make_shared<Highlighter>(
        formatter,
        make_shared<QueryScorer>(
            static_cast<std::deque<std::shared_ptr<WeightedSpanTerm>>>(
                weightedTerms)));
  } else if (mode == QUERY_TERM) {
    return make_shared<Highlighter>(
        formatter, make_shared<QueryTermScorer>(weightedTerms));

  } else {
    throw runtime_error(L"Unknown highlight mode");
  }
}

void SynonymTokenizer::TestHighlightRunner::doStandardHighlights(
    shared_ptr<Analyzer> analyzer, shared_ptr<IndexSearcher> searcher,
    shared_ptr<TopDocs> hits, shared_ptr<Query> query,
    shared_ptr<Formatter> formatter) 
{
  doStandardHighlights(analyzer, searcher, hits, query, formatter, false);
}

void SynonymTokenizer::TestHighlightRunner::doStandardHighlights(
    shared_ptr<Analyzer> analyzer, shared_ptr<IndexSearcher> searcher,
    shared_ptr<TopDocs> hits, shared_ptr<Query> query,
    shared_ptr<Formatter> formatter, bool expandMT) 
{

  for (int i = 0; i < hits->totalHits; i++) {
    constexpr int docId = hits->scoreDocs[i]->doc;
    shared_ptr<Document> *const doc = searcher->doc(docId);
    wstring text = doc[HighlighterTest::FIELD_NAME];
    int maxNumFragmentsRequired = 2;
    wstring fragmentSeparator = L"...";
    shared_ptr<Scorer> scorer = nullptr;
    shared_ptr<TokenStream> tokenStream = TokenSources::getTokenStream(
        HighlighterTest::FIELD_NAME,
        searcher->getIndexReader()->getTermVectors(docId), text, analyzer, -1);
    if (mode == QUERY) {
      scorer = make_shared<QueryScorer>(query);
    } else if (mode == QUERY_TERM) {
      scorer = make_shared<QueryTermScorer>(query);
    }
    shared_ptr<Highlighter> highlighter =
        make_shared<Highlighter>(formatter, scorer);
    highlighter->setTextFragmenter(frag);
    wstring result = highlighter->getBestFragments(
        tokenStream, text, maxNumFragmentsRequired, fragmentSeparator);
    if (LuceneTestCase::VERBOSE) {
      wcout << L"\t" << result << endl;
    }
  }
}

void SynonymTokenizer::TestHighlightRunner::start() 
{
  if (LuceneTestCase::VERBOSE) {
    wcout << L"Run QueryScorer" << endl;
  }
  run();
  if (LuceneTestCase::VERBOSE) {
    wcout << L"Run QueryTermScorer" << endl;
  }
  mode = QUERY_TERM;
  run();
}
} // namespace org::apache::lucene::search::highlight