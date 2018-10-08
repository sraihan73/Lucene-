using namespace std;

#include "BasePostingsFormatTestCase.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
//    import static org.apache.lucene.index.PostingsEnum.ALL;
//    import static org.apache.lucene.index.PostingsEnum.FREQS;
//    import static org.apache.lucene.index.PostingsEnum.NONE;
//    import static org.apache.lucene.index.PostingsEnum.OFFSETS;
//    import static org.apache.lucene.index.PostingsEnum.PAYLOADS;
//    import static org.apache.lucene.index.PostingsEnum.POSITIONS;
shared_ptr<RandomPostingsTester> BasePostingsFormatTestCase::postingsTester;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void createPostings() throws
// java.io.IOException
void BasePostingsFormatTestCase::createPostings() 
{
  postingsTester = make_shared<RandomPostingsTester>(random());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void BasePostingsFormatTestCase::afterClass() 
{
  postingsTester.reset();
}

void BasePostingsFormatTestCase::testDocsOnly() 
{
  postingsTester->testFull(getCodec(),
                           createTempDir(L"testPostingsFormat.testExact"),
                           IndexOptions::DOCS, false);
}

void BasePostingsFormatTestCase::testDocsAndFreqs() 
{
  postingsTester->testFull(getCodec(),
                           createTempDir(L"testPostingsFormat.testExact"),
                           IndexOptions::DOCS_AND_FREQS, false);
}

void BasePostingsFormatTestCase::testDocsAndFreqsAndPositions() throw(
    runtime_error)
{
  postingsTester->testFull(getCodec(),
                           createTempDir(L"testPostingsFormat.testExact"),
                           IndexOptions::DOCS_AND_FREQS_AND_POSITIONS, false);
}

void BasePostingsFormatTestCase::
    testDocsAndFreqsAndPositionsAndPayloads() 
{
  postingsTester->testFull(getCodec(),
                           createTempDir(L"testPostingsFormat.testExact"),
                           IndexOptions::DOCS_AND_FREQS_AND_POSITIONS, true);
}

void BasePostingsFormatTestCase::testDocsAndFreqsAndPositionsAndOffsets() throw(
    runtime_error)
{
  postingsTester->testFull(
      getCodec(), createTempDir(L"testPostingsFormat.testExact"),
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS, false);
}

void BasePostingsFormatTestCase::
    testDocsAndFreqsAndPositionsAndOffsetsAndPayloads() 
{
  postingsTester->testFull(
      getCodec(), createTempDir(L"testPostingsFormat.testExact"),
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS, true);
}

void BasePostingsFormatTestCase::testRandom() 
{

  int iters = 5;

  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<Path> path = createTempDir(L"testPostingsFormat");
    shared_ptr<Directory> dir = newFSDirectory(path);

    bool indexPayloads = random()->nextBoolean();
    // TODO test thread safety of buildIndex too
    shared_ptr<FieldsProducer> fieldsProducer = postingsTester->buildIndex(
        getCodec(), dir, IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS,
        indexPayloads, false);

    postingsTester->testFields(fieldsProducer);

    // NOTE: you can also test "weaker" index options than
    // you indexed with:
    postingsTester->testTerms(
        fieldsProducer, EnumSet::allOf(RandomPostingsTester::Option::typeid),
        IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS,
        IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS, false);

    fieldsProducer->close();
    fieldsProducer.reset();

    delete dir;
  }
}

bool BasePostingsFormatTestCase::isPostingsEnumReuseImplemented()
{
  return true;
}

void BasePostingsFormatTestCase::testPostingsEnumReuse() 
{

  shared_ptr<Path> path = createTempDir(L"testPostingsEnumReuse");
  shared_ptr<Directory> dir = newFSDirectory(path);

  shared_ptr<FieldsProducer> fieldsProducer = postingsTester->buildIndex(
      getCodec(), dir, IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS,
      random()->nextBoolean(), true);
  Collections::shuffle(postingsTester->allTerms, random());
  shared_ptr<RandomPostingsTester::FieldAndTerm> fieldAndTerm =
      postingsTester->allTerms[0];

  shared_ptr<Terms> terms = fieldsProducer->terms(fieldAndTerm->field);
  shared_ptr<TermsEnum> te = terms->begin();

  te->seekExact(fieldAndTerm->term);
  checkReuse(te, PostingsEnum::FREQS, PostingsEnum::ALL, false);
  if (isPostingsEnumReuseImplemented()) {
    checkReuse(te, PostingsEnum::ALL, PostingsEnum::ALL, true);
  }

  fieldsProducer->close();
  delete dir;
}

void BasePostingsFormatTestCase::checkReuse(shared_ptr<TermsEnum> termsEnum,
                                            int firstFlags, int secondFlags,
                                            bool shouldReuse) 
{
  shared_ptr<PostingsEnum> postings1 = termsEnum->postings(nullptr, firstFlags);
  shared_ptr<PostingsEnum> postings2 =
      termsEnum->postings(postings1, secondFlags);
  if (shouldReuse) {
    assertSame(L"Expected PostingsEnum " + postings1->getClassName() +
                   L" to be reused",
               postings1, postings2);
  } else {
    assertNotSame(L"Expected PostingsEnum " + postings1->getClassName() +
                      L" to not be reused",
                  postings1, postings2);
  }
}

void BasePostingsFormatTestCase::testJustEmptyField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"", L"something", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  TestUtil::assertEquals(1, ar->getFieldInfos()->size());
  shared_ptr<Terms> terms = ar->terms(L"");
  assertNotNull(terms);
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum->next());
  TestUtil::assertEquals(termsEnum->term(),
                         make_shared<BytesRef>(L"something"));
  assertNull(termsEnum->next());
  ir->close();
  delete iw;
  delete dir;
}

void BasePostingsFormatTestCase::testEmptyFieldAndEmptyTerm() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"", L"", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  TestUtil::assertEquals(1, ar->getFieldInfos()->size());
  shared_ptr<Terms> terms = ar->terms(L"");
  assertNotNull(terms);
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum->next());
  TestUtil::assertEquals(termsEnum->term(), make_shared<BytesRef>(L""));
  assertNull(termsEnum->next());
  ir->close();
  delete iw;
  delete dir;
}

void BasePostingsFormatTestCase::testDidntWantFreqsButAskedAnyway() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"value", Field::Store::NO));
  iw->addDocument(doc);
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  shared_ptr<TermsEnum> termsEnum = ar->terms(L"field")->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"value")));
  shared_ptr<PostingsEnum> docsEnum =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(0, docsEnum->nextDoc());
  TestUtil::assertEquals(1, docsEnum->freq());
  TestUtil::assertEquals(1, docsEnum->nextDoc());
  TestUtil::assertEquals(1, docsEnum->freq());
  ir->close();
  delete iw;
  delete dir;
}

void BasePostingsFormatTestCase::testAskForPositionsWhenNotThere() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"value", Field::Store::NO));
  iw->addDocument(doc);
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  shared_ptr<TermsEnum> termsEnum = ar->terms(L"field")->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"value")));
  shared_ptr<PostingsEnum> docsEnum =
      termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
  TestUtil::assertEquals(0, docsEnum->nextDoc());
  TestUtil::assertEquals(1, docsEnum->freq());
  TestUtil::assertEquals(1, docsEnum->nextDoc());
  TestUtil::assertEquals(1, docsEnum->freq());
  ir->close();
  delete iw;
  delete dir;
}

void BasePostingsFormatTestCase::testGhosts() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  iwc->setCodec(getCodec());
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);
  doc->push_back(newStringField(L"ghostField", L"something", Field::Store::NO));
  iw->addDocument(doc);
  iw->forceMerge(1);
  iw->deleteDocuments(make_shared<Term>(
      L"ghostField", L"something")); // delete the only term for the field
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  // Ghost busting terms dict impls will have
  // fields.size() == 0; all others must be == 1:
  assertTrue(ar->getFieldInfos()->size() <= 1);
  shared_ptr<Terms> terms = ar->terms(L"ghostField");
  if (terms != nullptr) {
    shared_ptr<TermsEnum> termsEnum = terms->begin();
    shared_ptr<BytesRef> term = termsEnum->next();
    if (term != nullptr) {
      shared_ptr<PostingsEnum> postingsEnum = termsEnum->postings(nullptr);
      assertTrue(postingsEnum->nextDoc() == PostingsEnum::NO_MORE_DOCS);
    }
  }
  ir->close();
  delete iw;
  delete dir;
}

void BasePostingsFormatTestCase::testLevel2Ghosts() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  iwc->setCodec(getCodec());
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"suggest_field", L"apples", Field::Store::NO));
  iw->addDocument(document);
  // need another document so whole segment isn't deleted
  iw->addDocument(make_shared<Document>());
  iw->commit();

  document = make_shared<Document>();
  document->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"suggest_field2", L"apples", Field::Store::NO));
  iw->addDocument(document);
  iw->commit();

  iw->deleteDocuments({make_shared<Term>(L"id", L"0")});
  // first force merge creates a level 1 ghost field
  iw->forceMerge(1);

  // second force merge creates a level 2 ghost field, causing MultiFields to
  // include "suggest_field" in its iteration, yet a null Terms is returned (no
  // documents have this field anymore)
  iw->addDocument(make_shared<Document>());
  iw->forceMerge(1);

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);
  shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(reader);

  TestUtil::assertEquals(1, indexSearcher->count(make_shared<TermQuery>(
                                make_shared<Term>(L"id", L"1"))));

  reader->close();
  delete iw;
  delete dir;
}

void BasePostingsFormatTestCase::testInvertedWrite() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);

  // Must be concurrent because thread(s) can be merging
  // while up to one thread flushes, and each of those
  // threads iterates over the map_obj while the flushing
  // thread might be adding to it:
  const unordered_map<wstring, std::shared_ptr<TermFreqs>> termFreqs =
      make_shared<ConcurrentHashMap<wstring, std::shared_ptr<TermFreqs>>>();

  shared_ptr<AtomicLong> *const sumDocFreq = make_shared<AtomicLong>();
  shared_ptr<AtomicLong> *const sumTotalTermFreq = make_shared<AtomicLong>();

  // TODO: would be better to use / delegate to the current
  // Codec returned by getCodec()

  iwc->setCodec(make_shared<FilterCodecAnonymousInnerClass>(
      shared_from_this(), getCodec()->getName(), getCodec(), termFreqs,
      sumDocFreq, sumTotalTermFreq));

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  int bytesToIndex = atLeast(100) * 1024;
  int bytesIndexed = 0;
  while (bytesIndexed < bytesToIndex) {
    shared_ptr<Document> doc = docs->nextDoc();
    shared_ptr<Document> justBodyDoc = make_shared<Document>();
    justBodyDoc->push_back(doc->getField(L"body"));
    w->addDocument(justBodyDoc);
    bytesIndexed += RamUsageTester::sizeOf(justBodyDoc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"body");
  TestUtil::assertEquals(sumDocFreq->get(), terms->getSumDocFreq());
  TestUtil::assertEquals(sumTotalTermFreq->get(), terms->getSumTotalTermFreq());

  shared_ptr<TermsEnum> termsEnum = terms->begin();
  int64_t termCount = 0;
  bool supportsOrds = true;
  while (termsEnum->next() != nullptr) {
    shared_ptr<BytesRef> term = termsEnum->term();
    TestUtil::assertEquals(termFreqs[term->utf8ToString()]->docFreq,
                           termsEnum->docFreq());
    TestUtil::assertEquals(termFreqs[term->utf8ToString()]->totalTermFreq,
                           termsEnum->totalTermFreq());
    if (supportsOrds) {
      int64_t ord;
      try {
        ord = termsEnum->ord();
      } catch (const UnsupportedOperationException &uoe) {
        supportsOrds = false;
        ord = -1;
      }
      if (ord != -1) {
        TestUtil::assertEquals(termCount, ord);
      }
    }
    termCount++;
  }
  TestUtil::assertEquals(termFreqs.size(), termCount);

  delete r;
  delete dir;
}

BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    FilterCodecAnonymousInnerClass(
        shared_ptr<BasePostingsFormatTestCase> outerInstance,
        const wstring &getName,
        shared_ptr<org::apache::lucene::codecs::Codec> getCodec,
        unordered_map<wstring, std::shared_ptr<TermFreqs>> &termFreqs,
        shared_ptr<AtomicLong> sumDocFreq,
        shared_ptr<AtomicLong> sumTotalTermFreq)
    : org::apache::lucene::codecs::FilterCodec(getName, getCodec)
{
  this->outerInstance = outerInstance;
  this->termFreqs = termFreqs;
  this->sumDocFreq = sumDocFreq;
  this->sumTotalTermFreq = sumTotalTermFreq;
}

shared_ptr<PostingsFormat>
BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::postingsFormat()
{

  shared_ptr<PostingsFormat> *const defaultPostingsFormat =
      delegate_::postingsFormat();

  shared_ptr<Thread> *const mainThread = Thread::currentThread();

  // A PF that counts up some stats and then in
  // the end we verify the stats match what the
  // final IndexReader says, just to exercise the
  // new freedom of iterating the postings more
  // than once at flush/merge:

  return make_shared<PostingsFormatAnonymousInnerClass>(
      shared_from_this(), defaultPostingsFormat->getName(),
      defaultPostingsFormat, mainThread);
}

BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    PostingsFormatAnonymousInnerClass::PostingsFormatAnonymousInnerClass(
        shared_ptr<FilterCodecAnonymousInnerClass> outerInstance,
        const wstring &getName,
        shared_ptr<PostingsFormat> defaultPostingsFormat,
        shared_ptr<Thread> mainThread)
    : org::apache::lucene::codecs::PostingsFormat(getName)
{
  this->outerInstance = outerInstance;
  this->defaultPostingsFormat = defaultPostingsFormat;
  this->mainThread = mainThread;
}

shared_ptr<FieldsConsumer>
BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    PostingsFormatAnonymousInnerClass::fieldsConsumer(
        shared_ptr<SegmentWriteState> state) 
{

  shared_ptr<FieldsConsumer> *const fieldsConsumer =
      defaultPostingsFormat->fieldsConsumer(state);

  return make_shared<FieldsConsumerAnonymousInnerClass>(shared_from_this(),
                                                        state, fieldsConsumer);
}

BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    PostingsFormatAnonymousInnerClass::FieldsConsumerAnonymousInnerClass::
        FieldsConsumerAnonymousInnerClass(
            shared_ptr<PostingsFormatAnonymousInnerClass> outerInstance,
            shared_ptr<org::apache::lucene::index::SegmentWriteState> state,
            shared_ptr<FieldsConsumer> fieldsConsumer)
{
  this->outerInstance = outerInstance;
  this->state = state;
  this->fieldsConsumer = fieldsConsumer;
}

void BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    PostingsFormatAnonymousInnerClass::FieldsConsumerAnonymousInnerClass::write(
        shared_ptr<Fields> fields) 
{
  fieldsConsumer->write(fields);

  bool isMerge = state->context->context == IOContext::Context::MERGE;

  // We only use one thread for flushing
  // in this test:
  assert(isMerge || Thread::currentThread() == outerInstance->mainThread);

  // We iterate the provided TermsEnum
  // twice, so we excercise this new freedom
  // with the inverted API; if
  // addOnSecondPass is true, we add up
  // term stats on the 2nd iteration:
  bool addOnSecondPass = random().nextBoolean();

  // System.out.println("write isMerge=" + isMerge + " 2ndPass=" +
  // addOnSecondPass);

  // Gather our own stats:
  shared_ptr<Terms> terms = fields->terms(L"body");
  assert(terms != nullptr);

  shared_ptr<TermsEnum> termsEnum = terms->begin();
  shared_ptr<PostingsEnum> docs = nullptr;
  while (termsEnum->next() != nullptr) {
    shared_ptr<BytesRef> term = termsEnum->term();
    // TODO: also sometimes ask for payloads/offsets?
    bool noPositions = random().nextBoolean();
    if (noPositions) {
      docs = termsEnum->postings(docs, PostingsEnum::FREQS);
    } else {
      docs = termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
    }
    int docFreq = 0;
    int64_t totalTermFreq = 0;
    while (docs->nextDoc() != PostingsEnum::NO_MORE_DOCS) {
      docFreq++;
      totalTermFreq += docs->freq();
      int limit = TestUtil::nextInt(random(), 1, docs->freq());
      if (!noPositions) {
        for (int i = 0; i < limit; i++) {
          docs->nextPosition();
        }
      }
    }

    wstring termString = term->utf8ToString();

    // During merge we should only see terms
    // we had already seen during a
    // previous flush:
    assertTrue(isMerge == false ||
               outerInstance->outerInstance.termFreqs.find(termString) !=
                   outerInstance->outerInstance.termFreqs.end());

    if (isMerge == false) {
      if (addOnSecondPass == false) {
        shared_ptr<TermFreqs> tf =
            outerInstance->outerInstance.termFreqs[termString];
        if (tf == nullptr) {
          tf = make_shared<TermFreqs>();
          outerInstance->outerInstance.termFreqs.emplace(termString, tf);
        }
        tf->docFreq += docFreq;
        tf->totalTermFreq += totalTermFreq;
        outerInstance->outerInstance.sumDocFreq.addAndGet(docFreq);
        outerInstance->outerInstance.sumTotalTermFreq.addAndGet(totalTermFreq);
      } else if (outerInstance->outerInstance.termFreqs.find(termString) !=
                 outerInstance->outerInstance.termFreqs.end() == false) {
        // Add placeholder (2nd pass will
        // set its counts):
        outerInstance->outerInstance.termFreqs.emplace(
            termString, make_shared<TermFreqs>());
      }
    }
  }

  // Also test seeking the TermsEnum:
  for (wstring term : outerInstance->outerInstance.termFreqs.keySet()) {
    if (termsEnum->seekExact(make_shared<BytesRef>(term))) {
      // TODO: also sometimes ask for payloads/offsets?
      bool noPositions = random().nextBoolean();
      if (noPositions) {
        docs = termsEnum->postings(docs, PostingsEnum::FREQS);
      } else {
        docs = termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
      }

      int docFreq = 0;
      int64_t totalTermFreq = 0;
      while (docs->nextDoc() != PostingsEnum::NO_MORE_DOCS) {
        docFreq++;
        totalTermFreq += docs->freq();
        int limit = TestUtil::nextInt(random(), 1, docs->freq());
        if (!noPositions) {
          for (int i = 0; i < limit; i++) {
            docs->nextPosition();
          }
        }
      }

      if (isMerge == false && addOnSecondPass) {
        shared_ptr<TermFreqs> tf = outerInstance->outerInstance.termFreqs[term];
        assert(tf != nullptr);
        tf->docFreq += docFreq;
        tf->totalTermFreq += totalTermFreq;
        outerInstance->outerInstance.sumDocFreq.addAndGet(docFreq);
        outerInstance->outerInstance.sumTotalTermFreq.addAndGet(totalTermFreq);
      }

      // System.out.println("  term=" + term + " docFreq=" + docFreq + " ttDF="
      // + termToDocFreq.get(term));
      assertTrue(docFreq <=
                 outerInstance->outerInstance.termFreqs[term].docFreq);
      assertTrue(totalTermFreq <=
                 outerInstance->outerInstance.termFreqs[term].totalTermFreq);
    }
  }

  // Also test seekCeil
  for (int iter = 0; iter < 10; iter++) {
    shared_ptr<BytesRef> term =
        make_shared<BytesRef>(TestUtil::randomRealisticUnicodeString(random()));
    SeekStatus status = termsEnum->seekCeil(term);
    if (status == SeekStatus::NOT_FOUND) {
      assertTrue(term->compareTo(termsEnum->term()) < 0);
    }
  }
}

BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    PostingsFormatAnonymousInnerClass::FieldsConsumerAnonymousInnerClass::
        ~FieldsConsumerAnonymousInnerClass()
{
  delete fieldsConsumer;
}

shared_ptr<FieldsProducer>
BasePostingsFormatTestCase::FilterCodecAnonymousInnerClass::
    PostingsFormatAnonymousInnerClass::fieldsProducer(
        shared_ptr<SegmentReadState> state) 
{
  return defaultPostingsFormat->fieldsProducer(state);
}

void BasePostingsFormatTestCase::assertReused(const wstring &field,
                                              shared_ptr<PostingsEnum> p1,
                                              shared_ptr<PostingsEnum> p2)
{
  // if its not DirectPF, we should always reuse. This one has trouble.
  if (L"Direct" != TestUtil::getPostingsFormat(field)) {
    assertSame(p1, p2);
  }
}

void BasePostingsFormatTestCase::testPostingsEnumDocsOnly() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"));
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(1, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<TermsEnum> termsEnum =
      getOnlyLeafReader(reader)->terms(L"foo")->begin();
  termsEnum->seekExact(make_shared<BytesRef>(L"bar"));
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  assertReused(L"foo", postings, postings2);
  // and it had better work
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(1, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // asking for any flags: ok
  for (auto flag :
       std::deque<int>{NONE, FREQS, POSITIONS, PAYLOADS, OFFSETS, ALL}) {
    postings = termsEnum->postings(nullptr, flag);
    TestUtil::assertEquals(-1, postings->docID());
    TestUtil::assertEquals(0, postings->nextDoc());
    TestUtil::assertEquals(1, postings->freq());
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());
    // reuse that too
    postings2 = termsEnum->postings(postings, flag);
    assertNotNull(postings2);
    assertReused(L"foo", postings, postings2);
    // and it had better work
    TestUtil::assertEquals(-1, postings2->docID());
    TestUtil::assertEquals(0, postings2->nextDoc());
    TestUtil::assertEquals(1, postings2->freq());
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                           postings2->nextDoc());
  }

  delete iw;
  reader->close();
  delete dir;
}

void BasePostingsFormatTestCase::testPostingsEnumFreqs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  doc->push_back(make_shared<Field>(L"foo", L"bar bar", ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"));
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(2, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<TermsEnum> termsEnum =
      getOnlyLeafReader(reader)->terms(L"foo")->begin();
  termsEnum->seekExact(make_shared<BytesRef>(L"bar"));
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  assertReused(L"foo", postings, postings2);
  // and it had better work
  TestUtil::assertEquals(-1, postings2->docID());
  TestUtil::assertEquals(0, postings2->nextDoc());
  TestUtil::assertEquals(2, postings2->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(-1, docsOnly->docID());
  TestUtil::assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  assertReused(L"foo", docsOnly, docsOnly2);
  // and it had better work
  TestUtil::assertEquals(-1, docsOnly2->docID());
  TestUtil::assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for any flags: ok
  for (auto flag :
       std::deque<int>{NONE, FREQS, POSITIONS, PAYLOADS, OFFSETS, ALL}) {
    postings = termsEnum->postings(nullptr, flag);
    TestUtil::assertEquals(-1, postings->docID());
    TestUtil::assertEquals(0, postings->nextDoc());
    if (flag != NONE) {
      TestUtil::assertEquals(2, postings->freq());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());
    // reuse that too
    postings2 = termsEnum->postings(postings, flag);
    assertNotNull(postings2);
    assertReused(L"foo", postings, postings2);
    // and it had better work
    TestUtil::assertEquals(-1, postings2->docID());
    TestUtil::assertEquals(0, postings2->nextDoc());
    if (flag != NONE) {
      TestUtil::assertEquals(2, postings2->freq());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                           postings2->nextDoc());
  }

  delete iw;
  reader->close();
  delete dir;
}

BasePostingsFormatTestCase::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<BasePostingsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BasePostingsFormatTestCase::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BasePostingsFormatTestCase::testPostingsEnumPositions() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"foo", L"bar bar", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"));
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(2, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<TermsEnum> termsEnum =
      getOnlyLeafReader(reader)->terms(L"foo")->begin();
  termsEnum->seekExact(make_shared<BytesRef>(L"bar"));
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  assertReused(L"foo", postings, postings2);
  // and it had better work
  TestUtil::assertEquals(-1, postings2->docID());
  TestUtil::assertEquals(0, postings2->nextDoc());
  TestUtil::assertEquals(2, postings2->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(-1, docsOnly->docID());
  TestUtil::assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  assertReused(L"foo", docsOnly, docsOnly2);
  // and it had better work
  TestUtil::assertEquals(-1, docsOnly2->docID());
  TestUtil::assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"),
                                          PostingsEnum::POSITIONS);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  // payloads, offsets, etc don't cause an error if they aren't there
  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  // but make sure they work
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

BasePostingsFormatTestCase::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<BasePostingsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BasePostingsFormatTestCase::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BasePostingsFormatTestCase::testPostingsEnumOffsets() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  doc->push_back(make_shared<Field>(L"foo", L"bar bar", ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"));
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(2, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<TermsEnum> termsEnum =
      getOnlyLeafReader(reader)->terms(L"foo")->begin();
  termsEnum->seekExact(make_shared<BytesRef>(L"bar"));
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  assertReused(L"foo", postings, postings2);
  // and it had better work
  TestUtil::assertEquals(-1, postings2->docID());
  TestUtil::assertEquals(0, postings2->nextDoc());
  TestUtil::assertEquals(2, postings2->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(-1, docsOnly->docID());
  TestUtil::assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  assertReused(L"foo", docsOnly, docsOnly2);
  // and it had better work
  TestUtil::assertEquals(-1, docsOnly2->docID());
  TestUtil::assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"),
                                          PostingsEnum::POSITIONS);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  // payloads don't cause an error if they aren't there
  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  // but make sure they work
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

BasePostingsFormatTestCase::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<BasePostingsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BasePostingsFormatTestCase::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BasePostingsFormatTestCase::testPostingsEnumPayloads() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Token> token1 = make_shared<Token>(L"bar", 0, 3);
  token1->setPayload(make_shared<BytesRef>(L"pay1"));
  shared_ptr<Token> token2 = make_shared<Token>(L"bar", 4, 7);
  token2->setPayload(make_shared<BytesRef>(L"pay2"));
  doc->push_back(make_shared<TextField>(
      L"foo", make_shared<CannedTokenStream>(token1, token2)));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"));
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(2, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<TermsEnum> termsEnum =
      getOnlyLeafReader(reader)->terms(L"foo")->begin();
  termsEnum->seekExact(make_shared<BytesRef>(L"bar"));
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  assertReused(L"foo", postings, postings2);
  // and it had better work
  TestUtil::assertEquals(-1, postings2->docID());
  TestUtil::assertEquals(0, postings2->nextDoc());
  TestUtil::assertEquals(2, postings2->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(-1, docsOnly->docID());
  TestUtil::assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  assertReused(L"foo", docsOnly, docsOnly2);
  // and it had better work
  TestUtil::assertEquals(-1, docsOnly2->docID());
  TestUtil::assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"),
                                          PostingsEnum::POSITIONS);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  // payloads
  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

void BasePostingsFormatTestCase::testPostingsEnumAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Token> token1 = make_shared<Token>(L"bar", 0, 3);
  token1->setPayload(make_shared<BytesRef>(L"pay1"));
  shared_ptr<Token> token2 = make_shared<Token>(L"bar", 4, 7);
  token2->setPayload(make_shared<BytesRef>(L"pay2"));
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  doc->push_back(make_shared<Field>(
      L"foo", make_shared<CannedTokenStream>(token1, token2), ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"));
  TestUtil::assertEquals(-1, postings->docID());
  TestUtil::assertEquals(0, postings->nextDoc());
  TestUtil::assertEquals(2, postings->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<TermsEnum> termsEnum =
      getOnlyLeafReader(reader)->terms(L"foo")->begin();
  termsEnum->seekExact(make_shared<BytesRef>(L"bar"));
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  assertReused(L"foo", postings, postings2);
  // and it had better work
  TestUtil::assertEquals(-1, postings2->docID());
  TestUtil::assertEquals(0, postings2->nextDoc());
  TestUtil::assertEquals(2, postings2->freq());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(-1, docsOnly->docID());
  TestUtil::assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  assertReused(L"foo", docsOnly, docsOnly2);
  // and it had better work
  TestUtil::assertEquals(-1, docsOnly2->docID());
  TestUtil::assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      getOnlyLeafReader(reader)->postings(make_shared<Term>(L"foo", L"bar"),
                                          PostingsEnum::POSITIONS);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  // payloads
  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = getOnlyLeafReader(reader)->postings(
      make_shared<Term>(L"foo", L"bar"), PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  TestUtil::assertEquals(-1, docsAndPositionsEnum->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertReused(L"foo", docsAndPositionsEnum, docsAndPositionsEnum2);
  TestUtil::assertEquals(-1, docsAndPositionsEnum2->docID());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextDoc());
  TestUtil::assertEquals(2, docsAndPositionsEnum2->freq());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(0, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(3, docsAndPositionsEnum2->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay1"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(1, docsAndPositionsEnum2->nextPosition());
  TestUtil::assertEquals(4, docsAndPositionsEnum2->startOffset());
  TestUtil::assertEquals(7, docsAndPositionsEnum2->endOffset());
  TestUtil::assertEquals(make_shared<BytesRef>(L"pay2"),
                         docsAndPositionsEnum2->getPayload());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

void BasePostingsFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  for (IndexOptions opts : IndexOptions::values()) {
    if (opts == IndexOptions::NONE) {
      continue;
    }
    shared_ptr<FieldType> ft = make_shared<FieldType>();
    ft->setIndexOptions(opts);
    ft->freeze();
    constexpr int numFields = random()->nextInt(5);
    for (int j = 0; j < numFields; ++j) {
      doc->push_back(make_shared<Field>(
          L"f_" + opts, TestUtil::randomSimpleString(random(), 2), ft));
    }
  }
}
} // namespace org::apache::lucene::index