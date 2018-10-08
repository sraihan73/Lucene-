using namespace std;

#include "FastVectorHighlighterTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using DelegatingAnalyzerWrapper =
    org::apache::lucene::analysis::DelegatingAnalyzerWrapper;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using CommonTermsQuery = org::apache::lucene::queries::CommonTermsQuery;
using CustomScoreQuery = org::apache::lucene::queries::CustomScoreQuery;
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using DefaultEncoder = org::apache::lucene::search::highlight::DefaultEncoder;
using Encoder = org::apache::lucene::search::highlight::Encoder;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;

void FastVectorHighlighterTest::testSimpleHighlightTest() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      L"This is a test where foo is highlighed and should be highlighted",
      type);

  doc->push_back(field);
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();

  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;
  shared_ptr<FieldQuery> fieldQuery = highlighter->getFieldQuery(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")), reader);
  std::deque<wstring> bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  // highlighted results are centered
  assertEquals(L"This is a test where <b>foo</b> is highlighed and should be "
               L"highlighted",
               bestFragments[0]);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 52, 1);
  assertEquals(L"This is a test where <b>foo</b> is highlighed and should be",
               bestFragments[0]);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 30, 1);
  assertEquals(L"a test where <b>foo</b> is highlighed", bestFragments[0]);
  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testCustomScoreQueryHighlight() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      L"This is a test where foo is highlighed and should be highlighted",
      type);

  doc->push_back(field);
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();

  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;
  shared_ptr<FieldQuery> fieldQuery = highlighter->getFieldQuery(
      make_shared<CustomScoreQuery>(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"foo"))),
      reader);
  std::deque<wstring> bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  // highlighted results are centered
  assertEquals(L"This is a test where <b>foo</b> is highlighed and should be "
               L"highlighted",
               bestFragments[0]);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 52, 1);
  assertEquals(L"This is a test where <b>foo</b> is highlighed and should be",
               bestFragments[0]);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 30, 1);
  assertEquals(L"a test where <b>foo</b> is highlighed", bestFragments[0]);
  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testFunctionScoreQueryHighlight() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      L"This is a test where foo is highlighed and should be highlighted",
      type);

  doc->push_back(field);
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();

  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;
  shared_ptr<FieldQuery> fieldQuery = highlighter->getFieldQuery(
      make_shared<FunctionScoreQuery>(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
          DoubleValuesSource::constant(1)),
      reader);
  std::deque<wstring> bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  // highlighted results are centered
  assertEquals(L"This is a test where <b>foo</b> is highlighed and should be "
               L"highlighted",
               bestFragments[0]);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 52, 1);
  assertEquals(L"This is a test where <b>foo</b> is highlighed and should be",
               bestFragments[0]);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 30, 1);
  assertEquals(L"a test where <b>foo</b> is highlighed", bestFragments[0]);
  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testPhraseHighlightLongTextTest() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<Field> text = make_shared<Field>(
      L"text",
      L"Netscape was the general name for a series of web browsers originally "
      L"produced by Netscape Communications Corporation, now a subsidiary of "
      L"AOL The original browser was once the dominant browser in terms of "
      L"usage share, but as a result of the first browser war it lost "
      L"virtually all of its share to Internet Explorer Netscape was "
      L"discontinued and support for all Netscape browsers and client products "
      L"was terminated on March 1, 2008 Netscape Navigator was the name of "
      L"Netscape\u0027s web browser from versions 1.0 through 4.8 The first "
      L"beta release versions of the browser were released in 1994 and known "
      L"as Mosaic and then Mosaic Netscape until a legal challenge from the "
      L"National Center for Supercomputing Applications (makers of NCSA "
      L"Mosaic, which many of Netscape\u0027s founders used to develop), led "
      L"to the name change to Netscape Navigator The company\u0027s name also "
      L"changed from Mosaic Communications Corporation to Netscape "
      L"Communications Corporation The browser was easily the most advanced...",
      type);
  doc->push_back(text);
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;
  wstring field = L"text";
  {
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"internet")),
               Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"explorer")),
               Occur::MUST);
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query->build(), reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 128, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"first browser war it lost virtually all of its share to "
                 L"<b>Internet</b> <b>Explorer</b> Netscape was discontinued "
                 L"and support for all Netscape browsers",
                 bestFragments[0]);
  }

  {
    shared_ptr<PhraseQuery> query =
        make_shared<PhraseQuery>(field, L"internet", L"explorer");
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query, reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 128, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"first browser war it lost virtually all of its share to "
                 L"<b>Internet Explorer</b> Netscape was discontinued and "
                 L"support for all Netscape browsers",
                 bestFragments[0]);
  }
  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testPhraseHighlightTest() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<Field> longTermField = make_shared<Field>(
      L"long_term",
      L"This is a test thisisaverylongwordandmakessurethisfails where foo is "
      L"highlighed and should be highlighted",
      type);
  shared_ptr<Field> noLongTermField = make_shared<Field>(
      L"no_long_term",
      L"This is a test where foo is highlighed and should be highlighted",
      type);

  doc->push_back(longTermField);
  doc->push_back(noLongTermField);
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;
  wstring field = L"no_long_term";
  {
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"test")),
               Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"foo")),
               Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"highlighed")),
               Occur::MUST);
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query->build(), reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 18, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"<b>foo</b> is <b>highlighed</b> and", bestFragments[0]);
  }
  {
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    shared_ptr<PhraseQuery> pq =
        make_shared<PhraseQuery>(5, field, L"test", L"foo", L"highlighed");
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"foo")),
               Occur::MUST);
    query->add(pq, Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"highlighed")),
               Occur::MUST);
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query->build(), reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 18, 1);
    // highlighted results are centered
    assertEquals(0, bestFragments.size());
    bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 30, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"a <b>test</b> where <b>foo</b> is <b>highlighed</b> and",
                 bestFragments[0]);
  }
  {
    shared_ptr<PhraseQuery> query =
        make_shared<PhraseQuery>(3, field, L"test", L"foo", L"highlighed");
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query, reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 18, 1);
    // highlighted results are centered
    assertEquals(0, bestFragments.size());
    bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 30, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"a <b>test</b> where <b>foo</b> is <b>highlighed</b> and",
                 bestFragments[0]);
  }
  {
    shared_ptr<PhraseQuery> query =
        make_shared<PhraseQuery>(30, field, L"test", L"foo", L"highlighed");
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query, reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 18, 1);
    assertEquals(0, bestFragments.size());
  }
  {
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    shared_ptr<PhraseQuery> pq =
        make_shared<PhraseQuery>(5, field, L"test", L"foo", L"highlighed");
    shared_ptr<BooleanQuery::Builder> inner =
        make_shared<BooleanQuery::Builder>();
    inner->add(pq, Occur::MUST);
    inner->add(make_shared<TermQuery>(make_shared<Term>(field, L"foo")),
               Occur::MUST);
    query->add(inner->build(), Occur::MUST);
    query->add(pq, Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"highlighed")),
               Occur::MUST);
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query->build(), reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 18, 1);
    assertEquals(0, bestFragments.size());

    bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 30, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"a <b>test</b> where <b>foo</b> is <b>highlighed</b> and",
                 bestFragments[0]);
  }

  field = L"long_term";
  {
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(
                   field, L"thisisaverylongwordandmakessurethisfails")),
               Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"foo")),
               Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(field, L"highlighed")),
               Occur::MUST);
    shared_ptr<FieldQuery> fieldQuery =
        highlighter->getFieldQuery(query->build(), reader);
    std::deque<wstring> bestFragments =
        highlighter->getBestFragments(fieldQuery, reader, docId, field, 18, 1);
    // highlighted results are centered
    assertEquals(1, bestFragments.size());
    assertEquals(L"<b>thisisaverylongwordandmakessurethisfails</b>",
                 bestFragments[0]);
  }
  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testBoostedPhraseHighlightTest() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>();
  text->append(L"words words junk junk junk junk junk junk junk junk highlight "
               L"junk junk junk junk together junk ");
  for (int i = 0; i < 10; i++) {
    text->append(L"junk junk junk junk junk junk junk junk junk junk junk junk "
                 L"junk junk junk junk junk junk junk junk ");
  }
  text->append(L"highlight words together ");
  for (int i = 0; i < 10; i++) {
    text->append(L"junk junk junk junk junk junk junk junk junk junk junk junk "
                 L"junk junk junk junk junk junk junk junk ");
  }
  doc->push_back(make_shared<Field>(L"text", text->toString()->trim(), type));
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);

  // This mimics what some query parsers do to <highlight words together>
  shared_ptr<BooleanQuery::Builder> terms =
      make_shared<BooleanQuery::Builder>();
  terms->add(clause(L"text", {L"highlight"}), Occur::MUST);
  terms->add(clause(L"text", {L"words"}), Occur::MUST);
  terms->add(clause(L"text", {L"together"}), Occur::MUST);
  // This mimics what some query parsers do to <"highlight words together">
  shared_ptr<BooleanQuery::Builder> phraseB =
      make_shared<BooleanQuery::Builder>();
  phraseB->add(clause(L"text", {L"highlight", L"words", L"together"}),
               Occur::MUST);
  shared_ptr<Query> phrase = phraseB->build();
  phrase = make_shared<BoostQuery>(phrase, 100.0f);
  // Now combine those results in a bool query which should pull the phrases
  // to the front of the deque of fragments
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(phrase, Occur::MUST);
  query->add(phrase, Occur::SHOULD);
  shared_ptr<FieldQuery> fieldQuery =
      make_shared<FieldQuery>(query->build(), reader, true, false);
  wstring fragment =
      highlighter->getBestFragment(fieldQuery, reader, 0, L"text", 100);
  assertEquals(L"junk junk junk junk junk junk junk junk <b>highlight words "
               L"together</b> junk junk junk junk junk junk junk junk",
               fragment);

  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testCommonTermsQueryHighlight() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(
               make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                         MockTokenFilter::ENGLISH_STOPSET)));
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  std::deque<wstring> texts = {
      L"Hello this is a piece of text that is very long and contains too much "
      L"preamble and the meat is really here which says kennedy has been shot",
      L"This piece of text refers to Kennedy at the beginning then has a "
      L"longer piece of text that is very long in the middle and finally ends "
      L"with another reference to Kennedy",
      L"JFK has been shot",
      L"John Kennedy has been shot",
      L"This text has a typo in referring to Keneddy",
      L"wordx wordy wordz wordx wordy wordx worda wordb wordy wordc",
      L"y z x y z a b",
      L"lets is a the lets is a the lets is a the lets"};
  for (int i = 0; i < texts.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> field = make_shared<Field>(L"field", texts[i], type);
    doc->push_back(field);
    writer->addDocument(doc);
  }
  shared_ptr<CommonTermsQuery> query =
      make_shared<CommonTermsQuery>(Occur::MUST, Occur::SHOULD, 2);
  query->add(make_shared<Term>(L"field", L"text"));
  query->add(make_shared<Term>(L"field", L"long"));
  query->add(make_shared<Term>(L"field", L"very"));

  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TopDocs> hits = searcher->search(query, 10);
  assertEquals(2, hits->totalHits);
  shared_ptr<FieldQuery> fieldQuery = highlighter->getFieldQuery(query, reader);
  std::deque<wstring> bestFragments = highlighter->getBestFragments(
      fieldQuery, reader, hits->scoreDocs[0]->doc, L"field", 1000, 1);
  assertEquals(
      L"This piece of <b>text</b> refers to Kennedy at the beginning then has "
      L"a longer piece of <b>text</b> that is <b>very</b> <b>long</b> in the "
      L"middle and finally ends with another reference to Kennedy",
      bestFragments[0]);

  fieldQuery = highlighter->getFieldQuery(query, reader);
  bestFragments = highlighter->getBestFragments(
      fieldQuery, reader, hits->scoreDocs[1]->doc, L"field", 1000, 1);
  assertEquals(L"Hello this is a piece of <b>text</b> that is <b>very</b> "
               L"<b>long</b> and contains too much preamble and the meat is "
               L"really here which says kennedy has been shot",
               bestFragments[0]);

  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testMatchedFields() 
{
  // Searching just on the stored field doesn't highlight a stopword
  matchedFieldsTestCase(false, true,
                        {L"a match", L"a <b>match</b>",
                         clause(L"field", {L"a"}),
                         clause(L"field", {L"match"})});

  // Even if you add an unqueried matched field that would match it
  matchedFieldsTestCase(
      L"a match", L"a <b>match</b>",
      {clause(L"field", {L"a"}), clause(L"field", {L"match"})});

  // Nor if you query the field but don't add it as a matched field to the
  // highlighter
  matchedFieldsTestCase(false, false,
                        {L"a match", L"a <b>match</b>",
                         clause(L"field_exact", {L"a"}),
                         clause(L"field", {L"match"})});

  // But if you query the field and add it as a matched field to the highlighter
  // then it is highlighted
  matchedFieldsTestCase(
      L"a match", L"<b>a</b> <b>match</b>",
      {clause(L"field_exact", {L"a"}), clause(L"field", {L"match"})});

  // It is also ok to match just the matched field but get highlighting from the
  // stored field
  matchedFieldsTestCase(
      L"a match", L"<b>a</b> <b>match</b>",
      {clause(L"field_exact", {L"a"}), clause(L"field_exact", {L"match"})});

  // Boosted matched fields work too
  matchedFieldsTestCase(
      L"a match", L"<b>a</b> <b>match</b>",
      {clause(L"field_exact", 5, L"a"), clause(L"field", {L"match"})});

  // It is also ok if both the stored and the matched field match the term
  matchedFieldsTestCase(
      L"a match", L"a <b>match</b>",
      {clause(L"field_exact", {L"match"}), clause(L"field", {L"match"})});

  // And the highlighter respects the boosts on matched fields when sorting
  // fragments
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"junk junk <b>a cat</b> junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_exact", 5, L"a", L"cat")});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> <b>cat</b> junk junk junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_exact", {L"a", L"cat"})});

  // The same thing works across three fields as well
  matchedFieldsTestCase(
      L"cat cat CAT junk junk junk junk junk junk junk a cat junk junk",
      L"junk junk <b>a cat</b> junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_exact", 200, L"a", L"cat"),
       clause(L"field_super_exact", 5, L"CAT")});
  matchedFieldsTestCase(
      L"a cat cat junk junk junk junk junk junk junk a CAT junk junk",
      L"junk junk <b>a CAT</b> junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_exact", 5, L"a", L"cat"),
       clause(L"field_super_exact", 200, L"a", L"CAT")});

  // And across fields with different tokenizers!
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"junk junk <b>a cat</b> junk junk",
      {clause(L"field_exact", 5, L"a", L"cat"),
       clause(L"field_characters", {L"c"})});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>c</b>at <b>c</b>at junk junk junk junk",
      {clause(L"field_exact", {L"a", L"cat"}),
       clause(L"field_characters", {L"c"})});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"ca<b>t</b> ca<b>t</b> junk junk junk junk",
      {clause(L"field_exact", {L"a", L"cat"}),
       clause(L"field_characters", {L"t"})});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> <b>cat</b> junk junk junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_characters", 5, L"c")});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"junk junk <b>a cat</b> junk junk",
      {clause(L"field", {L"cat"}),
       clause(L"field_characters", 5, L"a", L" ", L"c", L"a", L"t")});

  // Phrases and tokens inside one another are joined
  matchedFieldsTestCase(
      L"cats wow", L"<b>cats w</b>ow",
      {clause(L"field", {L"cats"}), clause(L"field_tripples", {L"s w"})});

  // Everything works pretty well even if you don't require a field match
  matchedFieldsTestCase(
      true, false,
      {L"cat cat junk junk junk junk junk junk junk a cat junk junk",
       L"junk junk <b>a cat</b> junk junk", clause(L"field", {L"cat"}),
       clause(L"field_characters", 10, L"a", L" ", L"c", L"a", L"t")});

  // Even boosts keep themselves pretty much intact
  matchedFieldsTestCase(
      true, false,
      {L"a cat cat junk junk junk junk junk junk junk a CAT junk junk",
       L"junk junk <b>a CAT</b> junk junk", clause(L"field", {L"cat"}),
       clause(L"field_exact", 5, L"a", L"cat"),
       clause(L"field_super_exact", 200, L"a", L"CAT")});
  matchedFieldsTestCase(
      true, false,
      {L"cat cat CAT junk junk junk junk junk junk junk a cat junk junk",
       L"junk junk <b>a cat</b> junk junk", clause(L"field", {L"cat"}),
       clause(L"field_exact", 200, L"a", L"cat"),
       clause(L"field_super_exact", 5, L"CAT")});

  // Except that all the matched field matches apply even if they aren't
  // mentioned in the query which can make for some confusing scoring.  This
  // isn't too big a deal, just something you need to think about when you don't
  // force a field match.
  matchedFieldsTestCase(
      true, false,
      {L"cat cat junk junk junk junk junk junk junk a cat junk junk",
       L"<b>cat</b> <b>cat</b> junk junk junk junk", clause(L"field", {L"cat"}),
       clause(L"field_characters", 4, L"a", L" ", L"c", L"a", L"t")});

  // It is also cool to match fields that don't have _exactly_ the same text so
  // long as you are careful. In this case field_sliced is a prefix of field.
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> <b>cat</b> junk junk junk junk",
      {clause(L"field_sliced", {L"cat"})});

  // Multiple matches add to the score of the segment
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> <b>cat</b> junk junk junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_sliced", {L"cat"}),
       clause(L"field_exact", 2, L"a", L"cat")});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"junk junk <b>a cat</b> junk junk",
      {clause(L"field", {L"cat"}), clause(L"field_sliced", {L"cat"}),
       clause(L"field_exact", 4, L"a", L"cat")});

  // Even fields with tokens on top of one another are ok
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> cat junk junk junk junk",
      {clause(L"field_der_red", 2, L"der"),
       clause(L"field_exact", {L"a", L"cat"})});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> cat junk junk junk junk",
      {clause(L"field_der_red", 2, L"red"),
       clause(L"field_exact", {L"a", L"cat"})});
  matchedFieldsTestCase(
      L"cat cat junk junk junk junk junk junk junk a cat junk junk",
      L"<b>cat</b> cat junk junk junk junk",
      {clause(L"field_der_red", {L"red"}), clause(L"field_der_red", {L"der"}),
       clause(L"field_exact", {L"a", L"cat"})});
}

void FastVectorHighlighterTest::testMultiValuedSortByScore() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  doc->push_back(
      make_shared<Field>(L"field", L"zero if naught",
                         type)); // The first two fields contain the best match
  doc->push_back(
      make_shared<Field>(L"field", L"hero of legend",
                         type)); // but total a lower score (3) than the bottom
  doc->push_back(
      make_shared<Field>(L"field", L"naught of hero", type)); // two fields (4)
  doc->push_back(make_shared<Field>(L"field", L"naught of hero", type));
  writer->addDocument(doc);

  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();

  shared_ptr<ScoreOrderFragmentsBuilder> fragmentsBuilder =
      make_shared<ScoreOrderFragmentsBuilder>();
  fragmentsBuilder->setDiscreteMultiValueHighlighting(true);
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  std::deque<wstring> preTags = {L"<b>"};
  std::deque<wstring> postTags = {L"</b>"};
  shared_ptr<Encoder> encoder = make_shared<DefaultEncoder>();
  int docId = 0;
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(clause(L"field", {L"hero"}), Occur::SHOULD);
  query->add(clause(L"field", {L"of"}), Occur::SHOULD);
  query->add(clause(L"field", {L"legend"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fieldQuery =
      highlighter->getFieldQuery(query->build(), reader);

  for (auto fragListBuilder : std::deque<std::shared_ptr<FragListBuilder>>{
           make_shared<SimpleFragListBuilder>(),
           make_shared<WeightedFragListBuilder>()}) {
    std::deque<wstring> bestFragments = highlighter->getBestFragments(
        fieldQuery, reader, docId, L"field", 20, 1, fragListBuilder,
        fragmentsBuilder, preTags, postTags, encoder);
    assertEquals(L"<b>hero</b> <b>of</b> <b>legend</b>", bestFragments[0]);
    bestFragments = highlighter->getBestFragments(
        fieldQuery, reader, docId, L"field", 28, 1, fragListBuilder,
        fragmentsBuilder, preTags, postTags, encoder);
    assertEquals(L"<b>hero</b> <b>of</b> <b>legend</b>", bestFragments[0]);
    bestFragments = highlighter->getBestFragments(
        fieldQuery, reader, docId, L"field", 30000, 1, fragListBuilder,
        fragmentsBuilder, preTags, postTags, encoder);
    assertEquals(L"<b>hero</b> <b>of</b> <b>legend</b>", bestFragments[0]);
  }

  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testWithSynonym() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"the quick brown fox", type));
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();

  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;

  // query1: simple synonym query
  shared_ptr<SynonymQuery> synQuery =
      make_shared<SynonymQuery>(make_shared<Term>(L"field", L"quick"),
                                make_shared<Term>(L"field", L"fast"));
  shared_ptr<FieldQuery> fieldQuery =
      highlighter->getFieldQuery(synQuery, reader);
  std::deque<wstring> bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  assertEquals(L"the <b>quick</b> brown fox", bestFragments[0]);

  // query2: bool query with synonym query
  shared_ptr<BooleanQuery::Builder> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<BooleanClause>(synQuery, Occur::MUST))
          ->add(make_shared<BooleanClause>(
              make_shared<TermQuery>(make_shared<Term>(L"field", L"fox")),
              Occur::MUST));
  fieldQuery = highlighter->getFieldQuery(bq->build(), reader);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  assertEquals(L"the <b>quick</b> brown <b>fox</b>", bestFragments[0]);

  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testBooleanPhraseWithSynonym() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> type =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();
  shared_ptr<Token> syn = make_shared<Token>(L"httpwwwfacebookcom", 6, 29);
  syn->setPositionIncrement(0);
  shared_ptr<CannedTokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"test", 0, 4), make_shared<Token>(L"http", 6, 10),
      syn, make_shared<Token>(L"www", 13, 16),
      make_shared<Token>(L"facebook", 17, 25),
      make_shared<Token>(L"com", 26, 29));
  shared_ptr<Field> field = make_shared<Field>(L"field", ts, type);
  doc->push_back(field);
  doc->push_back(
      make_shared<StoredField>(L"field", L"Test: http://www.facebook.com"));
  writer->addDocument(doc);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();

  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;

  // query1: match
  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(
      L"field", L"test", L"http", L"www", L"facebook", L"com");
  shared_ptr<FieldQuery> fieldQuery = highlighter->getFieldQuery(pq, reader);
  std::deque<wstring> bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  assertEquals(L"<b>Test: http://www.facebook.com</b>", bestFragments[0]);

  // query2: match
  shared_ptr<PhraseQuery> pq2 = make_shared<PhraseQuery>(
      L"field", L"test", L"httpwwwfacebookcom", L"www", L"facebook", L"com");
  fieldQuery = highlighter->getFieldQuery(pq2, reader);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  assertEquals(L"<b>Test: http://www.facebook.com</b>", bestFragments[0]);

  // query3: OR query1 and query2 together
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(pq, Occur::SHOULD);
  bq->add(pq2, Occur::SHOULD);
  fieldQuery = highlighter->getFieldQuery(bq->build(), reader);
  bestFragments =
      highlighter->getBestFragments(fieldQuery, reader, docId, L"field", 54, 1);
  assertEquals(L"<b>Test: http://www.facebook.com</b>", bestFragments[0]);

  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::testPhrasesSpanningFieldValues() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  // positionIncrementGap is 0 so the pharse is found across multiple field
  // values.
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"one two three five", type));
  doc->push_back(make_shared<Field>(L"field", L"two three four", type));
  doc->push_back(make_shared<Field>(L"field", L"five six five", type));
  doc->push_back(
      make_shared<Field>(L"field",
                         wstring(L"six seven eight nine eight nine eight ") +
                             L"nine eight nine eight nine eight nine",
                         type));
  doc->push_back(make_shared<Field>(L"field", L"eight nine", type));
  doc->push_back(make_shared<Field>(L"field", L"ten eleven", type));
  doc->push_back(make_shared<Field>(L"field", L"twelve thirteen", type));
  writer->addDocument(doc);

  shared_ptr<BaseFragListBuilder> fragListBuilder =
      make_shared<SimpleFragListBuilder>();
  shared_ptr<BaseFragmentsBuilder> fragmentsBuilder =
      make_shared<SimpleFragmentsBuilder>();
  fragmentsBuilder->setDiscreteMultiValueHighlighting(true);
  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>(true, true, fragListBuilder,
                                         fragmentsBuilder);
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  int docId = 0;

  // Phrase that spans a field value
  shared_ptr<Query> q = make_shared<PhraseQuery>(L"field", L"four", L"five");
  shared_ptr<FieldQuery> fieldQuery = highlighter->getFieldQuery(q, reader);
  std::deque<wstring> bestFragments = highlighter->getBestFragments(
      fieldQuery, reader, docId, L"field", 1000, 1000);
  assertEquals(L"two three <b>four</b>", bestFragments[0]);
  assertEquals(L"<b>five</b> six five", bestFragments[1]);
  assertEquals(2, bestFragments.size());

  // Phrase that ends at a field value
  q = make_shared<PhraseQuery>(L"field", L"three", L"five");
  fieldQuery = highlighter->getFieldQuery(q, reader);
  bestFragments = highlighter->getBestFragments(fieldQuery, reader, docId,
                                                L"field", 1000, 1000);
  assertEquals(L"one two <b>three five</b>", bestFragments[0]);
  assertEquals(1, bestFragments.size());

  // Phrase that spans across three values
  q = make_shared<PhraseQuery>(L"field", L"nine", L"ten", L"eleven", L"twelve");
  fieldQuery = highlighter->getFieldQuery(q, reader);
  bestFragments = highlighter->getBestFragments(fieldQuery, reader, docId,
                                                L"field", 1000, 1000);
  assertEquals(L"eight <b>nine</b>", bestFragments[0]);
  assertEquals(L"<b>ten eleven</b>", bestFragments[1]);
  assertEquals(L"<b>twelve</b> thirteen", bestFragments[2]);
  assertEquals(3, bestFragments.size());

  // Term query that appears in multiple values
  q = make_shared<TermQuery>(make_shared<Term>(L"field", L"two"));
  fieldQuery = highlighter->getFieldQuery(q, reader);
  bestFragments = highlighter->getBestFragments(fieldQuery, reader, docId,
                                                L"field", 1000, 1000);
  assertEquals(L"one <b>two</b> three five", bestFragments[0]);
  assertEquals(L"<b>two</b> three four", bestFragments[1]);
  assertEquals(2, bestFragments.size());

  delete reader;
  delete writer;
  delete dir;
}

void FastVectorHighlighterTest::matchedFieldsTestCase(
    const wstring &fieldValue, const wstring &expected,
    deque<Query> &queryClauses) 
{
  matchedFieldsTestCase(true, true, {fieldValue, expected, queryClauses});
}

void FastVectorHighlighterTest::matchedFieldsTestCase(
    bool useMatchedFields, bool fieldMatch, const wstring &fieldValue,
    const wstring &expected, deque<Query> &queryClauses) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> stored = make_shared<FieldType>(TextField::TYPE_STORED);
  stored->setStoreTermVectorOffsets(true);
  stored->setStoreTermVectorPositions(true);
  stored->setStoreTermVectors(true);
  stored->freeze();
  shared_ptr<FieldType> matched =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  matched->setStoreTermVectorOffsets(true);
  matched->setStoreTermVectorPositions(true);
  matched->setStoreTermVectors(true);
  matched->freeze();
  doc->push_back(make_shared<Field>(
      L"field", fieldValue,
      stored)); // Whitespace tokenized with English stop words
  doc->push_back(
      make_shared<Field>(L"field_exact", fieldValue,
                         matched)); // Whitespace tokenized without stop words
  doc->push_back(
      make_shared<Field>(L"field_super_exact", fieldValue,
                         matched)); // Whitespace tokenized without toLower
  doc->push_back(make_shared<Field>(L"field_characters", fieldValue,
                                    matched)); // Each letter is a token
  doc->push_back(make_shared<Field>(L"field_tripples", fieldValue,
                                    matched)); // Every three letters is a token
  doc->push_back(make_shared<Field>(
      L"field_sliced", fieldValue.substr(0, min(fieldValue.length() - 1, 10)),
      matched));
  doc->push_back(
      make_shared<Field>(L"field_der_red",
                         make_shared<CannedTokenStream>(token(L"der", 1, 0, 3),
                                                        token(L"red", 0, 0, 3)),
                         matched));

  const unordered_map<wstring, std::shared_ptr<Analyzer>> fieldAnalyzers =
      map_obj<wstring, std::shared_ptr<Analyzer>>();
  fieldAnalyzers.emplace(L"field", make_shared<MockAnalyzer>(
                                       random(), MockTokenizer::WHITESPACE,
                                       true, MockTokenFilter::ENGLISH_STOPSET));
  fieldAnalyzers.emplace(L"field_exact", make_shared<MockAnalyzer>(random()));
  fieldAnalyzers.emplace(
      L"field_super_exact",
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  fieldAnalyzers.emplace(
      L"field_characters",
      make_shared<MockAnalyzer>(random(),
                                make_shared<CharacterRunAutomaton>(
                                    (make_shared<RegExp>(L"."))->toAutomaton()),
                                true));
  fieldAnalyzers.emplace(L"field_tripples",
                         make_shared<MockAnalyzer>(
                             random(),
                             make_shared<CharacterRunAutomaton>(
                                 (make_shared<RegExp>(L"..."))->toAutomaton()),
                             true));
  fieldAnalyzers.emplace(L"field_sliced", fieldAnalyzers[L"field"]);
  fieldAnalyzers.emplace(
      L"field_der_red",
      fieldAnalyzers[L"field"]); // This is required even though we provide a
                                 // token stream
  shared_ptr<Analyzer> analyzer =
      make_shared<DelegatingAnalyzerWrapperAnonymousInnerClass>(
          shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY,
          fieldAnalyzers);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  writer->addDocument(doc);

  shared_ptr<FastVectorHighlighter> highlighter =
      make_shared<FastVectorHighlighter>();
  shared_ptr<FragListBuilder> fragListBuilder =
      make_shared<SimpleFragListBuilder>();
  shared_ptr<FragmentsBuilder> fragmentsBuilder =
      make_shared<ScoreOrderFragmentsBuilder>();
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  std::deque<wstring> preTags = {L"<b>"};
  std::deque<wstring> postTags = {L"</b>"};
  shared_ptr<Encoder> encoder = make_shared<DefaultEncoder>();
  int docId = 0;
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  for (shared_ptr<Query> clause : queryClauses) {
    query->add(clause, Occur::MUST);
  }
  shared_ptr<FieldQuery> fieldQuery =
      make_shared<FieldQuery>(query->build(), reader, true, fieldMatch);
  std::deque<wstring> bestFragments;
  if (useMatchedFields) {
    shared_ptr<Set<wstring>> matchedFields = unordered_set<wstring>();
    matchedFields->add(L"field");
    matchedFields->add(L"field_exact");
    matchedFields->add(L"field_super_exact");
    matchedFields->add(L"field_characters");
    matchedFields->add(L"field_tripples");
    matchedFields->add(L"field_sliced");
    matchedFields->add(L"field_der_red");
    bestFragments = highlighter->getBestFragments(
        fieldQuery, reader, docId, L"field", matchedFields, 25, 1,
        fragListBuilder, fragmentsBuilder, preTags, postTags, encoder);
  } else {
    bestFragments = highlighter->getBestFragments(
        fieldQuery, reader, docId, L"field", 25, 1, fragListBuilder,
        fragmentsBuilder, preTags, postTags, encoder);
  }
  assertEquals(expected, bestFragments[0]);

  delete reader;
  delete writer;
  delete dir;
}

FastVectorHighlighterTest::DelegatingAnalyzerWrapperAnonymousInnerClass::
    DelegatingAnalyzerWrapperAnonymousInnerClass(
        shared_ptr<FastVectorHighlighterTest> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY,
        unordered_map<wstring, std::shared_ptr<Analyzer>> &fieldAnalyzers)
    : org::apache::lucene::analysis::DelegatingAnalyzerWrapper(
          PER_FIELD_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
  this->fieldAnalyzers = fieldAnalyzers;
}

shared_ptr<Analyzer>
FastVectorHighlighterTest::DelegatingAnalyzerWrapperAnonymousInnerClass::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return fieldAnalyzers[fieldName];
}

shared_ptr<Query> FastVectorHighlighterTest::clause(const wstring &field,
                                                    deque<wstring> &terms)
{
  return clause(field, 1, terms);
}

shared_ptr<Query> FastVectorHighlighterTest::clause(const wstring &field,
                                                    float boost,
                                                    deque<wstring> &terms)
{
  shared_ptr<Query> q;
  if (terms->length == 1) {
    q = make_shared<TermQuery>(make_shared<Term>(field, terms[0]));
  } else {
    q = make_shared<PhraseQuery>(field, terms);
  }
  q = make_shared<BoostQuery>(q, boost);
  return q;
}

shared_ptr<Token> FastVectorHighlighterTest::token(const wstring &term,
                                                   int posInc, int startOffset,
                                                   int endOffset)
{
  shared_ptr<Token> t = make_shared<Token>(term, startOffset, endOffset);
  t->setPositionIncrement(posInc);
  return t;
}
} // namespace org::apache::lucene::search::vectorhighlight