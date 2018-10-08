using namespace std;

#include "TestSuggestField.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using Codec = org::apache::lucene::codecs::Codec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Lucene70Codec = org::apache::lucene::codecs::lucene70::Lucene70Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IntPoint = org::apache::lucene::document::IntPoint;
using StoredField = org::apache::lucene::document::StoredField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
using Directory = org::apache::lucene::store::Directory;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
//    import static
//    org.apache.lucene.analysis.BaseTokenStreamTestCase.assertTokenStreamContents;
//    import static
//    org.apache.lucene.search.suggest.document.TopSuggestDocs.SuggestScoreDoc;
//    import static org.hamcrest.core.IsEqual.equalTo;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before() throws Exception
void TestSuggestField::before() ; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws Exception
void TestSuggestField::after()  { delete dir; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptySuggestion() throws Exception
void TestSuggestField::testEmptySuggestion() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SuggestField>(L"suggest_field", L"", 3);
  });
  assertTrue(expected.what()->contains(L"value"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNegativeWeight() throws Exception
void TestSuggestField::testNegativeWeight() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SuggestField>(L"suggest_field", L"sugg", -1);
  });
  assertTrue(expected.what()->contains(L"weight"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReservedChars() throws Exception
void TestSuggestField::testReservedChars() 
{
  shared_ptr<CharsRefBuilder> charsRefBuilder = make_shared<CharsRefBuilder>();
  charsRefBuilder->append(L"sugg");
  charsRefBuilder->setCharAt(
      2, static_cast<wchar_t>(ConcatenateGraphFilter::SEP_LABEL));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SuggestField>(L"name", charsRefBuilder->toString(), 1);
  });
  assertTrue(expected.what()->contains(L"[0x1f]"));

  charsRefBuilder->setCharAt(
      2, static_cast<wchar_t>(CompletionAnalyzer::HOLE_CHARACTER));
  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SuggestField>(L"name", charsRefBuilder->toString(), 1);
  });
  assertTrue(expected.what()->contains(L"[0x1e]"));

  charsRefBuilder->setCharAt(
      2, static_cast<wchar_t>(NRTSuggesterBuilder::END_BYTE));
  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SuggestField>(L"name", charsRefBuilder->toString(), 1);
  });
  assertTrue(expected.what()->contains(L"[0x0]"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmpty() throws Exception
void TestSuggestField::testEmpty() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"ab"));
  shared_ptr<TopSuggestDocs> lookupDocs =
      suggestIndexSearcher->suggest(query, 3, false);
  assertThat(lookupDocs->totalHits, equalTo(0LL));
  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream() throws Exception
void TestSuggestField::testTokenStream() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SuggestField> suggestField =
      make_shared<SuggestField>(L"field", L"input", 1);
  shared_ptr<BytesRef> surfaceForm = make_shared<BytesRef>(L"input");
  shared_ptr<ByteArrayOutputStream> byteArrayOutputStream =
      make_shared<ByteArrayOutputStream>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.store.OutputStreamDataOutput output = new
  // org.apache.lucene.store.OutputStreamDataOutput(byteArrayOutputStream))
  {
    org::apache::lucene::store::OutputStreamDataOutput output =
        org::apache::lucene::store::OutputStreamDataOutput(
            byteArrayOutputStream);
    output->writeVInt(surfaceForm->length);
    output->writeBytes(surfaceForm->bytes, surfaceForm->offset,
                       surfaceForm->length);
    output->writeVInt(1 + 1);
    output->writeByte(SuggestField::TYPE);
  }
  shared_ptr<BytesRef> payload =
      make_shared<BytesRef>(byteArrayOutputStream->toByteArray());
  shared_ptr<TokenStream> stream = make_shared<PayloadAttrToTypeAttrFilter>(
      suggestField->tokenStream(analyzer, nullptr));
  assertTokenStreamContents(stream, std::deque<wstring>{L"input"}, nullptr,
                            nullptr,
                            std::deque<wstring>{payload->utf8ToString()},
                            std::deque<int>{1}, nullptr, nullptr);

  shared_ptr<CompletionAnalyzer> completionAnalyzer =
      make_shared<CompletionAnalyzer>(analyzer);
  stream = make_shared<PayloadAttrToTypeAttrFilter>(
      suggestField->tokenStream(completionAnalyzer, nullptr));
  assertTokenStreamContents(stream, std::deque<wstring>{L"input"}, nullptr,
                            nullptr,
                            std::deque<wstring>{payload->utf8ToString()},
                            std::deque<int>{1}, nullptr, nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDupSuggestFieldValues() throws Exception
void TestSuggestField::testDupSuggestFieldValues() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  constexpr int num = min(1000, atLeast(300));
  std::deque<int> weights(num);
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    weights[i] = random()->nextInt(numeric_limits<int>::max());
    document->push_back(
        make_shared<SuggestField>(L"suggest_field", L"abc", weights[i]));
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  std::deque<std::shared_ptr<Entry>> expectedEntries(num);
  Arrays::sort(weights);
  for (int i = 1; i <= num; i++) {
    expectedEntries[i - 1] = make_shared<Entry>(L"abc", weights[num - i]);
  }

  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc"));
  shared_ptr<TopSuggestDocs> lookupDocs =
      suggestIndexSearcher->suggest(query, num, false);
  assertSuggestions(lookupDocs, expectedEntries);

  reader->close();
  delete iw;
}

void TestSuggestField::testDeduplication() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  constexpr int num = TestUtil::nextInt(random(), 2, 20);
  std::deque<int> weights(num);
  int bestABCWeight = numeric_limits<int>::min();
  int bestABDWeight = numeric_limits<int>::min();
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    weights[i] = random()->nextInt(numeric_limits<int>::max());
    wstring suggestValue;
    bool doABC;
    if (i == 0) {
      doABC = true;
    } else if (i == 1) {
      doABC = false;
    } else {
      doABC = random()->nextBoolean();
    }
    if (doABC) {
      suggestValue = L"abc";
      bestABCWeight = max(bestABCWeight, weights[i]);
    } else {
      suggestValue = L"abd";
      bestABDWeight = max(bestABDWeight, weights[i]);
    }
    document->push_back(
        make_shared<SuggestField>(L"suggest_field", suggestValue, weights[i]));
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  std::deque<std::shared_ptr<Entry>> expectedEntries(2);
  if (bestABDWeight > bestABCWeight) {
    expectedEntries[0] = make_shared<Entry>(L"abd", bestABDWeight);
    expectedEntries[1] = make_shared<Entry>(L"abc", bestABCWeight);
  } else {
    expectedEntries[0] = make_shared<Entry>(L"abc", bestABCWeight);
    expectedEntries[1] = make_shared<Entry>(L"abd", bestABDWeight);
  }

  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"a"));
  shared_ptr<TopSuggestDocsCollector> collector =
      make_shared<TopSuggestDocsCollector>(2, true);
  suggestIndexSearcher->suggest(query, collector);
  shared_ptr<TopSuggestDocs> lookupDocs = collector->get();
  assertSuggestions(lookupDocs, expectedEntries);

  reader->close();
  delete iw;
}

void TestSuggestField::testExtremeDeduplication() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  constexpr int num = atLeast(5000);
  int bestWeight = numeric_limits<int>::min();
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    int weight = TestUtil::nextInt(random(), 10, 100);
    bestWeight = max(weight, bestWeight);
    document->push_back(
        make_shared<SuggestField>(L"suggest_field", L"abc", weight));
    iw->addDocument(document);
    if (rarely()) {
      iw->commit();
    }
  }
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abd", 7));
  iw->addDocument(document);

  if (random()->nextBoolean()) {
    iw->forceMerge(1);
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  std::deque<std::shared_ptr<Entry>> expectedEntries(2);
  expectedEntries[0] = make_shared<Entry>(L"abc", bestWeight);
  expectedEntries[1] = make_shared<Entry>(L"abd", 7);

  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"a"));
  shared_ptr<TopSuggestDocsCollector> collector =
      make_shared<TopSuggestDocsCollector>(2, true);
  suggestIndexSearcher->suggest(query, collector);
  shared_ptr<TopSuggestDocs> lookupDocs = collector->get();
  assertSuggestions(lookupDocs, expectedEntries);

  reader->close();
  delete iw;
}

wstring TestSuggestField::randomSimpleString(int numDigits, int maxLen)
{
  constexpr int len = TestUtil::nextInt(random(), 1, maxLen);
  const std::deque<wchar_t> chars = std::deque<wchar_t>(len);
  for (int j = 0; j < len; j++) {
    chars[j] = static_cast<wchar_t>(L'a' + random()->nextInt(numDigits));
  }
  return wstring(chars);
}

void TestSuggestField::testRandom() 
{
  int numDigits = TestUtil::nextInt(random(), 1, 6);
  shared_ptr<Set<wstring>> keys = unordered_set<wstring>();
  int keyCount = TestUtil::nextInt(random(), 1, 20);
  if (numDigits == 1) {
    keyCount = min(9, keyCount);
  }
  while (keys->size() < keyCount) {
    keys->add(randomSimpleString(numDigits, 10));
  }
  deque<wstring> keysList = deque<wstring>(keys);

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc =
      iwcWithSuggestField(analyzer, {L"suggest_field"});
  // we rely on docID order:
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  int docCount = TestUtil::nextInt(random(), 1, 200);
  std::deque<std::shared_ptr<Entry>> docs(docCount);
  for (int i = 0; i < docCount; i++) {
    int weight = random()->nextInt(40);
    wstring key = keysList[random()->nextInt(keyCount)];
    // System.out.println("KEY: " + key);
    docs[i] = make_shared<Entry>(key, nullptr, weight, i);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<SuggestField>(L"suggest_field", key, weight));
    iw->addDocument(doc);
    if (usually()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> searcher =
      make_shared<SuggestIndexSearcher>(reader);

  int iters = atLeast(200);
  for (int iter = 0; iter < iters; iter++) {
    wstring prefix = randomSimpleString(numDigits, 2);
    if (VERBOSE) {
      wcout << L"\nTEST: prefix=" << prefix << endl;
    }

    // slow but hopefully correct suggester:
    deque<std::shared_ptr<Entry>> expected = deque<std::shared_ptr<Entry>>();
    for (auto doc : docs) {
      if (StringHelper::startsWith(doc->output, prefix)) {
        expected.push_back(doc);
      }
    }
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expected, new
    // java.util.Comparator<Entry>()
    sort(expected.begin(), expected.end(),
         make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

    bool dedup = random()->nextBoolean();
    if (dedup) {
      deque<std::shared_ptr<Entry>> deduped = deque<std::shared_ptr<Entry>>();
      shared_ptr<Set<wstring>> seen = unordered_set<wstring>();
      for (auto entry : expected) {
        if (seen->contains(entry->output) == false) {
          seen->add(entry->output);
          deduped.push_back(entry);
        }
      }
      expected = deduped;
    }

    // TODO: re-enable this, except something is buggy about tie breaks at the
    // topN threshold now:
    // int topN = TestUtil.nextInt(random(), 1, docCount+10);
    int topN = docCount;

    if (VERBOSE) {
      if (dedup) {
        wcout << L"  expected (dedup'd) topN=" << topN << L":" << endl;
      } else {
        wcout << L"  expected topN=" << topN << L":" << endl;
      }
      for (int i = 0; i < expected.size(); i++) {
        if (i >= topN) {
          wcout << L"    leftover: " << i << L": " << expected[i] << endl;
        } else {
          wcout << L"    " << i << L": " << expected[i] << endl;
        }
      }
    }
    expected = expected.subList(0, min(topN, expected.size()));

    shared_ptr<PrefixCompletionQuery> query =
        make_shared<PrefixCompletionQuery>(
            analyzer, make_shared<Term>(L"suggest_field", prefix));
    shared_ptr<TopSuggestDocsCollector> collector =
        make_shared<TopSuggestDocsCollector>(topN, dedup);
    searcher->suggest(query, collector);
    shared_ptr<TopSuggestDocs> actual = collector->get();
    if (VERBOSE) {
      wcout << L"  actual:" << endl;
      std::deque<std::shared_ptr<SuggestScoreDoc>> suggestScoreDocs =
          static_cast<std::deque<std::shared_ptr<SuggestScoreDoc>>>(
              actual->scoreDocs);
      for (int i = 0; i < suggestScoreDocs.size(); i++) {
        wcout << L"    " << i << L": " << suggestScoreDocs[i] << endl;
      }
    }

    assertSuggestions(
        actual, {expected.toArray(
                    std::deque<std::shared_ptr<Entry>>(expected.size()))});
  }

  reader->close();
  delete iw;
}

TestSuggestField::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestSuggestField> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestSuggestField::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Entry> a, shared_ptr<Entry> b)
{
  // sort by higher score:
  int cmp = Float::compare(b->value, a->value);
  if (cmp == 0) {
    // tie break by smaller docID:
    cmp = Integer::compare(a->id, b->id);
  }
  return cmp;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNRTDeletedDocFiltering() throws
// Exception
void TestSuggestField::testNRTDeletedDocFiltering() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  // using IndexWriter instead of RandomIndexWriter
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));

  int num = min(1000, atLeast(10));

  int numLive = 0;
  deque<std::shared_ptr<Entry>> expectedEntries =
      deque<std::shared_ptr<Entry>>();
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(
        L"suggest_field", L"abc_" + to_wstring(i), num - i));
    if (i % 2 == 0) {
      document->push_back(
          newStringField(L"str_field", L"delete", Field::Store::YES));
    } else {
      numLive++;
      expectedEntries.push_back(
          make_shared<Entry>(L"abc_" + to_wstring(i), num - i));
      document->push_back(
          newStringField(L"str_field", L"no_delete", Field::Store::YES));
    }
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  iw->deleteDocuments({make_shared<Term>(L"str_field", L"delete")});

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"));
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, numLive, false);
  assertSuggestions(
      suggest, {expectedEntries.toArray(std::deque<std::shared_ptr<Entry>>(
                   expectedEntries.size()))});

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSuggestOnAllFilteredDocuments() throws
// Exception
void TestSuggestField::testSuggestOnAllFilteredDocuments() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  int num = min(1000, atLeast(10));
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(L"suggest_field",
                                                  L"abc_" + to_wstring(i), i));
    document->push_back(
        newStringField(L"str_fld", L"deleted", Field::Store::NO));
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  shared_ptr<BitsProducer> filter =
      make_shared<BitsProducerAnonymousInnerClass>(shared_from_this());
  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  // no random access required;
  // calling suggest with filter that does not match any documents should early
  // terminate
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"), filter);
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, num, false);
  assertThat(suggest->totalHits, equalTo(0LL));
  reader->close();
  delete iw;
}

TestSuggestField::BitsProducerAnonymousInnerClass::
    BitsProducerAnonymousInnerClass(shared_ptr<TestSuggestField> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Bits> TestSuggestField::BitsProducerAnonymousInnerClass::getBits(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<Bits::MatchNoBits>(context->reader()->maxDoc());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSuggestOnAllDeletedDocuments() throws
// Exception
void TestSuggestField::testSuggestOnAllDeletedDocuments() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  // using IndexWriter instead of RandomIndexWriter
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  int num = min(1000, atLeast(10));
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(L"suggest_field",
                                                  L"abc_" + to_wstring(i), i));
    document->push_back(newStringField(L"delete", L"delete", Field::Store::NO));
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  iw->deleteDocuments({make_shared<Term>(L"delete", L"delete")});

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"));
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, num, false);
  assertThat(suggest->totalHits, equalTo(0LL));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSuggestOnMostlyDeletedDocuments() throws
// Exception
void TestSuggestField::testSuggestOnMostlyDeletedDocuments() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  // using IndexWriter instead of RandomIndexWriter
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  int num = min(1000, atLeast(10));
  for (int i = 1; i <= num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(L"suggest_field",
                                                  L"abc_" + to_wstring(i), i));
    document->push_back(make_shared<StoredField>(L"weight_fld", i));
    document->push_back(make_shared<IntPoint>(L"weight_fld", i));
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  iw->deleteDocuments(
      {IntPoint::newRangeQuery(L"weight_fld", 2, numeric_limits<int>::max())});

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"));
  shared_ptr<TopSuggestDocs> suggest = indexSearcher->suggest(query, 1, false);
  assertSuggestions(suggest, {make_shared<Entry>(L"abc_1", 1)});

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultipleSuggestFieldsPerDoc() throws
// Exception
void TestSuggestField::testMultipleSuggestFieldsPerDoc() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      iwcWithSuggestField(analyzer, {L"sug_field_1", L"sug_field_2"}));

  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(L"sug_field_1", L"apple", 4));
  document->push_back(make_shared<SuggestField>(L"sug_field_2", L"april", 3));
  iw->addDocument(document);
  document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(L"sug_field_1", L"aples", 3));
  document->push_back(
      make_shared<SuggestField>(L"sug_field_2", L"apartment", 2));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();

  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"sug_field_1", L"ap"));
  shared_ptr<TopSuggestDocs> suggestDocs1 =
      suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggestDocs1, {make_shared<Entry>(L"apple", 4),
                                   make_shared<Entry>(L"aples", 3)});
  query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"sug_field_2", L"ap"));
  shared_ptr<TopSuggestDocs> suggestDocs2 =
      suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggestDocs2, {make_shared<Entry>(L"april", 3),
                                   make_shared<Entry>(L"apartment", 2)});

  // check that the doc ids are consistent
  for (int i = 0; i < suggestDocs1->scoreDocs.size(); i++) {
    shared_ptr<ScoreDoc> suggestScoreDoc = suggestDocs1->scoreDocs[i];
    assertThat(suggestScoreDoc->doc, equalTo(suggestDocs2->scoreDocs[i]->doc));
  }

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEarlyTermination() throws Exception
void TestSuggestField::testEarlyTermination() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  int num = min(1000, atLeast(10));

  // have segments of 4 documents
  // with descending suggestion weights
  // suggest should early terminate for
  // segments with docs having lower suggestion weights
  for (int i = num; i > 0; i--) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(L"suggest_field",
                                                  L"abc_" + to_wstring(i), i));
    iw->addDocument(document);
    if (i % 4 == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"));
  shared_ptr<TopSuggestDocs> suggest = indexSearcher->suggest(query, 1, false);
  assertSuggestions(suggest,
                    {make_shared<Entry>(L"abc_" + to_wstring(num), num)});

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultipleSegments() throws Exception
void TestSuggestField::testMultipleSegments() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  int num = min(1000, atLeast(10));
  deque<std::shared_ptr<Entry>> entries = deque<std::shared_ptr<Entry>>();

  // ensure at least some segments have no suggest field
  for (int i = num; i > 0; i--) {
    shared_ptr<Document> document = make_shared<Document>();
    if (random()->nextInt(4) == 1) {
      document->push_back(make_shared<SuggestField>(
          L"suggest_field", L"abc_" + to_wstring(i), i));
      entries.push_back(make_shared<Entry>(L"abc_" + to_wstring(i), i));
    }
    document->push_back(make_shared<StoredField>(L"weight_fld", i));
    iw->addDocument(document);
    if (usually()) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"));
  shared_ptr<TopSuggestDocs> suggest = indexSearcher->suggest(
      query, (entries.empty()) ? 1 : entries.size(), false);
  assertSuggestions(
      suggest,
      {entries.toArray(std::deque<std::shared_ptr<Entry>>(entries.size()))});

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReturnedDocID() throws Exception
void TestSuggestField::testReturnedDocID() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));

  int num = min(1000, atLeast(10));
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<SuggestField>(
        L"suggest_field", L"abc_" + to_wstring(i), num));
    document->push_back(make_shared<StoredField>(L"int_field", i));
    iw->addDocument(document);

    if (random()->nextBoolean()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<PrefixCompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"abc_"));
  shared_ptr<TopSuggestDocs> suggest =
      indexSearcher->suggest(query, num, false);
  TestUtil::assertEquals(num, suggest->totalHits);
  for (auto suggestScoreDoc : suggest->scoreLookupDocs()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring key = suggestScoreDoc->key->toString();
    assertTrue(StringHelper::startsWith(key, L"abc_"));
    wstring substring = key.substr(4);
    int fieldValue = stoi(substring);
    shared_ptr<Document> doc = reader->document(suggestScoreDoc->doc);
    TestUtil::assertEquals(
        doc->getField(L"int_field")->numericValue()->intValue(), fieldValue);
  }

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testScoring() throws Exception
void TestSuggestField::testScoring() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));

  int num = min(1000, atLeast(100));
  std::deque<wstring> prefixes = {L"abc", L"bac", L"cab"};
  unordered_map<wstring, int> mappings = unordered_map<wstring, int>();
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    wstring suggest = prefixes[i % 3] +
                      TestUtil::randomSimpleString(random(), 10) + L"_" +
                      to_wstring(i);
    int weight = random()->nextInt(numeric_limits<int>::max());
    document->push_back(
        make_shared<SuggestField>(L"suggest_field", suggest, weight));
    mappings.emplace(suggest, weight);
    iw->addDocument(document);

    if (usually()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  for (auto prefix : prefixes) {
    shared_ptr<PrefixCompletionQuery> query =
        make_shared<PrefixCompletionQuery>(
            analyzer, make_shared<Term>(L"suggest_field", prefix));
    shared_ptr<TopSuggestDocs> suggest =
        indexSearcher->suggest(query, num, false);
    assertTrue(suggest->totalHits > 0);
    float topScore = -1;
    for (auto scoreDoc : suggest->scoreLookupDocs()) {
      if (topScore != -1) {
        assertTrue(topScore >= scoreDoc->score);
      }
      topScore = scoreDoc->score;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertThat(static_cast<float>(mappings[scoreDoc->key->toString()]),
                 equalTo(scoreDoc->score));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertNotNull(mappings.erase(scoreDoc->key->toString()));
    }
  }

  assertThat(mappings.size(), equalTo(0));
  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRealisticKeys() throws Exception
void TestSuggestField::testRealisticKeys() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, {L"suggest_field"}));
  shared_ptr<LineFileDocs> lineFileDocs = make_shared<LineFileDocs>(random());
  int num = min(1000, atLeast(100));
  unordered_map<wstring, int> mappings = unordered_map<wstring, int>();
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> document = lineFileDocs->nextDoc();
    wstring title = document->getField(L"title").stringValue();
    int maxLen = min(title.length(), 500);
    wstring prefix = title.substr(0, maxLen);
    int weight = random()->nextInt(numeric_limits<int>::max());
    optional<int> prevWeight = mappings[prefix];
    if (!prevWeight || prevWeight < weight) {
      mappings.emplace(prefix, weight);
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<SuggestField>(L"suggest_field", prefix, weight));
    iw->addDocument(doc);

    if (rarely()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);

  for (auto entry : mappings) {
    wstring title = entry.first;

    shared_ptr<PrefixCompletionQuery> query =
        make_shared<PrefixCompletionQuery>(
            analyzer, make_shared<Term>(L"suggest_field", title));
    shared_ptr<TopSuggestDocs> suggest =
        indexSearcher->suggest(query, mappings.size(), false);
    assertTrue(suggest->totalHits > 0);
    bool matched = false;
    for (auto scoreDoc : suggest->scoreDocs) {
      matched = Float::compare(scoreDoc->score,
                               static_cast<float>(entry.second)) == 0;
      if (matched) {
        break;
      }
    }
    assertTrue(L"at least one of the entries should have the score", matched);
  }

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testThreads() throws Exception
void TestSuggestField::testThreads() 
{
  shared_ptr<Analyzer> *const analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      iwcWithSuggestField(analyzer, {L"suggest_field_1", L"suggest_field_2",
                                     L"suggest_field_3"}));
  int num = min(1000, atLeast(100));
  const wstring prefix1 = L"abc1_";
  const wstring prefix2 = L"abc2_";
  const wstring prefix3 = L"abc3_";
  std::deque<std::shared_ptr<Entry>> entries1(num);
  std::deque<std::shared_ptr<Entry>> entries2(num);
  std::deque<std::shared_ptr<Entry>> entries3(num);
  for (int i = 0; i < num; i++) {
    int weight = num - (i + 1);
    entries1[i] = make_shared<Entry>(prefix1 + to_wstring(weight), weight);
    entries2[i] = make_shared<Entry>(prefix2 + to_wstring(weight), weight);
    entries3[i] = make_shared<Entry>(prefix3 + to_wstring(weight), weight);
  }
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<SuggestField>(L"suggest_field_1",
                                             prefix1 + to_wstring(i), i));
    doc->push_back(make_shared<SuggestField>(L"suggest_field_2",
                                             prefix2 + to_wstring(i), i));
    doc->push_back(make_shared<SuggestField>(L"suggest_field_3",
                                             prefix3 + to_wstring(i), i));
    iw->addDocument(doc);

    if (rarely()) {
      iw->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  int numThreads = TestUtil::nextInt(random(), 2, 7);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  shared_ptr<CyclicBarrier> *const startingGun =
      make_shared<CyclicBarrier>(numThreads + 1);
  shared_ptr<CopyOnWriteArrayList<runtime_error>> *const errors =
      make_shared<CopyOnWriteArrayList<runtime_error>>();
  shared_ptr<SuggestIndexSearcher> *const indexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), analyzer, num, prefix1, prefix2, prefix3, entries1,
        entries2, entries3, startingGun, errors, indexSearcher);
    threads[i]->start();
  }

  startingGun->await();
  for (auto t : threads) {
    t->join();
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(errors->toString(), errors->isEmpty());

  reader->close();
  delete iw;
}

TestSuggestField::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestSuggestField> outerInstance, shared_ptr<Analyzer> analyzer,
    int num, const wstring &prefix1, const wstring &prefix2,
    const wstring &prefix3,
    deque<std::shared_ptr<org::apache::lucene::search::suggest::document::
                               TestSuggestField::Entry>> &entries1,
    deque<std::shared_ptr<org::apache::lucene::search::suggest::document::
                               TestSuggestField::Entry>> &entries2,
    deque<std::shared_ptr<org::apache::lucene::search::suggest::document::
                               TestSuggestField::Entry>> &entries3,
    shared_ptr<CyclicBarrier> startingGun,
    shared_ptr<CopyOnWriteArrayList<runtime_error>> errors,
    shared_ptr<
        org::apache::lucene::search::suggest::document::SuggestIndexSearcher>
        indexSearcher)
{
  this->outerInstance = outerInstance;
  this->analyzer = analyzer;
  this->num = num;
  this->prefix1 = prefix1;
  this->prefix2 = prefix2;
  this->prefix3 = prefix3;
  this->entries1 = entries1;
  this->entries2 = entries2;
  this->entries3 = entries3;
  this->startingGun = startingGun;
  this->errors = errors;
  this->indexSearcher = indexSearcher;
}

void TestSuggestField::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    shared_ptr<PrefixCompletionQuery> query =
        make_shared<PrefixCompletionQuery>(
            analyzer, make_shared<Term>(L"suggest_field_1", prefix1));
    shared_ptr<TopSuggestDocs> suggest =
        indexSearcher->suggest(query, num, false);
    assertSuggestions(suggest, entries1);
    query = make_shared<PrefixCompletionQuery>(
        analyzer, make_shared<Term>(L"suggest_field_2", prefix2));
    suggest = indexSearcher->suggest(query, num, false);
    assertSuggestions(suggest, entries2);
    query = make_shared<PrefixCompletionQuery>(
        analyzer, make_shared<Term>(L"suggest_field_3", prefix3));
    suggest = indexSearcher->suggest(query, num, false);
    assertSuggestions(suggest, entries3);
  } catch (const runtime_error &e) {
    errors->add(e);
  }
}

TestSuggestField::Entry::Entry(const wstring &output, float value)
    : Entry(output, nullptr, value)
{
}

TestSuggestField::Entry::Entry(const wstring &output, const wstring &context,
                               float value)
    : Entry(output, context, value, -1)
{
}

TestSuggestField::Entry::Entry(const wstring &output, const wstring &context,
                               float value, int id)
    : output(output), value(value), context(context), id(id)
{
}

wstring TestSuggestField::Entry::toString()
{
  return L"key=" + output + L" score=" + to_wstring(value) + L" context=" +
         context + L" id=" + to_wstring(id);
}

void TestSuggestField::assertSuggestions(shared_ptr<TopDocs> actual,
                                         deque<Entry> &expected)
{
  std::deque<std::shared_ptr<SuggestScoreDoc>> suggestScoreDocs =
      static_cast<std::deque<std::shared_ptr<SuggestScoreDoc>>>(
          actual->scoreDocs);
  for (int i = 0; i < min(expected->length, suggestScoreDocs.size()); i++) {
    shared_ptr<SuggestScoreDoc> lookupDoc = suggestScoreDocs[i];
    wstring msg = L"Hit " + to_wstring(i) + L": expected: " +
                  toString(expected[i]) + L" but actual: " +
                  toString(lookupDoc);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertThat(msg, lookupDoc->key->toString(), equalTo(expected[i].output));
    assertThat(msg, lookupDoc->score, equalTo(expected[i].value));
    assertThat(msg, lookupDoc->context, equalTo(expected[i].context));
  }
  assertThat(suggestScoreDocs.size(), equalTo(expected->length));
}

wstring TestSuggestField::toString(shared_ptr<Entry> expected)
{
  return L"key:" + expected->output + L" score:" + to_wstring(expected->value) +
         L" context:" + expected->context;
}

wstring TestSuggestField::toString(shared_ptr<SuggestScoreDoc> actual)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"key:" + actual->key->toString() + L" score:" + actual->score +
         L" context:" + actual->context;
}

shared_ptr<IndexWriterConfig>
TestSuggestField::iwcWithSuggestField(shared_ptr<Analyzer> analyzer,
                                      deque<wstring> &suggestFields)
{
  return iwcWithSuggestField(analyzer, asSet({suggestFields}));
}

shared_ptr<IndexWriterConfig>
TestSuggestField::iwcWithSuggestField(shared_ptr<Analyzer> analyzer,
                                      shared_ptr<Set<wstring>> suggestFields)
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<Codec> filterCodec =
      make_shared<Lucene70CodecAnonymousInnerClass>(suggestFields);
  iwc->setCodec(filterCodec);
  return iwc;
}

TestSuggestField::Lucene70CodecAnonymousInnerClass::
    Lucene70CodecAnonymousInnerClass(shared_ptr<Set<wstring>> suggestFields)
{
  this->suggestFields = suggestFields;
}

shared_ptr<PostingsFormat>
TestSuggestField::Lucene70CodecAnonymousInnerClass::getPostingsFormatForField(
    const wstring &field)
{
  if (suggestFields->contains(field)) {
    return postingsFormat;
  }
  return outerInstance->super->getPostingsFormatForField(field);
}

TestSuggestField::PayloadAttrToTypeAttrFilter::PayloadAttrToTypeAttrFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestSuggestField::PayloadAttrToTypeAttrFilter::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    // we move them over so we can assert them more easily in the tests
    type->setType(payload->getPayload()->utf8ToString());
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::search::suggest::document