using namespace std;

#include "TestPerFieldPostingsFormat2.h"

namespace org::apache::lucene::codecs::perfield
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using LuceneVarGapFixedInterval =
    org::apache::lucene::codecs::blockterms::LuceneVarGapFixedInterval;
using DirectPostingsFormat =
    org::apache::lucene::codecs::memory::DirectPostingsFormat;
using MemoryPostingsFormat =
    org::apache::lucene::codecs::memory::MemoryPostingsFormat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogDocMergePolicy = org::apache::lucene::index::LogDocMergePolicy;
using MergeState = org::apache::lucene::index::MergeState;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

shared_ptr<IndexWriter> TestPerFieldPostingsFormat2::newWriter(
    shared_ptr<Directory> dir,
    shared_ptr<IndexWriterConfig> conf) 
{
  shared_ptr<LogDocMergePolicy> logByteSizeMergePolicy =
      make_shared<LogDocMergePolicy>();
  logByteSizeMergePolicy->setNoCFSRatio(0.0); // make sure we use plain
  // files
  conf->setMergePolicy(logByteSizeMergePolicy);

  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(dir, conf);
  return writer;
}

void TestPerFieldPostingsFormat2::addDocs(shared_ptr<IndexWriter> writer,
                                          int numDocs) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
    writer->addDocument(doc);
  }
}

void TestPerFieldPostingsFormat2::addDocs2(shared_ptr<IndexWriter> writer,
                                           int numDocs) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"bbb", Field::Store::NO));
    writer->addDocument(doc);
  }
}

void TestPerFieldPostingsFormat2::addDocs3(shared_ptr<IndexWriter> writer,
                                           int numDocs) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"ccc", Field::Store::NO));
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    writer->addDocument(doc);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMergeUnusedPerFieldCodec() throws
// java.io.IOException
void TestPerFieldPostingsFormat2::testMergeUnusedPerFieldCodec() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwconf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(OpenMode::CREATE)
          ->setCodec(make_shared<MockCodec>());
  shared_ptr<IndexWriter> writer = newWriter(dir, iwconf);
  addDocs(writer, 10);
  writer->commit();
  addDocs3(writer, 10);
  writer->commit();
  addDocs2(writer, 10);
  writer->commit();
  TestUtil::assertEquals(30, writer->maxDoc());
  TestUtil::checkIndex(dir);
  writer->forceMerge(1);
  TestUtil::assertEquals(30, writer->maxDoc());
  delete writer;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testChangeCodecAndMerge() throws
// java.io.IOException
void TestPerFieldPostingsFormat2::testChangeCodecAndMerge() 
{
  shared_ptr<Directory> dir = newDirectory();
  if (VERBOSE) {
    wcout << L"TEST: make new index" << endl;
  }
  shared_ptr<IndexWriterConfig> iwconf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(OpenMode::CREATE)
          ->setCodec(make_shared<MockCodec>());
  iwconf->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  //((LogMergePolicy) iwconf.getMergePolicy()).setMergeFactor(10);
  shared_ptr<IndexWriter> writer = newWriter(dir, iwconf);

  addDocs(writer, 10);
  writer->commit();
  assertQuery(make_shared<Term>(L"content", L"aaa"), dir, 10);
  if (VERBOSE) {
    wcout << L"TEST: addDocs3" << endl;
  }
  addDocs3(writer, 10);
  writer->commit();
  delete writer;

  assertQuery(make_shared<Term>(L"content", L"ccc"), dir, 10);
  assertQuery(make_shared<Term>(L"content", L"aaa"), dir, 10);
  shared_ptr<Codec> codec = iwconf->getCodec();

  iwconf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setCodec(codec);
  //((LogMergePolicy) iwconf.getMergePolicy()).setNoCFSRatio(0.0);
  //((LogMergePolicy) iwconf.getMergePolicy()).setMergeFactor(10);
  iwconf->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);

  iwconf->setCodec(
      make_shared<MockCodec2>()); // uses standard for field content
  writer = newWriter(dir, iwconf);
  // swap in new codec for currently written segments
  if (VERBOSE) {
    wcout << L"TEST: add docs w/ Standard codec for content field" << endl;
  }
  addDocs2(writer, 10);
  writer->commit();
  codec = iwconf->getCodec();
  TestUtil::assertEquals(30, writer->maxDoc());
  assertQuery(make_shared<Term>(L"content", L"bbb"), dir, 10);
  assertQuery(make_shared<Term>(L"content", L"ccc"), dir, 10); ////
  assertQuery(make_shared<Term>(L"content", L"aaa"), dir, 10);

  if (VERBOSE) {
    wcout << L"TEST: add more docs w/ new codec" << endl;
  }
  addDocs2(writer, 10);
  writer->commit();
  assertQuery(make_shared<Term>(L"content", L"ccc"), dir, 10);
  assertQuery(make_shared<Term>(L"content", L"bbb"), dir, 20);
  assertQuery(make_shared<Term>(L"content", L"aaa"), dir, 10);
  TestUtil::assertEquals(40, writer->maxDoc());

  if (VERBOSE) {
    wcout << L"TEST: now optimize" << endl;
  }
  writer->forceMerge(1);
  TestUtil::assertEquals(40, writer->maxDoc());
  delete writer;
  assertQuery(make_shared<Term>(L"content", L"ccc"), dir, 10);
  assertQuery(make_shared<Term>(L"content", L"bbb"), dir, 20);
  assertQuery(make_shared<Term>(L"content", L"aaa"), dir, 10);

  delete dir;
}

void TestPerFieldPostingsFormat2::assertQuery(shared_ptr<Term> t,
                                              shared_ptr<Directory> dir,
                                              int num) 
{
  if (VERBOSE) {
    wcout << L"\nTEST: assertQuery " << t << endl;
  }
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TopDocs> search =
      searcher->search(make_shared<TermQuery>(t), num + 10);
  TestUtil::assertEquals(num, search->totalHits);
  delete reader;
}

shared_ptr<PostingsFormat>
TestPerFieldPostingsFormat2::MockCodec::getPostingsFormatForField(
    const wstring &field)
{
  if (field == L"id") {
    return direct;
  } else if (field == L"content") {
    return memory;
  } else {
    return luceneDefault;
  }
}

shared_ptr<PostingsFormat>
TestPerFieldPostingsFormat2::MockCodec2::getPostingsFormatForField(
    const wstring &field)
{
  if (field == L"id") {
    return direct;
  } else {
    return luceneDefault;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testStressPerFieldCodec() throws
// java.io.IOException
void TestPerFieldPostingsFormat2::testStressPerFieldCodec() 
{
  shared_ptr<Directory> dir = newDirectory(random());
  constexpr int docsPerRound = 97;
  int numRounds = atLeast(1);
  for (int i = 0; i < numRounds; i++) {
    int num = TestUtil::nextInt(random(), 30, 60);
    shared_ptr<IndexWriterConfig> config =
        newIndexWriterConfig(random(), make_shared<MockAnalyzer>(random()));
    config->setOpenMode(OpenMode::CREATE_OR_APPEND);
    shared_ptr<IndexWriter> writer = newWriter(dir, config);
    for (int j = 0; j < docsPerRound; j++) {
      shared_ptr<Document> *const doc = make_shared<Document>();
      for (int k = 0; k < num; k++) {
        shared_ptr<FieldType> customType =
            make_shared<FieldType>(TextField::TYPE_NOT_STORED);
        customType->setTokenized(random()->nextBoolean());
        customType->setOmitNorms(random()->nextBoolean());
        shared_ptr<Field> field = newField(
            L"" + to_wstring(k),
            TestUtil::randomRealisticUnicodeString(random(), 128), customType);
        doc->push_back(field);
      }
      writer->addDocument(doc);
    }
    if (random()->nextBoolean()) {
      writer->forceMerge(1);
    }
    writer->commit();
    TestUtil::assertEquals((i + 1) * docsPerRound, writer->maxDoc());
    delete writer;
  }
  delete dir;
}

void TestPerFieldPostingsFormat2::testSameCodecDifferentInstance() throw(
    runtime_error)
{
  shared_ptr<Codec> codec =
      make_shared<AssertingCodecAnonymousInnerClass>(shared_from_this());
  doTestMixedPostings(codec);
}

TestPerFieldPostingsFormat2::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestPerFieldPostingsFormat2> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PostingsFormat>
TestPerFieldPostingsFormat2::AssertingCodecAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  if (L"id" == field) {
    return make_shared<MemoryPostingsFormat>();
  } else if (L"date" == field) {
    return make_shared<MemoryPostingsFormat>();
  } else {
    return outerInstance->super->getPostingsFormatForField(field);
  }
}

void TestPerFieldPostingsFormat2::testSameCodecDifferentParams() throw(
    runtime_error)
{
  shared_ptr<Codec> codec =
      make_shared<AssertingCodecAnonymousInnerClass2>(shared_from_this());
  doTestMixedPostings(codec);
}

TestPerFieldPostingsFormat2::AssertingCodecAnonymousInnerClass2::
    AssertingCodecAnonymousInnerClass2(
        shared_ptr<TestPerFieldPostingsFormat2> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PostingsFormat>
TestPerFieldPostingsFormat2::AssertingCodecAnonymousInnerClass2::
    getPostingsFormatForField(const wstring &field)
{
  if (L"id" == field) {
    return make_shared<LuceneVarGapFixedInterval>(1);
  } else if (L"date" == field) {
    return make_shared<LuceneVarGapFixedInterval>(2);
  } else {
    return outerInstance->super->getPostingsFormatForField(field);
  }
}

void TestPerFieldPostingsFormat2::doTestMixedPostings(
    shared_ptr<Codec> codec) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(codec);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  // turn on vectors for the checkindex cross-check
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  ft->setStoreTermVectorPositions(true);
  shared_ptr<Field> idField = make_shared<Field>(L"id", L"", ft);
  shared_ptr<Field> dateField = make_shared<Field>(L"date", L"", ft);
  doc->push_back(idField);
  doc->push_back(dateField);
  for (int i = 0; i < 100; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(random()->nextInt(50)));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    dateField->setStringValue(Integer::toString(random()->nextInt(100)));
    iw->addDocument(doc);
  }
  delete iw;
  delete dir; // checkindex
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("deprecation") public void
// testMergeCalledOnTwoFormats() throws java.io.IOException
void TestPerFieldPostingsFormat2::testMergeCalledOnTwoFormats() throw(
    IOException)
{
  shared_ptr<MergeRecordingPostingsFormatWrapper> pf1 =
      make_shared<MergeRecordingPostingsFormatWrapper>(
          TestUtil::getDefaultPostingsFormat());
  shared_ptr<MergeRecordingPostingsFormatWrapper> pf2 =
      make_shared<MergeRecordingPostingsFormatWrapper>(
          TestUtil::getDefaultPostingsFormat());

  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>();
  iwc->setCodec(make_shared<AssertingCodecAnonymousInnerClass3>(
      shared_from_this(), pf1, pf2));

  shared_ptr<Directory> directory = newDirectory();

  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f1", L"val1", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"f2", L"val2", Field::Store::YES));
  doc->push_back(make_shared<IntPoint>(
      L"f3", 3)); // Points are not indexed as postings and should not appear in
                  // the merge fields
  doc->push_back(make_shared<StringField>(L"f4", L"val4", Field::Store::NO));
  iwriter->addDocument(doc);
  iwriter->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f1", L"val5", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"f2", L"val6", Field::Store::YES));
  doc->push_back(make_shared<IntPoint>(L"f3", 7));
  doc->push_back(make_shared<StringField>(L"f4", L"val8", Field::Store::NO));
  iwriter->addDocument(doc);
  iwriter->commit();

  iwriter->forceMerge(1, true);
  delete iwriter;

  TestUtil::assertEquals(1, pf1->nbMergeCalls);
  TestUtil::assertEquals(unordered_set<>(Arrays::asList(L"f1", L"f2")),
                         unordered_set<>(pf1->fieldNames));
  TestUtil::assertEquals(1, pf2->nbMergeCalls);
  TestUtil::assertEquals(Collections::singletonList(L"f4"), pf2->fieldNames);

  delete directory;
}

TestPerFieldPostingsFormat2::AssertingCodecAnonymousInnerClass3::
    AssertingCodecAnonymousInnerClass3(
        shared_ptr<TestPerFieldPostingsFormat2> outerInstance,
        shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldPostingsFormat2::
                MergeRecordingPostingsFormatWrapper>
            pf1,
        shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldPostingsFormat2::
                MergeRecordingPostingsFormatWrapper>
            pf2)
{
  this->outerInstance = outerInstance;
  this->pf1 = pf1;
  this->pf2 = pf2;
}

shared_ptr<PostingsFormat>
TestPerFieldPostingsFormat2::AssertingCodecAnonymousInnerClass3::
    getPostingsFormatForField(const wstring &field)
{
  switch (field) {
  case L"f1":
  case L"f2":
    return pf1;

  case L"f3":
  case L"f4":
    return pf2;

  default:
    return outerInstance->super->getPostingsFormatForField(field);
  }
}

TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    MergeRecordingPostingsFormatWrapper(shared_ptr<PostingsFormat> delegate_)
    : org::apache::lucene::codecs::PostingsFormat(delegate_->getName()),
      delegate_(delegate_)
{
}

shared_ptr<FieldsConsumer>
TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    fieldsConsumer(shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<FieldsConsumer> *const consumer = delegate_->fieldsConsumer(state);
  return make_shared<FieldsConsumerAnonymousInnerClass>(shared_from_this(),
                                                        consumer);
}

TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    FieldsConsumerAnonymousInnerClass::FieldsConsumerAnonymousInnerClass(
        shared_ptr<MergeRecordingPostingsFormatWrapper> outerInstance,
        shared_ptr<FieldsConsumer> consumer)
{
  this->outerInstance = outerInstance;
  this->consumer = consumer;
}

void TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    FieldsConsumerAnonymousInnerClass::write(shared_ptr<Fields> fields) throw(
        IOException)
{
  consumer->write(fields);
}

void TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    FieldsConsumerAnonymousInnerClass::merge(
        shared_ptr<MergeState> mergeState) 
{
  outerInstance->nbMergeCalls++;
  for (auto fi : mergeState->mergeFieldInfos) {
    outerInstance->fieldNames.push_back(fi->name);
  }
  consumer->merge(mergeState);
}

TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    FieldsConsumerAnonymousInnerClass::~FieldsConsumerAnonymousInnerClass()
{
  delete consumer;
}

shared_ptr<FieldsProducer>
TestPerFieldPostingsFormat2::MergeRecordingPostingsFormatWrapper::
    fieldsProducer(shared_ptr<SegmentReadState> state) 
{
  return delegate_->fieldsProducer(state);
}
} // namespace org::apache::lucene::codecs::perfield