using namespace std;

#include "TestMemoryIndexAgainstRAMDir.h"

namespace org::apache::lucene::index::memory
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using namespace org::apache::lucene::index;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using Allocator = org::apache::lucene::util::ByteBlockPool::Allocator;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using RecyclingByteBlockAllocator =
    org::apache::lucene::util::RecyclingByteBlockAllocator;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.hamcrest.CoreMatchers.equalTo;

void TestMemoryIndexAgainstRAMDir::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  queries->addAll(readQueries(L"testqueries.txt"));
  queries->addAll(readQueries(L"testqueries2.txt"));
}

shared_ptr<Set<wstring>> TestMemoryIndexAgainstRAMDir::readQueries(
    const wstring &resource) 
{
  shared_ptr<Set<wstring>> queries = unordered_set<wstring>();
  shared_ptr<InputStream> stream = getClass().getResourceAsStream(resource);
  shared_ptr<BufferedReader> reader = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(stream, StandardCharsets::UTF_8));
  wstring line = L"";
  while ((line = reader->readLine()) != L"") {
    line = StringHelper::trim(line);
    if (line.length() > 0 && !StringHelper::startsWith(line, L"#") &&
        !StringHelper::startsWith(line, L"//")) {
      queries->add(line);
    }
  }
  return queries;
}

void TestMemoryIndexAgainstRAMDir::testRandomQueries() 
{
  shared_ptr<MemoryIndex> index = randomMemoryIndex();
  for (int i = 0; i < ITERATIONS; i++) {
    assertAgainstRAMDirectory(index);
  }
}

void TestMemoryIndexAgainstRAMDir::assertAgainstRAMDirectory(
    shared_ptr<MemoryIndex> memory) 
{
  memory->reset();
  shared_ptr<StringBuilder> fooField = make_shared<StringBuilder>();
  shared_ptr<StringBuilder> termField = make_shared<StringBuilder>();

  // add up to 250 terms to field "foo"
  constexpr int numFooTerms = random()->nextInt(250 * RANDOM_MULTIPLIER);
  for (int i = 0; i < numFooTerms; i++) {
    fooField->append(L" ");
    fooField->append(randomTerm());
  }

  // add up to 250 terms to field "term"
  constexpr int numTermTerms = random()->nextInt(250 * RANDOM_MULTIPLIER);
  for (int i = 0; i < numTermTerms; i++) {
    termField->append(L" ");
    termField->append(randomTerm());
  }

  shared_ptr<Directory> ramdir = make_shared<RAMDirectory>();
  shared_ptr<Analyzer> analyzer = randomAnalyzer();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ramdir, (make_shared<IndexWriterConfig>(analyzer))
                  ->setCodec(TestUtil::alwaysPostingsFormat(
                      TestUtil::getDefaultPostingsFormat())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field1 =
      newTextField(L"foo", fooField->toString(), Field::Store::NO);
  shared_ptr<Field> field2 =
      newTextField(L"term", termField->toString(), Field::Store::NO);
  doc->push_back(field1);
  doc->push_back(field2);
  writer->addDocument(doc);
  delete writer;

  memory->addField(L"foo", fooField->toString(), analyzer);
  memory->addField(L"term", termField->toString(), analyzer);

  shared_ptr<LeafReader> reader = std::static_pointer_cast<LeafReader>(
      memory->createSearcher().getIndexReader());
  TestUtil::checkReader(reader);
  shared_ptr<DirectoryReader> competitor = DirectoryReader::open(ramdir);
  duellReaders(competitor, reader);
  IOUtils::close({reader, competitor});
  assertAllQueries(memory, ramdir, analyzer);
  delete ramdir;
}

void TestMemoryIndexAgainstRAMDir::duellReaders(
    shared_ptr<CompositeReader> other,
    shared_ptr<LeafReader> memIndexReader) 
{
  shared_ptr<Fields> memFields = memIndexReader->getTermVectors(0);
  for (auto field : MultiFields::getFields(other)) {
    shared_ptr<Terms> memTerms = memFields->terms(field);
    shared_ptr<Terms> iwTerms = memIndexReader->terms(field);
    if (iwTerms == nullptr) {
      assertNull(memTerms);
    } else {
      shared_ptr<NumericDocValues> normValues =
          MultiDocValues::getNormValues(other, field);
      shared_ptr<NumericDocValues> memNormValues =
          memIndexReader->getNormValues(field);
      if (normValues != nullptr) {
        // mem idx always computes norms on the fly
        assertNotNull(memNormValues);
        TestUtil::assertEquals(0, normValues->nextDoc());
        TestUtil::assertEquals(0, memNormValues->nextDoc());
        TestUtil::assertEquals(normValues->longValue(),
                               memNormValues->longValue());
      }

      assertNotNull(memTerms);
      TestUtil::assertEquals(iwTerms->getDocCount(), memTerms->getDocCount());
      TestUtil::assertEquals(iwTerms->getSumDocFreq(),
                             memTerms->getSumDocFreq());
      TestUtil::assertEquals(iwTerms->getSumTotalTermFreq(),
                             memTerms->getSumTotalTermFreq());
      shared_ptr<TermsEnum> iwTermsIter = iwTerms->begin();
      shared_ptr<TermsEnum> memTermsIter = memTerms->begin();
      if (iwTerms->hasPositions()) {
        constexpr bool offsets =
            iwTerms->hasOffsets() && memTerms->hasOffsets();

        while (iwTermsIter->next() != nullptr) {
          assertNotNull(memTermsIter->next());
          TestUtil::assertEquals(iwTermsIter->term(), memTermsIter->term());
          shared_ptr<PostingsEnum> iwDocsAndPos =
              iwTermsIter->postings(nullptr, PostingsEnum::ALL);
          shared_ptr<PostingsEnum> memDocsAndPos =
              memTermsIter->postings(nullptr, PostingsEnum::ALL);
          while (iwDocsAndPos->nextDoc() != PostingsEnum::NO_MORE_DOCS) {
            TestUtil::assertEquals(iwDocsAndPos->docID(),
                                   memDocsAndPos->nextDoc());
            TestUtil::assertEquals(iwDocsAndPos->freq(), memDocsAndPos->freq());
            for (int i = 0; i < iwDocsAndPos->freq(); i++) {
              assertEquals(L"term: " + iwTermsIter->term()->utf8ToString(),
                           iwDocsAndPos->nextPosition(),
                           memDocsAndPos->nextPosition());
              if (offsets) {
                TestUtil::assertEquals(iwDocsAndPos->startOffset(),
                                       memDocsAndPos->startOffset());
                TestUtil::assertEquals(iwDocsAndPos->endOffset(),
                                       memDocsAndPos->endOffset());
              }

              if (iwTerms->hasPayloads()) {
                TestUtil::assertEquals(iwDocsAndPos->getPayload(),
                                       memDocsAndPos->getPayload());
              }
            }
          }
        }
      } else {
        while (iwTermsIter->next() != nullptr) {
          TestUtil::assertEquals(iwTermsIter->term(), memTermsIter->term());
          shared_ptr<PostingsEnum> iwDocsAndPos =
              iwTermsIter->postings(nullptr);
          shared_ptr<PostingsEnum> memDocsAndPos =
              memTermsIter->postings(nullptr);
          while (iwDocsAndPos->nextDoc() != PostingsEnum::NO_MORE_DOCS) {
            TestUtil::assertEquals(iwDocsAndPos->docID(),
                                   memDocsAndPos->nextDoc());
            TestUtil::assertEquals(iwDocsAndPos->freq(), memDocsAndPos->freq());
          }
        }
      }
    }
  }
}

void TestMemoryIndexAgainstRAMDir::assertAllQueries(
    shared_ptr<MemoryIndex> memory, shared_ptr<Directory> ramdir,
    shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(ramdir);
  shared_ptr<IndexSearcher> ram = newSearcher(reader);
  shared_ptr<IndexSearcher> mem = memory->createSearcher();
  shared_ptr<QueryParser> qp = make_shared<QueryParser>(L"foo", analyzer);
  for (auto query : queries) {
    shared_ptr<TopDocs> ramDocs = ram->search(qp->parse(query), 1);
    shared_ptr<TopDocs> memDocs = mem->search(qp->parse(query), 1);
    assertEquals(query, ramDocs->totalHits, memDocs->totalHits);
  }
  delete reader;
}

shared_ptr<Analyzer> TestMemoryIndexAgainstRAMDir::randomAnalyzer()
{
  switch (random()->nextInt(4)) {
  case 0:
    return make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  case 1:
    return make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                     MockTokenFilter::ENGLISH_STOPSET);
  case 2:
    return make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  default:
    return make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE,
                                     false);
  }
}

TestMemoryIndexAgainstRAMDir::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestMemoryIndexAgainstRAMDir> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMemoryIndexAgainstRAMDir::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CrazyTokenFilter>(tokenizer));
}

TestMemoryIndexAgainstRAMDir::CrazyTokenFilter::CrazyTokenFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestMemoryIndexAgainstRAMDir::CrazyTokenFilter::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    if (termAtt->length() > 0 && termAtt->buffer()[0] == L't') {
      termAtt->setLength(0);
    }
    return true;
  } else {
    return false;
  }
}

std::deque<wstring> const TestMemoryIndexAgainstRAMDir::TEST_TERMS = {
    L"term", L"Term",   L"tErm",     L"TERM",      L"telm",     L"stop",
    L"drop", L"roll",   L"phrase",   L"a",         L"c",        L"bar",
    L"blar", L"gack",   L"weltbank", L"worlbank",  L"hello",    L"on",
    L"the",  L"apache", L"Apache",   L"copyright", L"Copyright"};

wstring TestMemoryIndexAgainstRAMDir::randomTerm()
{
  if (random()->nextBoolean()) {
    // return a random TEST_TERM
    return TEST_TERMS[random()->nextInt(TEST_TERMS.size())];
  } else {
    // return a random unicode term
    return TestUtil::randomUnicodeString(random());
  }
}

void TestMemoryIndexAgainstRAMDir::testDocsEnumStart() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<MemoryIndex> memory = make_shared<MemoryIndex>(
      random()->nextBoolean(), false, random()->nextInt(50) * 1024 * 1024);
  memory->addField(L"foo", L"bar", analyzer);
  shared_ptr<LeafReader> reader = std::static_pointer_cast<LeafReader>(
      memory->createSearcher().getIndexReader());
  TestUtil::checkReader(reader);
  shared_ptr<PostingsEnum> disi =
      TestUtil::docs(random(), reader, L"foo", make_shared<BytesRef>(L"bar"),
                     nullptr, PostingsEnum::NONE);
  int docid = disi->docID();
  TestUtil::assertEquals(-1, docid);
  assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  // now reuse and check again
  shared_ptr<TermsEnum> te = reader->terms(L"foo")->begin();
  assertTrue(te->seekExact(make_shared<BytesRef>(L"bar")));
  disi = te->postings(disi, PostingsEnum::NONE);
  docid = disi->docID();
  TestUtil::assertEquals(-1, docid);
  assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  delete reader;
}

shared_ptr<ByteBlockPool::Allocator>
TestMemoryIndexAgainstRAMDir::randomByteBlockAllocator()
{
  if (random()->nextBoolean()) {
    return make_shared<RecyclingByteBlockAllocator>();
  } else {
    return make_shared<ByteBlockPool::DirectAllocator>();
  }
}

shared_ptr<MemoryIndex> TestMemoryIndexAgainstRAMDir::randomMemoryIndex()
{
  return make_shared<MemoryIndex>(random()->nextBoolean(),
                                  random()->nextBoolean(),
                                  random()->nextInt(50) * 1024 * 1024);
}

void TestMemoryIndexAgainstRAMDir::testDocsAndPositionsEnumStart() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  int numIters = atLeast(3);
  shared_ptr<MemoryIndex> memory = make_shared<MemoryIndex>(
      true, false, random()->nextInt(50) * 1024 * 1024);
  for (int i = 0; i < numIters; i++) { // check reuse
    memory->addField(L"foo", L"bar", analyzer);
    shared_ptr<LeafReader> reader = std::static_pointer_cast<LeafReader>(
        memory->createSearcher().getIndexReader());
    TestUtil::checkReader(reader);
    TestUtil::assertEquals(1, reader->terms(L"foo")->getSumTotalTermFreq());
    shared_ptr<PostingsEnum> disi =
        reader->postings(make_shared<Term>(L"foo", L"bar"), PostingsEnum::ALL);
    int docid = disi->docID();
    TestUtil::assertEquals(-1, docid);
    assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    TestUtil::assertEquals(0, disi->nextPosition());
    TestUtil::assertEquals(0, disi->startOffset());
    TestUtil::assertEquals(3, disi->endOffset());

    // now reuse and check again
    shared_ptr<TermsEnum> te = reader->terms(L"foo")->begin();
    assertTrue(te->seekExact(make_shared<BytesRef>(L"bar")));
    disi = te->postings(disi);
    docid = disi->docID();
    TestUtil::assertEquals(-1, docid);
    assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    delete reader;
    memory->reset();
  }
}

void TestMemoryIndexAgainstRAMDir::testNullPointerException() 
{
  shared_ptr<RegexpQuery> regex =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"worl."));
  shared_ptr<SpanQuery> wrappedquery =
      make_shared<SpanMultiTermQueryWrapper<>>(regex);

  shared_ptr<MemoryIndex> mindex = randomMemoryIndex();
  mindex->addField(L"field", (make_shared<MockAnalyzer>(random()))
                                 ->tokenStream(L"field", L"hello there"));

  // This throws an NPE
  assertEquals(0, mindex->search(wrappedquery), 0.00001f);
  TestUtil::checkReader(mindex->createSearcher()->getIndexReader());
}

void TestMemoryIndexAgainstRAMDir::testPassesIfWrapped() 
{
  shared_ptr<RegexpQuery> regex =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"worl."));
  shared_ptr<SpanQuery> wrappedquery =
      make_shared<SpanOrQuery>(make_shared<SpanMultiTermQueryWrapper<>>(regex));

  shared_ptr<MemoryIndex> mindex = randomMemoryIndex();
  mindex->addField(L"field", (make_shared<MockAnalyzer>(random()))
                                 ->tokenStream(L"field", L"hello there"));

  // This passes though
  assertEquals(0, mindex->search(wrappedquery), 0.00001f);
  TestUtil::checkReader(mindex->createSearcher()->getIndexReader());
}

void TestMemoryIndexAgainstRAMDir::testSameFieldAddedMultipleTimes() throw(
    IOException)
{
  shared_ptr<MemoryIndex> mindex = randomMemoryIndex();
  shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());
  mindex->addField(L"field", L"the quick brown fox", mockAnalyzer);
  mindex->addField(L"field", L"jumps over the", mockAnalyzer);
  shared_ptr<LeafReader> reader = std::static_pointer_cast<LeafReader>(
      mindex->createSearcher()->getIndexReader());
  TestUtil::checkReader(reader);
  TestUtil::assertEquals(7, reader->terms(L"field")->getSumTotalTermFreq());
  shared_ptr<PhraseQuery> query =
      make_shared<PhraseQuery>(L"field", L"fox", L"jumps");
  assertTrue(mindex->search(query) > 0.1);
  mindex->reset();
  mockAnalyzer->setPositionIncrementGap(1 + random()->nextInt(10));
  mindex->addField(L"field", L"the quick brown fox", mockAnalyzer);
  mindex->addField(L"field", L"jumps over the", mockAnalyzer);
  assertEquals(0, mindex->search(query), 0.00001f);
  query = make_shared<PhraseQuery>(10, L"field", L"fox", L"jumps");
  assertTrue(L"posGap" +
                 to_wstring(mockAnalyzer->getPositionIncrementGap(L"field")),
             mindex->search(query) > 0.0001);
  TestUtil::checkReader(mindex->createSearcher()->getIndexReader());
}

void TestMemoryIndexAgainstRAMDir::testNonExistentField() 
{
  shared_ptr<MemoryIndex> mindex = randomMemoryIndex();
  shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());
  mindex->addField(L"field", L"the quick brown fox", mockAnalyzer);
  shared_ptr<LeafReader> reader = std::static_pointer_cast<LeafReader>(
      mindex->createSearcher()->getIndexReader());
  TestUtil::checkReader(reader);
  assertNull(reader->getNumericDocValues(L"not-in-index"));
  assertNull(reader->getNormValues(L"not-in-index"));
  assertNull(reader->postings(make_shared<Term>(L"not-in-index", L"foo")));
  assertNull(reader->postings(make_shared<Term>(L"not-in-index", L"foo"),
                              PostingsEnum::ALL));
  assertNull(reader->terms(L"not-in-index"));
}

void TestMemoryIndexAgainstRAMDir::
    testDocValuesMemoryIndexVsNormalIndex() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  int64_t randomLong = random()->nextLong();
  doc->push_back(make_shared<NumericDocValuesField>(L"numeric", randomLong));
  int numValues = atLeast(5);
  for (int i = 0; i < numValues; i++) {
    randomLong = random()->nextLong();
    doc->push_back(make_shared<SortedNumericDocValuesField>(L"sorted_numeric",
                                                            randomLong));
    if (random()->nextBoolean()) {
      // randomly duplicate field/value
      doc->push_back(make_shared<SortedNumericDocValuesField>(L"sorted_numeric",
                                                              randomLong));
    }
  }
  shared_ptr<BytesRef> randomTerm = make_shared<BytesRef>(this->randomTerm());
  doc->push_back(make_shared<BinaryDocValuesField>(L"binary", randomTerm));
  if (random()->nextBoolean()) {
    doc->push_back(
        make_shared<StringField>(L"binary", randomTerm, Field::Store::NO));
  }
  randomTerm = make_shared<BytesRef>(this->randomTerm());
  doc->push_back(make_shared<SortedDocValuesField>(L"sorted", randomTerm));
  if (random()->nextBoolean()) {
    doc->push_back(
        make_shared<StringField>(L"sorted", randomTerm, Field::Store::NO));
  }
  numValues = atLeast(5);
  for (int i = 0; i < numValues; i++) {
    randomTerm = make_shared<BytesRef>(this->randomTerm());
    doc->push_back(
        make_shared<SortedSetDocValuesField>(L"sorted_set", randomTerm));
    if (random()->nextBoolean()) {
      // randomly duplicate field/value
      doc->push_back(
          make_shared<SortedSetDocValuesField>(L"sorted_set", randomTerm));
    }
    if (random()->nextBoolean()) {
      // randomily just add a normal string field
      doc->push_back(make_shared<StringField>(L"sorted_set", randomTerm,
                                              Field::Store::NO));
    }
  }

  shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<MemoryIndex> memoryIndex =
      MemoryIndex::fromDocument(doc, mockAnalyzer);
  shared_ptr<IndexReader> indexReader =
      memoryIndex->createSearcher()->getIndexReader();
  shared_ptr<LeafReader> leafReader = indexReader->leaves()[0]->reader();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(), mockAnalyzer));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<IndexReader> controlIndexReader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> controlLeafReader =
      controlIndexReader->leaves()[0]->reader();

  shared_ptr<NumericDocValues> numericDocValues =
      leafReader->getNumericDocValues(L"numeric");
  shared_ptr<NumericDocValues> controlNumericDocValues =
      controlLeafReader->getNumericDocValues(L"numeric");
  TestUtil::assertEquals(0, numericDocValues->nextDoc());
  TestUtil::assertEquals(0, controlNumericDocValues->nextDoc());
  TestUtil::assertEquals(controlNumericDocValues->longValue(),
                         numericDocValues->longValue());

  shared_ptr<SortedNumericDocValues> sortedNumericDocValues =
      leafReader->getSortedNumericDocValues(L"sorted_numeric");
  TestUtil::assertEquals(0, sortedNumericDocValues->nextDoc());
  shared_ptr<SortedNumericDocValues> controlSortedNumericDocValues =
      controlLeafReader->getSortedNumericDocValues(L"sorted_numeric");
  TestUtil::assertEquals(0, controlSortedNumericDocValues->nextDoc());
  TestUtil::assertEquals(controlSortedNumericDocValues->docValueCount(),
                         sortedNumericDocValues->docValueCount());
  for (int i = 0; i < controlSortedNumericDocValues->docValueCount(); i++) {
    TestUtil::assertEquals(controlSortedNumericDocValues->nextValue(),
                           sortedNumericDocValues->nextValue());
  }

  shared_ptr<BinaryDocValues> binaryDocValues =
      leafReader->getBinaryDocValues(L"binary");
  shared_ptr<BinaryDocValues> controlBinaryDocValues =
      controlLeafReader->getBinaryDocValues(L"binary");
  TestUtil::assertEquals(0, binaryDocValues->nextDoc());
  TestUtil::assertEquals(0, controlBinaryDocValues->nextDoc());
  TestUtil::assertEquals(controlBinaryDocValues->binaryValue(),
                         binaryDocValues->binaryValue());

  shared_ptr<SortedDocValues> sortedDocValues =
      leafReader->getSortedDocValues(L"sorted");
  shared_ptr<SortedDocValues> controlSortedDocValues =
      controlLeafReader->getSortedDocValues(L"sorted");
  TestUtil::assertEquals(controlSortedDocValues->getValueCount(),
                         sortedDocValues->getValueCount());
  TestUtil::assertEquals(0, sortedDocValues->nextDoc());
  TestUtil::assertEquals(0, controlSortedDocValues->nextDoc());
  TestUtil::assertEquals(controlSortedDocValues->binaryValue(),
                         sortedDocValues->binaryValue());
  TestUtil::assertEquals(controlSortedDocValues->ordValue(),
                         sortedDocValues->ordValue());
  TestUtil::assertEquals(controlSortedDocValues->lookupOrd(0),
                         sortedDocValues->lookupOrd(0));

  shared_ptr<SortedSetDocValues> sortedSetDocValues =
      leafReader->getSortedSetDocValues(L"sorted_set");
  TestUtil::assertEquals(0, sortedSetDocValues->nextDoc());
  shared_ptr<SortedSetDocValues> controlSortedSetDocValues =
      controlLeafReader->getSortedSetDocValues(L"sorted_set");
  TestUtil::assertEquals(0, controlSortedSetDocValues->nextDoc());
  TestUtil::assertEquals(controlSortedSetDocValues->getValueCount(),
                         sortedSetDocValues->getValueCount());
  for (int64_t controlOrd = controlSortedSetDocValues->nextOrd();
       controlOrd != SortedSetDocValues::NO_MORE_ORDS;
       controlOrd = controlSortedSetDocValues->nextOrd()) {
    TestUtil::assertEquals(controlOrd, sortedSetDocValues->nextOrd());
    TestUtil::assertEquals(controlSortedSetDocValues->lookupOrd(controlOrd),
                           sortedSetDocValues->lookupOrd(controlOrd));
  }
  TestUtil::assertEquals(SortedSetDocValues::NO_MORE_ORDS,
                         sortedSetDocValues->nextOrd());

  delete indexReader;
  delete controlIndexReader;
  delete dir;
}

void TestMemoryIndexAgainstRAMDir::testNormsWithDocValues() 
{
  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>(true, true);
  shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());

  mi->addField(make_shared<BinaryDocValuesField>(
                   L"text", make_shared<BytesRef>(L"quick brown fox")),
               mockAnalyzer);
  mi->addField(
      make_shared<TextField>(L"text", L"quick brown fox", Field::Store::NO),
      mockAnalyzer);
  shared_ptr<LeafReader> leafReader =
      mi->createSearcher()->getIndexReader()->leaves()[0]->reader();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"text", make_shared<BytesRef>(L"quick brown fox")));
  shared_ptr<Field> field =
      make_shared<TextField>(L"text", L"quick brown fox", Field::Store::NO);
  doc->push_back(field);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(), mockAnalyzer));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<IndexReader> controlIndexReader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> controlLeafReader =
      controlIndexReader->leaves()[0]->reader();

  shared_ptr<NumericDocValues> norms =
      controlLeafReader->getNormValues(L"text");
  TestUtil::assertEquals(0, norms->nextDoc());
  shared_ptr<NumericDocValues> norms2 = leafReader->getNormValues(L"text");
  TestUtil::assertEquals(0, norms2->nextDoc());
  TestUtil::assertEquals(norms->longValue(), norms2->longValue());

  delete controlIndexReader;
  delete dir;
}

void TestMemoryIndexAgainstRAMDir::
    testPointValuesMemoryIndexVsNormalIndex() 
{
  int size = atLeast(12);

  deque<int> randomValues = deque<int>();

  shared_ptr<Document> doc = make_shared<Document>();
  for (shared_ptr<> : : optional<int> randomInteger :
       random()->ints(size).toArray()) {
    doc->push_back(make_shared<IntPoint>(L"int", randomInteger));
    randomValues.push_back(randomInteger);
    doc->push_back(make_shared<LongPoint>(L"long", randomInteger));
    doc->push_back(make_shared<FloatPoint>(L"float", randomInteger));
    doc->push_back(make_shared<DoublePoint>(L"double", randomInteger));
  }

  shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<MemoryIndex> memoryIndex =
      MemoryIndex::fromDocument(doc, mockAnalyzer);
  shared_ptr<IndexSearcher> memoryIndexSearcher = memoryIndex->createSearcher();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(), mockAnalyzer));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<IndexReader> controlIndexReader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> controlIndexSearcher =
      make_shared<IndexSearcher>(controlIndexReader);

  function<int()> valueSupplier = [&]() {
    randomValues[random()->nextInt(randomValues.size())];
  };
  std::deque<std::shared_ptr<Query>> queries = {
      IntPoint::newExactQuery(L"int", valueSupplier()),
      LongPoint::newExactQuery(L"long", valueSupplier()),
      FloatPoint::newExactQuery(L"float", valueSupplier()),
      DoublePoint::newExactQuery(L"double", valueSupplier()),
      IntPoint::newSetQuery(L"int", valueSupplier(), valueSupplier()),
      LongPoint::newSetQuery(L"long", valueSupplier(), valueSupplier()),
      FloatPoint::newSetQuery(L"float", valueSupplier(), valueSupplier()),
      DoublePoint::newSetQuery(L"double", valueSupplier(), valueSupplier()),
      IntPoint::newRangeQuery(L"int", valueSupplier(), valueSupplier()),
      LongPoint::newRangeQuery(L"long", valueSupplier(), valueSupplier()),
      FloatPoint::newRangeQuery(L"float", valueSupplier(), valueSupplier()),
      DoublePoint::newRangeQuery(L"double", valueSupplier(), valueSupplier())};
  for (auto query : queries) {
    TestUtil::assertEquals(controlIndexSearcher->count(query),
                           controlIndexSearcher->count(query));
  }

  delete memoryIndexSearcher->getIndexReader();
  delete controlIndexReader;
  delete dir;
}

void TestMemoryIndexAgainstRAMDir::testDuellMemIndex() 
{
  shared_ptr<LineFileDocs> lineFileDocs = make_shared<LineFileDocs>(random());
  int numDocs = atLeast(10);
  shared_ptr<MemoryIndex> memory = randomMemoryIndex();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());
    mockAnalyzer->setMaxTokenLength(
        TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(random(), mockAnalyzer));
    shared_ptr<Document> nextDoc = lineFileDocs->nextDoc();
    shared_ptr<Document> doc = make_shared<Document>();
    for (auto field : nextDoc->getFields()) {
      if (field->fieldType()->indexOptions() != IndexOptions::NONE) {
        doc->push_back(field);
        if (random()->nextInt(3) == 0) {
          doc->push_back(field); // randomly add the same field twice
        }
      }
    }

    writer->addDocument(doc);
    delete writer;
    for (auto field : doc) {
      memory->addField(field->name(),
                       (std::static_pointer_cast<Field>(field))->stringValue(),
                       mockAnalyzer);
    }
    shared_ptr<DirectoryReader> competitor = DirectoryReader::open(dir);
    shared_ptr<LeafReader> memIndexReader =
        std::static_pointer_cast<LeafReader>(
            memory->createSearcher().getIndexReader());
    TestUtil::checkReader(memIndexReader);
    duellReaders(competitor, memIndexReader);
    IOUtils::close({competitor, memIndexReader});
    memory->reset();
    delete dir;
  }
  delete lineFileDocs;
}

void TestMemoryIndexAgainstRAMDir::testEmptyString() 
{
  shared_ptr<MemoryIndex> memory = make_shared<MemoryIndex>();
  memory->addField(
      L"foo", make_shared<CannedTokenStream>(make_shared<Token>(L"", 0, 5)));
  shared_ptr<IndexSearcher> searcher = memory->createSearcher();
  shared_ptr<TopDocs> docs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"")), 10);
  TestUtil::assertEquals(1, docs->totalHits);
  TestUtil::checkReader(searcher->getIndexReader());
}

void TestMemoryIndexAgainstRAMDir::
    testDuelMemoryIndexCoreDirectoryWithArrayField() 
{

  const wstring field_name = L"text";
  shared_ptr<MockAnalyzer> mockAnalyzer = make_shared<MockAnalyzer>(random());
  if (random()->nextBoolean()) {
    mockAnalyzer->setOffsetGap(random()->nextInt(100));
  }
  // index into a random directory
  shared_ptr<FieldType> type = make_shared<FieldType>(TextField::TYPE_STORED);
  type->setStoreTermVectorOffsets(true);
  type->setStoreTermVectorPayloads(false);
  type->setStoreTermVectorPositions(true);
  type->setStoreTermVectors(true);
  type->freeze();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(field_name, L"la la", type));
  doc->push_back(make_shared<Field>(field_name, L"foo bar foo bar foo", type));

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(), mockAnalyzer));
  writer->updateDocument(make_shared<Term>(L"id", L"1"), doc);
  writer->commit();
  delete writer;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);

  // Index document in Memory index
  shared_ptr<MemoryIndex> memIndex = make_shared<MemoryIndex>(true);
  memIndex->addField(field_name, L"la la", mockAnalyzer);
  memIndex->addField(field_name, L"foo bar foo bar foo", mockAnalyzer);

  // compare term vectors
  shared_ptr<Terms> ramTv = reader->getTermVector(0, field_name);
  shared_ptr<IndexReader> memIndexReader =
      memIndex->createSearcher()->getIndexReader();
  TestUtil::checkReader(memIndexReader);
  shared_ptr<Terms> memTv = memIndexReader->getTermVector(0, field_name);

  compareTermVectors(ramTv, memTv, field_name);
  delete memIndexReader;
  reader->close();
  delete dir;
}

void TestMemoryIndexAgainstRAMDir::compareTermVectors(
    shared_ptr<Terms> terms, shared_ptr<Terms> memTerms,
    const wstring &field_name) 
{

  shared_ptr<TermsEnum> termEnum = terms->begin();
  shared_ptr<TermsEnum> memTermEnum = memTerms->begin();

  while (termEnum->next() != nullptr) {
    assertNotNull(memTermEnum->next());
    assertThat(termEnum->totalTermFreq(),
               equalTo(memTermEnum->totalTermFreq()));

    shared_ptr<PostingsEnum> docsPosEnum =
        termEnum->postings(nullptr, PostingsEnum::POSITIONS);
    shared_ptr<PostingsEnum> memDocsPosEnum =
        memTermEnum->postings(nullptr, PostingsEnum::POSITIONS);
    wstring currentTerm = termEnum->term()->utf8ToString();

    assertThat(L"Token mismatch for field: " + field_name, currentTerm,
               equalTo(memTermEnum->term()->utf8ToString()));

    docsPosEnum->nextDoc();
    memDocsPosEnum->nextDoc();

    int freq = docsPosEnum->freq();
    assertThat(freq, equalTo(memDocsPosEnum->freq()));
    for (int i = 0; i < freq; i++) {
      wstring failDesc =
          L" (field:" + field_name + L" term:" + currentTerm + L")";
      int memPos = memDocsPosEnum->nextPosition();
      int pos = docsPosEnum->nextPosition();
      assertThat(L"Position test failed" + failDesc, memPos, equalTo(pos));
      assertThat(L"Start offset test failed" + failDesc,
                 memDocsPosEnum->startOffset(),
                 equalTo(docsPosEnum->startOffset()));
      assertThat(L"End offset test failed" + failDesc,
                 memDocsPosEnum->endOffset(),
                 equalTo(docsPosEnum->endOffset()));
      assertThat(L"Missing payload test failed" + failDesc,
                 docsPosEnum->getPayload(), equalTo(docsPosEnum->getPayload()));
    }
  }
  assertNull(L"Still some tokens not processed", memTermEnum->next());
}
} // namespace org::apache::lucene::index::memory