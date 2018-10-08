using namespace std;

#include "TestMoreLikeThis.h"

namespace org::apache::lucene::queries::mlt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestMoreLikeThis::SHOP_TYPE = L"type";
const wstring TestMoreLikeThis::FOR_SALE = L"weSell";
const wstring TestMoreLikeThis::NOT_FOR_SALE = L"weDontSell";

void TestMoreLikeThis::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);

  // Add series of docs with specific information for MoreLikeThis
  addDoc(writer, L"lucene");
  addDoc(writer, L"lucene release");
  addDoc(writer, L"apache");
  addDoc(writer, L"apache lucene");

  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(reader);
}

void TestMoreLikeThis::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestMoreLikeThis::addDoc(shared_ptr<RandomIndexWriter> writer,
                              const wstring &text) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"text", text, Field::Store::YES));
  writer->addDocument(doc);
}

void TestMoreLikeThis::addDoc(shared_ptr<RandomIndexWriter> writer,
                              std::deque<wstring> &texts) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  for (auto text : texts) {
    doc->push_back(newTextField(L"text", text, Field::Store::YES));
  }
  writer->addDocument(doc);
}

void TestMoreLikeThis::testBoostFactor() 
{
  unordered_map<wstring, float> originalValues = getOriginalValues();

  shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  mlt->setAnalyzer(analyzer);
  mlt->setMinDocFreq(1);
  mlt->setMinTermFreq(1);
  mlt->setMinWordLen(1);
  mlt->setFieldNames(std::deque<wstring>{L"text"});
  mlt->setBoost(true);

  // this mean that every term boost factor will be multiplied by this
  // number
  float boostFactor = 5;
  mlt->setBoostFactor(boostFactor);

  shared_ptr<BooleanQuery> query = std::static_pointer_cast<BooleanQuery>(
      mlt->like(L"text", make_shared<StringReader>(L"lucene release")));
  shared_ptr<deque<std::shared_ptr<BooleanClause>>> clauses =
      query->clauses();

  assertEquals(L"Expected " + originalValues.size() + L" clauses.",
               originalValues.size(), clauses->size());

  for (auto clause : clauses) {
    shared_ptr<BoostQuery> bq =
        std::static_pointer_cast<BoostQuery>(clause->getQuery());
    shared_ptr<TermQuery> tq =
        std::static_pointer_cast<TermQuery>(bq->getQuery());
    optional<float> termBoost = originalValues[tq->getTerm()->text()];
    assertNotNull(L"Expected term " + tq->getTerm()->text(), termBoost);

    float totalBoost = termBoost * boostFactor;
    assertEquals(L"Expected boost of " + to_wstring(totalBoost) +
                     L" for term '" + tq->getTerm()->text() + L"' got " +
                     to_wstring(bq->getBoost()),
                 totalBoost, bq->getBoost(), 0.0001);
  }
  delete analyzer;
}

unordered_map<wstring, float>
TestMoreLikeThis::getOriginalValues() 
{
  unordered_map<wstring, float> originalValues =
      unordered_map<wstring, float>();
  shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  mlt->setAnalyzer(analyzer);
  mlt->setMinDocFreq(1);
  mlt->setMinTermFreq(1);
  mlt->setMinWordLen(1);
  mlt->setFieldNames(std::deque<wstring>{L"text"});
  mlt->setBoost(true);
  shared_ptr<BooleanQuery> query = std::static_pointer_cast<BooleanQuery>(
      mlt->like(L"text", make_shared<StringReader>(L"lucene release")));
  shared_ptr<deque<std::shared_ptr<BooleanClause>>> clauses =
      query->clauses();

  for (auto clause : clauses) {
    shared_ptr<BoostQuery> bq =
        std::static_pointer_cast<BoostQuery>(clause->getQuery());
    shared_ptr<TermQuery> tq =
        std::static_pointer_cast<TermQuery>(bq->getQuery());
    originalValues.emplace(tq->getTerm()->text(), bq->getBoost());
  }
  delete analyzer;
  return originalValues;
}

void TestMoreLikeThis::testMultiFields() 
{
  shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  mlt->setAnalyzer(analyzer);
  mlt->setMinDocFreq(1);
  mlt->setMinTermFreq(1);
  mlt->setMinWordLen(1);
  mlt->setFieldNames(std::deque<wstring>{L"text", L"foobar"});
  mlt->like(L"foobar", make_shared<StringReader>(L"this is a test"));
  delete analyzer;
}

void TestMoreLikeThis::testMultiValues() 
{
  shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  mlt->setAnalyzer(analyzer);
  mlt->setMinDocFreq(1);
  mlt->setMinTermFreq(1);
  mlt->setMinWordLen(1);
  mlt->setFieldNames(std::deque<wstring>{L"text"});

  shared_ptr<BooleanQuery> query = std::static_pointer_cast<BooleanQuery>(
      mlt->like(L"text", make_shared<StringReader>(L"lucene"),
                make_shared<StringReader>(L"lucene release"),
                make_shared<StringReader>(L"apache"),
                make_shared<StringReader>(L"apache lucene")));
  shared_ptr<deque<std::shared_ptr<BooleanClause>>> clauses =
      query->clauses();
  assertEquals(L"Expected 2 clauses only!", 2, clauses->size());
  for (auto clause : clauses) {
    shared_ptr<Term> term =
        (std::static_pointer_cast<TermQuery>(clause->getQuery()))->getTerm();
    assertTrue(Arrays::asList(make_shared<Term>(L"text", L"lucene"),
                              make_shared<Term>(L"text", L"apache"))
                   ->contains(term));
  }
  delete analyzer;
}

void TestMoreLikeThis::testMoreLikeThisQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Query> query = make_shared<MoreLikeThisQuery>(
      L"this is a test", std::deque<wstring>{L"text"}, analyzer, L"text");
  QueryUtils::check(random(), query, searcher);
  delete analyzer;
}

void TestMoreLikeThis::testTopN() 
{
  int numDocs = 100;
  int topN = 25;

  // add series of docs with terms of decreasing df
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < numDocs; i++) {
    addDoc(writer, generateStrSeq(0, i + 1));
  }
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  // setup MLT query
  shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  mlt->setAnalyzer(analyzer);
  mlt->setMaxQueryTerms(topN);
  mlt->setMinDocFreq(1);
  mlt->setMinTermFreq(1);
  mlt->setMinWordLen(1);
  mlt->setFieldNames(std::deque<wstring>{L"text"});

  // perform MLT query
  wstring likeText = L"";
  for (auto text : generateStrSeq(0, numDocs)) {
    likeText += text + L" ";
  }
  shared_ptr<BooleanQuery> query = std::static_pointer_cast<BooleanQuery>(
      mlt->like(L"text", make_shared<StringReader>(likeText)));

  // check best terms are topN of highest idf
  shared_ptr<deque<std::shared_ptr<BooleanClause>>> clauses =
      query->clauses();
  assertEquals(L"Expected" + to_wstring(topN) + L"clauses only!", topN,
               clauses->size());

  std::deque<std::shared_ptr<Term>> expectedTerms(topN);
  int idx = 0;
  for (auto text : generateStrSeq(numDocs - topN, topN)) {
    expectedTerms[idx++] = make_shared<Term>(L"text", text);
  }
  for (auto clause : clauses) {
    shared_ptr<Term> term =
        (std::static_pointer_cast<TermQuery>(clause->getQuery()))->getTerm();
    assertTrue(Arrays::asList(expectedTerms)->contains(term));
  }

  // clean up
  delete reader;
  delete dir;
  delete analyzer;
}

std::deque<wstring> TestMoreLikeThis::generateStrSeq(int from, int size)
{
  std::deque<wstring> generatedStrings(size);
  for (int i = 0; i < generatedStrings.size(); i++) {
    generatedStrings[i] = to_wstring(from + i);
  }
  return generatedStrings;
}

int TestMoreLikeThis::addShopDoc(
    shared_ptr<RandomIndexWriter> writer, const wstring &type,
    std::deque<wstring> &weSell,
    std::deque<wstring> &weDontSell) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(SHOP_TYPE, type, Field::Store::YES));
  for (auto item : weSell) {
    doc->push_back(newTextField(FOR_SALE, item, Field::Store::YES));
  }
  for (auto item : weDontSell) {
    doc->push_back(newTextField(NOT_FOR_SALE, item, Field::Store::YES));
  }
  writer->addDocument(doc);
  return writer->numDocs() - 1;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AwaitsFix(bugUrl =
// "https://issues.apache.org/jira/browse/LUCENE-7161") public void
// testMultiFieldShouldReturnPerFieldBooleanQuery() throws Exception
void TestMoreLikeThis::testMultiFieldShouldReturnPerFieldBooleanQuery() throw(
    runtime_error)
{
  shared_ptr<IndexReader> reader = nullptr;
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  try {
    int maxQueryTerms = 25;

    std::deque<wstring> itShopItemForSale = {
        L"watch",   L"ipod",     L"asrock", L"imac",    L"macbookpro",
        L"monitor", L"keyboard", L"mouse",  L"speakers"};
    std::deque<wstring> itShopItemNotForSale = {L"tie", L"trousers", L"shoes",
                                                 L"skirt", L"hat"};

    std::deque<wstring> clothesShopItemForSale = {L"tie", L"trousers",
                                                   L"shoes", L"skirt", L"hat"};
    std::deque<wstring> clothesShopItemNotForSale = {
        L"watch",   L"ipod",     L"asrock", L"imac",    L"macbookpro",
        L"monitor", L"keyboard", L"mouse",  L"speakers"};

    // add series of shop docs
    shared_ptr<RandomIndexWriter> writer =
        make_shared<RandomIndexWriter>(random(), dir);
    for (int i = 0; i < 300; i++) {
      addShopDoc(writer, L"it", itShopItemForSale, itShopItemNotForSale);
    }
    for (int i = 0; i < 300; i++) {
      addShopDoc(writer, L"clothes", clothesShopItemForSale,
                 clothesShopItemNotForSale);
    }
    // Input Document is a clothes shop
    int inputDocId = addShopDoc(writer, L"clothes", clothesShopItemForSale,
                                clothesShopItemNotForSale);
    reader = writer->getReader();
    delete writer;

    // setup MLT query
    shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);

    mlt->setAnalyzer(analyzer);
    mlt->setMaxQueryTerms(maxQueryTerms);
    mlt->setMinDocFreq(1);
    mlt->setMinTermFreq(1);
    mlt->setMinWordLen(1);
    mlt->setFieldNames(std::deque<wstring>{FOR_SALE, NOT_FOR_SALE});

    // perform MLT query
    shared_ptr<BooleanQuery> query =
        std::static_pointer_cast<BooleanQuery>(mlt->like(inputDocId));
    shared_ptr<deque<std::shared_ptr<BooleanClause>>> clauses =
        query->clauses();

    shared_ptr<deque<std::shared_ptr<BooleanClause>>>
        expectedClothesShopClauses = deque<std::shared_ptr<BooleanClause>>();
    for (auto itemForSale : clothesShopItemForSale) {
      shared_ptr<BooleanClause> booleanClause = make_shared<BooleanClause>(
          make_shared<TermQuery>(make_shared<Term>(FOR_SALE, itemForSale)),
          BooleanClause::Occur::SHOULD);
      expectedClothesShopClauses->add(booleanClause);
    }
    for (auto itemNotForSale : clothesShopItemNotForSale) {
      shared_ptr<BooleanClause> booleanClause =
          make_shared<BooleanClause>(make_shared<TermQuery>(make_shared<Term>(
                                         NOT_FOR_SALE, itemNotForSale)),
                                     BooleanClause::Occur::SHOULD);
      expectedClothesShopClauses->add(booleanClause);
    }

    for (auto expectedClause : expectedClothesShopClauses) {
      assertTrue(clauses->contains(expectedClause));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // clean up
    if (reader != nullptr) {
      delete reader;
    }
    delete dir;
    delete analyzer;
  }
}
} // namespace org::apache::lucene::queries::mlt