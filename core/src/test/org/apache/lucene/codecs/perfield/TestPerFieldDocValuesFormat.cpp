using namespace std;

#include "TestPerFieldDocValuesFormat.h"

namespace org::apache::lucene::codecs::perfield
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using BaseDocValuesFormatTestCase =
    org::apache::lucene::index::BaseDocValuesFormatTestCase;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomCodec = org::apache::lucene::index::RandomCodec;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPerFieldDocValuesFormat::setUp() 
{
  codec = make_shared<RandomCodec>(make_shared<Random>(random()->nextLong()),
                                   Collections::emptySet<wstring>());
  BaseDocValuesFormatTestCase::setUp();
}

shared_ptr<Codec> TestPerFieldDocValuesFormat::getCodec() { return codec; }

bool TestPerFieldDocValuesFormat::codecAcceptsHugeBinaryValues(
    const wstring &field)
{
  return TestUtil::fieldSupportsHugeBinaryDocValues(field);
}

void TestPerFieldDocValuesFormat::testTwoFieldsTwoFormats() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  // we don't use RandomIndexWriter because it might add more docvalues than we
  // expect !!!!1
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  shared_ptr<DocValuesFormat> *const fast =
      TestUtil::getDefaultDocValuesFormat();
  shared_ptr<DocValuesFormat> *const slow = DocValuesFormat::forName(L"Memory");
  iwc->setCodec(make_shared<AssertingCodecAnonymousInnerClass>(
      shared_from_this(), fast, slow));
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 5));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv2", make_shared<BytesRef>(L"hello world")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = newSearcher(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int hitDocID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(hitDocID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<NumericDocValues> dv =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv1");
    TestUtil::assertEquals(hitDocID, dv->advance(hitDocID));
    TestUtil::assertEquals(5, dv->longValue());

    shared_ptr<BinaryDocValues> dv2 =
        ireader->leaves()[0]->reader().getBinaryDocValues(L"dv2");
    TestUtil::assertEquals(hitDocID, dv2->advance(hitDocID));
    shared_ptr<BytesRef> *const term = dv2->binaryValue();
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello world"), term);
  }

  delete ireader;
  delete directory;
}

TestPerFieldDocValuesFormat::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestPerFieldDocValuesFormat> outerInstance,
        shared_ptr<DocValuesFormat> fast, shared_ptr<DocValuesFormat> slow)
{
  this->outerInstance = outerInstance;
  this->fast = fast;
  this->slow = slow;
}

shared_ptr<DocValuesFormat>
TestPerFieldDocValuesFormat::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  if (L"dv1" == field) {
    return fast;
  } else {
    return slow;
  }
}

void TestPerFieldDocValuesFormat::testMergeCalledOnTwoFormats() throw(
    IOException)
{
  shared_ptr<MergeRecordingDocValueFormatWrapper> dvf1 =
      make_shared<MergeRecordingDocValueFormatWrapper>(
          TestUtil::getDefaultDocValuesFormat());
  shared_ptr<MergeRecordingDocValueFormatWrapper> dvf2 =
      make_shared<MergeRecordingDocValueFormatWrapper>(
          TestUtil::getDefaultDocValuesFormat());

  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>();
  iwc->setCodec(make_shared<AssertingCodecAnonymousInnerClass2>(
      shared_from_this(), dvf1, dvf2));

  shared_ptr<Directory> directory = newDirectory();

  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 5));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv2", 42));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv3", make_shared<BytesRef>(L"hello world")));
  iwriter->addDocument(doc);
  iwriter->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 8));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv2", 45));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv3", make_shared<BytesRef>(L"goodbye world")));
  iwriter->addDocument(doc);
  iwriter->commit();

  iwriter->forceMerge(1, true);
  delete iwriter;

  TestUtil::assertEquals(1, dvf1->nbMergeCalls);
  TestUtil::assertEquals(unordered_set<>(Arrays::asList(L"dv1", L"dv2")),
                         unordered_set<>(dvf1->fieldNames));
  TestUtil::assertEquals(1, dvf2->nbMergeCalls);
  TestUtil::assertEquals(Collections::singletonList(L"dv3"), dvf2->fieldNames);

  delete directory;
}

TestPerFieldDocValuesFormat::AssertingCodecAnonymousInnerClass2::
    AssertingCodecAnonymousInnerClass2(
        shared_ptr<TestPerFieldDocValuesFormat> outerInstance,
        shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldDocValuesFormat::
                MergeRecordingDocValueFormatWrapper>
            dvf1,
        shared_ptr<
            org::apache::lucene::codecs::perfield::TestPerFieldDocValuesFormat::
                MergeRecordingDocValueFormatWrapper>
            dvf2)
{
  this->outerInstance = outerInstance;
  this->dvf1 = dvf1;
  this->dvf2 = dvf2;
}

shared_ptr<DocValuesFormat>
TestPerFieldDocValuesFormat::AssertingCodecAnonymousInnerClass2::
    getDocValuesFormatForField(const wstring &field)
{
  switch (field) {
  case L"dv1":
  case L"dv2":
    return dvf1;

  case L"dv3":
    return dvf2;

  default:
    return outerInstance->super->getDocValuesFormatForField(field);
  }
}

TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    MergeRecordingDocValueFormatWrapper(shared_ptr<DocValuesFormat> delegate_)
    : org::apache::lucene::codecs::DocValuesFormat(delegate_->getName()),
      delegate_(delegate_)
{
}

shared_ptr<DocValuesConsumer>
TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    fieldsConsumer(shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<DocValuesConsumer> *const consumer =
      delegate_->fieldsConsumer(state);
  return make_shared<DocValuesConsumerAnonymousInnerClass>(shared_from_this(),
                                                           consumer);
}

TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::DocValuesConsumerAnonymousInnerClass(
        shared_ptr<MergeRecordingDocValueFormatWrapper> outerInstance,
        shared_ptr<DocValuesConsumer> consumer)
{
  this->outerInstance = outerInstance;
  this->consumer = consumer;
}

void TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::addNumericField(
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> values) 
{
  consumer->addNumericField(field, values);
}

void TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::addBinaryField(
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> values) 
{
  consumer->addBinaryField(field, values);
}

void TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::addSortedField(
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> values) 
{
  consumer->addSortedField(field, values);
}

void TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::addSortedNumericField(
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> values) 
{
  consumer->addSortedNumericField(field, values);
}

void TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::addSortedSetField(
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> values) 
{
  consumer->addSortedSetField(field, values);
}

void TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::merge(
        shared_ptr<MergeState> mergeState) 
{
  outerInstance->nbMergeCalls++;
  for (auto fi : mergeState->mergeFieldInfos) {
    outerInstance->fieldNames.push_back(fi->name);
  }
  consumer->merge(mergeState);
}

TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    DocValuesConsumerAnonymousInnerClass::
        ~DocValuesConsumerAnonymousInnerClass()
{
  delete consumer;
}

shared_ptr<DocValuesProducer>
TestPerFieldDocValuesFormat::MergeRecordingDocValueFormatWrapper::
    fieldsProducer(shared_ptr<SegmentReadState> state) 
{
  return delegate_->fieldsProducer(state);
}
} // namespace org::apache::lucene::codecs::perfield