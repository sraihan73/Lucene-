using namespace std;

#include "TestIndexSorting.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using FieldType = org::apache::lucene::document::FieldType;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

TestIndexSorting::AssertingNeedsIndexSortCodec::AssertingNeedsIndexSortCodec()
    : org::apache::lucene::codecs::FilterCodec(
          TestUtil::getDefaultCodec()->getName(), TestUtil::getDefaultCodec())
{
}

shared_ptr<PointsFormat>
TestIndexSorting::AssertingNeedsIndexSortCodec::pointsFormat()
{
  shared_ptr<PointsFormat> *const pf = delegate_->pointsFormat();
  return make_shared<PointsFormatAnonymousInnerClass>(shared_from_this(), pf);
}

TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass(
        shared_ptr<AssertingNeedsIndexSortCodec> outerInstance,
        shared_ptr<PointsFormat> pf)
{
  this->outerInstance = outerInstance;
  this->pf = pf;
}

shared_ptr<PointsWriter> TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::fieldsWriter(
        shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PointsWriter> *const writer = pf->fieldsWriter(state);
  return make_shared<PointsWriterAnonymousInnerClass>(shared_from_this(),
                                                      writer);
}

TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::PointsWriterAnonymousInnerClass::
        PointsWriterAnonymousInnerClass(
            shared_ptr<PointsFormatAnonymousInnerClass> outerInstance,
            shared_ptr<PointsWriter> writer)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
}

void TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::PointsWriterAnonymousInnerClass::merge(
        shared_ptr<MergeState> mergeState) 
{
  // For single segment merge we cannot infer if the segment is already sorted
  // or not.
  if (mergeState->docMaps.size() > 1) {
    TestUtil::assertEquals(outerInstance->outerInstance.needsIndexSort,
                           mergeState->needsIndexSort);
  }
  ++outerInstance->outerInstance.numCalls;
  writer->merge(mergeState);
}

void TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::PointsWriterAnonymousInnerClass::
        writeField(shared_ptr<FieldInfo> fieldInfo,
                   shared_ptr<PointsReader> values) 
{
  writer->writeField(fieldInfo, values);
}

void TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::PointsWriterAnonymousInnerClass::
        finish() 
{
  writer->finish();
}

TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::PointsWriterAnonymousInnerClass::
        ~PointsWriterAnonymousInnerClass()
{
  delete writer;
}

shared_ptr<PointsReader> TestIndexSorting::AssertingNeedsIndexSortCodec::
    PointsFormatAnonymousInnerClass::fieldsReader(
        shared_ptr<SegmentReadState> state) 
{
  return pf->fieldsReader(state);
}

void TestIndexSorting::assertNeedsIndexSortMerge(
    shared_ptr<SortField> sortField,
    function<void(Document *)> &defaultValueConsumer,
    function<void(Document *)> &randomValueConsumer) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<AssertingNeedsIndexSortCodec> codec =
      make_shared<AssertingNeedsIndexSortCodec>();
  iwc->setCodec(codec);
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      sortField, make_shared<SortField>(L"id", SortField::Type::INT));
  iwc->setIndexSort(indexSort);
  shared_ptr<LogMergePolicy> policy = newLogMergePolicy();
  // make sure that merge factor is always > 2
  if (policy->getMergeFactor() <= 2) {
    policy->setMergeFactor(3);
  }
  iwc->setMergePolicy(policy);

  // add already sorted documents
  codec->numCalls = 0;
  codec->needsIndexSort = false;
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  bool withValues = random()->nextBoolean();
  for (int i = 100; i < 200; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    doc->push_back(make_shared<IntPoint>(L"point", random()->nextInt()));
    if (withValues) {
      defaultValueConsumer(doc);
    }
    w->addDocument(doc);
    if (i % 10 == 0) {
      w->commit();
    }
  }
  shared_ptr<Set<int>> deletedDocs = unordered_set<int>();
  int num = random()->nextInt(20);
  for (int i = 0; i < num; i++) {
    int nextDoc = random()->nextInt(100);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    w->deleteDocuments({make_shared<Term>(L"id", Integer::toString(nextDoc))});
    deletedDocs->add(nextDoc);
  }
  w->commit();
  w->waitForMerges();
  w->forceMerge(1);
  assertTrue(codec->numCalls > 0);

  // merge sort is needed
  codec->numCalls = 0;
  codec->needsIndexSort = true;
  for (int i = 10; i >= 0; i--) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    doc->push_back(make_shared<IntPoint>(L"point", random()->nextInt()));
    if (withValues) {
      defaultValueConsumer(doc);
    }
    w->addDocument(doc);
    w->commit();
  }
  w->commit();
  w->waitForMerges();
  w->forceMerge(1);
  assertTrue(codec->numCalls > 0);

  // segment sort is needed
  codec->needsIndexSort = true;
  codec->numCalls = 0;
  for (int i = 201; i < 300; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    doc->push_back(make_shared<IntPoint>(L"point", random()->nextInt()));
    randomValueConsumer(doc);
    w->addDocument(doc);
    if (i % 10 == 0) {
      w->commit();
    }
  }
  w->commit();
  w->waitForMerges();
  w->forceMerge(1);
  assertTrue(codec->numCalls > 0);

  delete w;
  delete dir;
}

void TestIndexSorting::testNumericAlreadySorted() 
{
  assertNeedsIndexSortMerge(
      make_shared<SortField>(L"foo", SortField::Type::INT),
      [&](doc) {
        doc->add(
            make_shared<org::apache::lucene::document::NumericDocValuesField>(
                L"foo", 0));
      },
      [&](doc) {
        doc->add(
            make_shared<org::apache::lucene::document::NumericDocValuesField>(
                L"foo", random()->nextInt()));
      });
}

void TestIndexSorting::testStringAlreadySorted() 
{
  assertNeedsIndexSortMerge(
      make_shared<SortField>(L"foo", SortField::Type::STRING),
      [&](doc) {
        doc->add(
            make_shared<org::apache::lucene::document::SortedDocValuesField>(
                L"foo",
                make_shared<org::apache::lucene::util::BytesRef>(L"default")));
      },
      [&](doc) {
        doc->add(
            make_shared<org::apache::lucene::document::SortedDocValuesField>(
                L"foo", org::apache::lucene::util::TestUtil::randomBinaryTerm(
                            random())));
      });
}

void TestIndexSorting::testMultiValuedNumericAlreadySorted() throw(
    runtime_error)
{
  assertNeedsIndexSortMerge(
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::INT),
      [&](doc) {
        doc->add(make_shared<SortedNumericDocValuesField>(
            L"foo", numeric_limits<int>::min()));
        int num = random()->nextInt(5);
        for (int j = 0; j < num; j++) {
          doc->add(make_shared<SortedNumericDocValuesField>(
              L"foo", random()->nextInt()));
        }
      },
      [&](doc) {
        int num = random()->nextInt(5);
        for (int j = 0; j < num; j++) {
          doc->add(make_shared<SortedNumericDocValuesField>(
              L"foo", random()->nextInt()));
        }
      });
}

void TestIndexSorting::testMultiValuedStringAlreadySorted() 
{
  assertNeedsIndexSortMerge(
      make_shared<SortedSetSortField>(L"foo", false),
      [&](doc) {
        doc->add(make_shared<SortedSetDocValuesField>(
            L"foo", make_shared<BytesRef>(L"")));
        int num = random()->nextInt(5);
        for (int j = 0; j < num; j++) {
          doc->add(make_shared<SortedSetDocValuesField>(
              L"foo", TestUtil::randomBinaryTerm(random())));
        }
      },
      [&](doc) {
        int num = random()->nextInt(5);
        for (int j = 0; j < num; j++) {
          doc->add(make_shared<SortedSetDocValuesField>(
              L"foo", TestUtil::randomBinaryTerm(random())));
        }
      });
}

void TestIndexSorting::testBasicString() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortField>(L"foo", SortField::Type::STRING));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"zzz")));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"aaa")));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"mmm")));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<SortedDocValues> values = leaf->getSortedDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(L"aaa", values->binaryValue()->utf8ToString());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(L"mmm", values->binaryValue()->utf8ToString());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(L"zzz", values->binaryValue()->utf8ToString());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicMultiValuedString() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortedSetSortField>(L"foo", false));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzz")));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"aaa")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzz")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"bcg")));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"mmm")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"pppp")));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1LL, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2LL, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3LL, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingStringFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::STRING);
  sortField->setMissingValue(SortField::STRING_FIRST);
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"zzz")));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"mmm")));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<SortedDocValues> values = leaf->getSortedDocValues(L"foo");
  // docID 0 is missing:
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(L"mmm", values->binaryValue()->utf8ToString());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(L"zzz", values->binaryValue()->utf8ToString());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedStringFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedSetSortField>(L"foo", false);
  sortField->setMissingValue(SortField::STRING_FIRST);
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzz")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzza")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzzd")));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"mmm")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"nnnn")));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1LL, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2LL, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3LL, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingStringLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::STRING);
  sortField->setMissingValue(SortField::STRING_LAST);
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"zzz")));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"mmm")));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<SortedDocValues> values = leaf->getSortedDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(L"mmm", values->binaryValue()->utf8ToString());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(L"zzz", values->binaryValue()->utf8ToString());
  TestUtil::assertEquals(NO_MORE_DOCS, values->nextDoc());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedStringLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedSetSortField>(L"foo", false);
  sortField->setMissingValue(SortField::STRING_LAST);
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzz")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"zzzd")));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"mmm")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"ppp")));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1LL, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2LL, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3LL, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicLong() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 18));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", -1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 7));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(-1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(7, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(18, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicMultiValuedLong() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 18));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 35));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", -1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 7));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 22));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingLongFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::LONG);
  sortField->setMissingValue(
      static_cast<int64_t>(numeric_limits<int64_t>::min()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 18));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 7));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  // docID 0 has no value
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(7, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(18, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedLongFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::LONG);
  sortField->setMissingValue(
      static_cast<int64_t>(numeric_limits<int64_t>::min()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 18));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 27));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 7));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 24));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingLongLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::LONG);
  sortField->setMissingValue(
      static_cast<int64_t>(numeric_limits<int64_t>::max()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 18));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 7));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(7, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(18, values->longValue());
  TestUtil::assertEquals(NO_MORE_DOCS, values->nextDoc());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedLongLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::LONG);
  sortField->setMissingValue(
      static_cast<int64_t>(numeric_limits<int64_t>::max()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 18));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 65));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 7));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 34));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 74));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicInt() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::INT));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 18));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", -1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 7));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(-1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(7, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(18, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicMultiValuedInt() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::INT));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 18));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 34));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", -1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 34));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 7));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 22));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 27));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingIntFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::INT);
  sortField->setMissingValue(static_cast<Integer>(numeric_limits<int>::min()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 18));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 7));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(7, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(18, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedIntFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::INT);
  sortField->setMissingValue(static_cast<Integer>(numeric_limits<int>::min()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 18));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 187667));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 7));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 34));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingIntLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::INT);
  sortField->setMissingValue(static_cast<Integer>(numeric_limits<int>::max()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 18));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 7));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(7, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(18, values->longValue());
  TestUtil::assertEquals(NO_MORE_DOCS, values->nextDoc());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedIntLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::INT);
  sortField->setMissingValue(static_cast<Integer>(numeric_limits<int>::max()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 18));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 6372));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 7));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 8));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicDouble() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortField>(L"foo", SortField::Type::DOUBLE));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", 18.0));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", -1.0));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", 7.0));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  assertEquals(-1.0, Double::longBitsToDouble(values->longValue()), 0.0);
  TestUtil::assertEquals(1, values->nextDoc());
  assertEquals(7.0, Double::longBitsToDouble(values->longValue()), 0.0);
  TestUtil::assertEquals(2, values->nextDoc());
  assertEquals(18.0, Double::longBitsToDouble(values->longValue()), 0.0);
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicMultiValuedDouble() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::DOUBLE));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(7.54)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(27.0)));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(-1.0)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(0.0)));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(7.0)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(7.67)));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingDoubleFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::DOUBLE);
  sortField->setMissingValue(-numeric_limits<double>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", 18.0));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", 7.0));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(1, values->nextDoc());
  assertEquals(7.0, Double::longBitsToDouble(values->longValue()), 0.0);
  TestUtil::assertEquals(2, values->nextDoc());
  assertEquals(18.0, Double::longBitsToDouble(values->longValue()), 0.0);
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedDoubleFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::DOUBLE);
  sortField->setMissingValue(-numeric_limits<double>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(18.0)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(18.76)));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(7.0)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(70.0)));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingDoubleLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::DOUBLE);
  sortField->setMissingValue(numeric_limits<double>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", 18.0));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"foo", 7.0));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  assertEquals(7.0, Double::longBitsToDouble(values->longValue()), 0.0);
  TestUtil::assertEquals(1, values->nextDoc());
  assertEquals(18.0, Double::longBitsToDouble(values->longValue()), 0.0);
  TestUtil::assertEquals(NO_MORE_DOCS, values->nextDoc());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedDoubleLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::DOUBLE);
  sortField->setMissingValue(numeric_limits<double>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(18.0)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(8262.0)));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(7.0)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::doubleToSortableLong(7.87)));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicFloat() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::FLOAT));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", 18.0f));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", -1.0f));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", 7.0f));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  assertEquals(-1.0f,
               Float::intBitsToFloat(static_cast<int>(values->longValue())),
               0.0f);
  TestUtil::assertEquals(1, values->nextDoc());
  assertEquals(
      7.0f, Float::intBitsToFloat(static_cast<int>(values->longValue())), 0.0f);
  TestUtil::assertEquals(2, values->nextDoc());
  assertEquals(18.0f,
               Float::intBitsToFloat(static_cast<int>(values->longValue())),
               0.0f);
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBasicMultiValuedFloat() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::FLOAT));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(18.0f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(29.0f)));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(-1.0f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(34.0f)));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(7.0f)));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingFloatFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::FLOAT);
  sortField->setMissingValue(-numeric_limits<float>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", 18.0f));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", 7.0f));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(1, values->nextDoc());
  assertEquals(
      7.0f, Float::intBitsToFloat(static_cast<int>(values->longValue())), 0.0f);
  TestUtil::assertEquals(2, values->nextDoc());
  assertEquals(18.0f,
               Float::intBitsToFloat(static_cast<int>(values->longValue())),
               0.0f);
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedFloatFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::FLOAT);
  sortField->setMissingValue(-numeric_limits<float>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(18.0f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(726.0f)));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(7.0f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(18.0f)));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingFloatLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"foo", SortField::Type::FLOAT);
  sortField->setMissingValue(numeric_limits<float>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", 18.0f));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  w->addDocument(make_shared<Document>());
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"foo", 7.0f));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"foo");
  TestUtil::assertEquals(0, values->nextDoc());
  assertEquals(
      7.0f, Float::intBitsToFloat(static_cast<int>(values->longValue())), 0.0f);
  TestUtil::assertEquals(1, values->nextDoc());
  assertEquals(18.0f,
               Float::intBitsToFloat(static_cast<int>(values->longValue())),
               0.0f);
  TestUtil::assertEquals(NO_MORE_DOCS, values->nextDoc());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMissingMultiValuedFloatLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::FLOAT);
  sortField->setMissingValue(numeric_limits<float>::infinity());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 2));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(726.0f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(18.0f)));
  w->addDocument(doc);
  // so we get more than one segment, so that forceMerge actually does merge,
  // since we only get a sorted segment by merging:
  w->commit();

  // missing
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 3));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", 1));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(12.67f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"foo", NumericUtils::floatToSortableInt(7.0f)));
  w->addDocument(doc);
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<LeafReader> leaf = getOnlyLeafReader(r);
  TestUtil::assertEquals(3, leaf->maxDoc());
  shared_ptr<NumericDocValues> values = leaf->getNumericDocValues(L"id");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(1, values->longValue());
  TestUtil::assertEquals(1, values->nextDoc());
  TestUtil::assertEquals(2, values->longValue());
  TestUtil::assertEquals(2, values->nextDoc());
  TestUtil::assertEquals(3, values->longValue());
  r->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testRandom1() 
{
  bool withDeletes = random()->nextBoolean();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  constexpr int numDocs = atLeast(1000);
  shared_ptr<FixedBitSet> *const deleted = make_shared<FixedBitSet>(numDocs);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<NumericDocValuesField>(L"foo", random()->nextInt(20)));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    w->addDocument(doc);
    if (random()->nextInt(5) == 0) {
      w->getReader()->close();
    } else if (random()->nextInt(30) == 0) {
      w->forceMerge(2);
    } else if (random()->nextInt(4) == 0) {
      constexpr int id = TestUtil::nextInt(random(), 0, i);
      deleted->set(id);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      w->deleteDocuments({make_shared<Term>(L"id", Integer::toString(id))});
    }
  }

  // Check that segments are sorted
  shared_ptr<DirectoryReader> reader = w->getReader();
  for (shared_ptr<LeafReaderContext> ctx : reader->leaves()) {
    shared_ptr<SegmentReader> *const leaf =
        std::static_pointer_cast<SegmentReader>(ctx->reader());
    shared_ptr<SegmentInfo> info = leaf->getSegmentInfo()->info;
    switch (info->getDiagnostics()[IndexWriter::SOURCE]) {
    case IndexWriter::SOURCE_FLUSH:
    case IndexWriter::SOURCE_MERGE: {
      TestUtil::assertEquals(indexSort, info->getIndexSort());
      shared_ptr<NumericDocValues> *const values =
          leaf->getNumericDocValues(L"foo");
      int64_t previous = numeric_limits<int64_t>::min();
      for (int i = 0; i < leaf->maxDoc(); ++i) {
        TestUtil::assertEquals(i, values->nextDoc());
        constexpr int64_t value = values->longValue();
        assertTrue(value >= previous);
        previous = value;
      }
      break;
    }
    default:
      fail();
    }
  }

  // Now check that the index is consistent
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  for (int i = 0; i < numDocs; ++i) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<TermQuery> termQuery =
        make_shared<TermQuery>(make_shared<Term>(L"id", Integer::toString(i)));
    shared_ptr<TopDocs> *const topDocs = searcher->search(termQuery, 1);
    if (deleted->get(i)) {
      TestUtil::assertEquals(0, topDocs->totalHits);
    } else {
      TestUtil::assertEquals(1, topDocs->totalHits);
      shared_ptr<NumericDocValues> values =
          MultiDocValues::getNumericValues(reader, L"id");
      TestUtil::assertEquals(topDocs->scoreDocs[0]->doc,
                             values->advance(topDocs->scoreDocs[0]->doc));
      TestUtil::assertEquals(i, values->longValue());
      shared_ptr<Document> document =
          reader->document(topDocs->scoreDocs[0]->doc);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(Integer::toString(i), document[L"id"]);
    }
  }

  reader->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testMultiValuedRandom1() 
{
  bool withDeletes = random()->nextBoolean();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  constexpr int numDocs = atLeast(1000);
  shared_ptr<FixedBitSet> *const deleted = make_shared<FixedBitSet>(numDocs);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    int num = random()->nextInt(10);
    for (int j = 0; j < num; j++) {
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"foo", random()->nextInt(2000)));
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    w->addDocument(doc);
    if (random()->nextInt(5) == 0) {
      w->getReader()->close();
    } else if (random()->nextInt(30) == 0) {
      w->forceMerge(2);
    } else if (random()->nextInt(4) == 0) {
      constexpr int id = TestUtil::nextInt(random(), 0, i);
      deleted->set(id);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      w->deleteDocuments({make_shared<Term>(L"id", Integer::toString(id))});
    }
  }

  shared_ptr<DirectoryReader> reader = w->getReader();
  // Now check that the index is consistent
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  for (int i = 0; i < numDocs; ++i) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<TermQuery> termQuery =
        make_shared<TermQuery>(make_shared<Term>(L"id", Integer::toString(i)));
    shared_ptr<TopDocs> *const topDocs = searcher->search(termQuery, 1);
    if (deleted->get(i)) {
      TestUtil::assertEquals(0, topDocs->totalHits);
    } else {
      TestUtil::assertEquals(1, topDocs->totalHits);
      shared_ptr<NumericDocValues> values =
          MultiDocValues::getNumericValues(reader, L"id");
      TestUtil::assertEquals(topDocs->scoreDocs[0]->doc,
                             values->advance(topDocs->scoreDocs[0]->doc));
      TestUtil::assertEquals(i, values->longValue());
      shared_ptr<Document> document =
          reader->document(topDocs->scoreDocs[0]->doc);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(Integer::toString(i), document[L"id"]);
    }
  }

  reader->close();
  delete w;
  delete dir;
}

TestIndexSorting::UpdateRunnable::UpdateRunnable(
    int numDocs, shared_ptr<Random> random, shared_ptr<CountDownLatch> latch,
    shared_ptr<AtomicInteger> updateCount, shared_ptr<IndexWriter> w,
    unordered_map<int, int64_t> &values)
    : numDocs(numDocs), random(random), updateCount(updateCount), w(w),
      values(values), latch(latch)
{
}

void TestIndexSorting::UpdateRunnable::run()
{
  try {
    latch->await();
    while (updateCount->decrementAndGet() >= 0) {
      constexpr int id = random->nextInt(numDocs);
      constexpr int64_t value = random->nextInt(20);
      shared_ptr<Document> doc = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StringField>(L"id", Integer::toString(id),
                                              Field::Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"foo", value));

      {
        lock_guard<mutex> lock(values);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        w->updateDocument(make_shared<Term>(L"id", Integer::toString(id)), doc);
        values.emplace(id, value);
      }

      switch (random->nextInt(10)) {
      case 0:
      case 1:
        // reopen
        DirectoryReader::open(w)->close();
        break;
      case 2:
        w->forceMerge(3);
        break;
      }
    }
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (IOException | InterruptedException e) {
    throw runtime_error(e);
  }
}

void TestIndexSorting::testConcurrentUpdates() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  unordered_map<int, int64_t> values = unordered_map<int, int64_t>();

  constexpr int numDocs = atLeast(100);
  std::deque<std::shared_ptr<Thread>> threads(2);

  shared_ptr<AtomicInteger> *const updateCount =
      make_shared<AtomicInteger>(atLeast(1000));
  shared_ptr<CountDownLatch> *const latch = make_shared<CountDownLatch>(1);
  for (int i = 0; i < threads.size(); ++i) {
    shared_ptr<Random> r = make_shared<Random>(random()->nextLong());
    threads[i] = make_shared<Thread>(
        make_shared<UpdateRunnable>(numDocs, r, latch, updateCount, w, values));
  }
  for (auto thread : threads) {
    thread->start();
  }
  latch->countDown();
  for (auto thread : threads) {
    thread->join();
  }
  w->forceMerge(1);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  for (int i = 0; i < numDocs; ++i) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<TopDocs> *const topDocs = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"id", Integer::toString(i))),
        1);
    if (values.find(i) != values.end() == false) {
      TestUtil::assertEquals(0, topDocs->totalHits);
    } else {
      TestUtil::assertEquals(1, topDocs->totalHits);
      shared_ptr<NumericDocValues> dvs =
          MultiDocValues::getNumericValues(reader, L"foo");
      int docID = topDocs->scoreDocs[0]->doc;
      TestUtil::assertEquals(docID, dvs->advance(docID));
      TestUtil::assertEquals(values[i], dvs->longValue());
    }
  }
  reader->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testBadDVUpdate() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", make_shared<BytesRef>(L"0"),
                                          Field::Store::NO));
  doc->push_back(
      make_shared<NumericDocValuesField>(L"foo", random()->nextInt()));
  w->addDocument(doc);
  w->commit();
  invalid_argument exc = expectThrows(invalid_argument::typeid, [&]() {
    w->updateDocValues(
        make_shared<Term>(L"id", L"0"),
        {make_shared<org::apache::lucene::document::NumericDocValuesField>(
            L"foo", -1)});
  });
  TestUtil::assertEquals(exc.what(),
                         L"cannot update docvalues field involved in the index "
                         L"sort, field=foo, sort=<long: \"foo\">");
  exc = expectThrows(invalid_argument::typeid, [&]() {
    w->updateNumericDocValue(make_shared<Term>(L"id", L"0"), L"foo", -1);
  });
  TestUtil::assertEquals(exc.what(),
                         L"cannot update docvalues field involved in the index "
                         L"sort, field=foo, sort=<long: \"foo\">");
  delete w;
  delete dir;
}

TestIndexSorting::DVUpdateRunnable::DVUpdateRunnable(
    int numDocs, shared_ptr<Random> random, shared_ptr<CountDownLatch> latch,
    shared_ptr<AtomicInteger> updateCount, shared_ptr<IndexWriter> w,
    unordered_map<int, int64_t> &values)
    : numDocs(numDocs), random(random), updateCount(updateCount), w(w),
      values(values), latch(latch)
{
}

void TestIndexSorting::DVUpdateRunnable::run()
{
  try {
    latch->await();
    while (updateCount->decrementAndGet() >= 0) {
      constexpr int id = random->nextInt(numDocs);
      constexpr int64_t value = random->nextInt(20);

      {
        lock_guard<mutex> lock(values);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        w->updateDocValues(make_shared<Term>(L"id", Integer::toString(id)),
                           {make_shared<NumericDocValuesField>(L"bar", value)});
        values.emplace(id, value);
      }

      switch (random->nextInt(10)) {
      case 0:
      case 1:
        // reopen
        DirectoryReader::open(w)->close();
        break;
      case 2:
        w->forceMerge(3);
        break;
      }
    }
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (IOException | InterruptedException e) {
    throw runtime_error(e);
  }
}

void TestIndexSorting::testConcurrentDVUpdates() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  unordered_map<int, int64_t> values = unordered_map<int, int64_t>();

  constexpr int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    doc->push_back(
        make_shared<NumericDocValuesField>(L"foo", random()->nextInt()));
    doc->push_back(make_shared<NumericDocValuesField>(L"bar", -1));
    w->addDocument(doc);
    values.emplace(i, -1LL);
  }
  std::deque<std::shared_ptr<Thread>> threads(2);
  shared_ptr<AtomicInteger> *const updateCount =
      make_shared<AtomicInteger>(atLeast(1000));
  shared_ptr<CountDownLatch> *const latch = make_shared<CountDownLatch>(1);
  for (int i = 0; i < threads.size(); ++i) {
    shared_ptr<Random> r = make_shared<Random>(random()->nextLong());
    threads[i] = make_shared<Thread>(make_shared<DVUpdateRunnable>(
        numDocs, r, latch, updateCount, w, values));
  }
  for (auto thread : threads) {
    thread->start();
  }
  latch->countDown();
  for (auto thread : threads) {
    thread->join();
  }
  w->forceMerge(1);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  for (int i = 0; i < numDocs; ++i) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<TopDocs> *const topDocs = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"id", Integer::toString(i))),
        1);
    TestUtil::assertEquals(1, topDocs->totalHits);
    shared_ptr<NumericDocValues> dvs =
        MultiDocValues::getNumericValues(reader, L"bar");
    int hitDoc = topDocs->scoreDocs[0]->doc;
    TestUtil::assertEquals(hitDoc, dvs->advance(hitDoc));
    TestUtil::assertEquals(values[i], dvs->longValue());
  }
  reader->close();
  delete w;
  delete dir;
}

void TestIndexSorting::testAddIndexes(bool withDeletes,
                                      bool useReaders) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Sort> indexSort =
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG));
  shared_ptr<IndexWriterConfig> iwc1 = newIndexWriterConfig();
  if (random()->nextBoolean()) {
    iwc1->setIndexSort(indexSort);
  }
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    doc->push_back(
        make_shared<NumericDocValuesField>(L"foo", random()->nextInt(20)));
    w->addDocument(doc);
  }
  if (withDeletes) {
    for (int i = random()->nextInt(5); i < numDocs;
         i += TestUtil::nextInt(random(), 1, 5)) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      w->deleteDocuments(make_shared<Term>(L"id", Integer::toString(i)));
    }
  }
  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const reader = w->getReader();
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc);

  if (useReaders) {
    std::deque<std::shared_ptr<CodecReader>> codecReaders(
        reader->leaves().size());
    for (int i = 0; i < codecReaders.size(); ++i) {
      codecReaders[i] =
          std::static_pointer_cast<CodecReader>(reader->leaves()[i]->reader());
    }
    w2->addIndexes(codecReaders);
  } else {
    w2->addIndexes({dir});
  }
  shared_ptr<IndexReader> *const reader2 = w2->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  shared_ptr<IndexSearcher> *const searcher2 = newSearcher(reader2);
  for (int i = 0; i < numDocs; ++i) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Query> query =
        make_shared<TermQuery>(make_shared<Term>(L"id", Integer::toString(i)));
    shared_ptr<TopDocs> *const topDocs = searcher->search(query, 1);
    shared_ptr<TopDocs> *const topDocs2 = searcher2->search(query, 1);
    TestUtil::assertEquals(topDocs->totalHits, topDocs2->totalHits);
    if (topDocs->totalHits == 1) {
      shared_ptr<NumericDocValues> dvs1 =
          MultiDocValues::getNumericValues(reader, L"foo");
      int hitDoc1 = topDocs->scoreDocs[0]->doc;
      TestUtil::assertEquals(hitDoc1, dvs1->advance(hitDoc1));
      int64_t value1 = dvs1->longValue();
      shared_ptr<NumericDocValues> dvs2 =
          MultiDocValues::getNumericValues(reader2, L"foo");
      int hitDoc2 = topDocs2->scoreDocs[0]->doc;
      TestUtil::assertEquals(hitDoc2, dvs2->advance(hitDoc2));
      int64_t value2 = dvs2->longValue();
      TestUtil::assertEquals(value1, value2);
    }
  }

  IOUtils::close({reader, reader2, w2, dir, dir2});
}

void TestIndexSorting::testAddIndexes() 
{
  testAddIndexes(false, true);
}

void TestIndexSorting::testAddIndexesWithDeletions() 
{
  testAddIndexes(true, true);
}

void TestIndexSorting::testAddIndexesWithDirectory() 
{
  testAddIndexes(false, false);
}

void TestIndexSorting::testAddIndexesWithDeletionsAndDirectory() throw(
    runtime_error)
{
  testAddIndexes(true, false);
}

void TestIndexSorting::testBadSort() 
{
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { iwc->setIndexSort(Sort::RELEVANCE); });
  TestUtil::assertEquals(L"invalid SortField type: must be one of [STRING, "
                         L"INT, FLOAT, LONG, DOUBLE] but got: <score>",
                         expected.what());
}

void TestIndexSorting::testIllegalChangeSort() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::LONG)));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  w->addDocument(make_shared<Document>());
  DirectoryReader::open(w)->close();
  w->addDocument(make_shared<Document>());
  w->forceMerge(1);
  delete w;

  shared_ptr<IndexWriterConfig> *const iwc2 =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc2->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"bar", SortField::Type::LONG)));
  invalid_argument e = expectThrows(
      invalid_argument::typeid, [&]() { make_shared<IndexWriter>(dir, iwc2); });
  wstring message = e.what();
  assertTrue(
      message.find(L"cannot change previous indexSort=<long: \"foo\">") !=
      wstring::npos);
  assertTrue(message.find(L"to new indexSort=<long: \"bar\">") !=
             wstring::npos);
  delete dir;
}

TestIndexSorting::NormsSimilarity::NormsSimilarity(shared_ptr<Similarity> in_)
    : in_(in_)
{
}

int64_t TestIndexSorting::NormsSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  if (state->getName() == L"norms") {
    return state->getLength();
  } else {
    return in_->computeNorm(state);
  }
}

shared_ptr<Similarity::SimWeight>
TestIndexSorting::NormsSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return in_->computeWeight(boost, collectionStats, {termStats});
}

shared_ptr<Similarity::SimScorer> TestIndexSorting::NormsSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return in_->simScorer(weight, context);
}

TestIndexSorting::PositionsTokenStream::PositionsTokenStream()
    : term(addAttribute(CharTermAttribute::typeid)),
      payload(addAttribute(PayloadAttribute::typeid)),
      offset(addAttribute(OffsetAttribute::typeid))
{
}

bool TestIndexSorting::PositionsTokenStream::incrementToken() 
{
  if (pos == 0) {
    return false;
  }

  clearAttributes();
  term->append(L"#all#");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  payload->setPayload(make_shared<BytesRef>(Integer::toString(pos)));
  offset->setOffset(off, off);
  --pos;
  ++off;
  return true;
}

void TestIndexSorting::PositionsTokenStream::setId(int id)
{
  pos = id / 10 + 1;
  off = 0;
}

void TestIndexSorting::testRandom2() 
{
  int numDocs = atLeast(100);

  shared_ptr<FieldType> POSITIONS_TYPE =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  POSITIONS_TYPE->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  POSITIONS_TYPE->freeze();

  shared_ptr<FieldType> TERM_VECTORS_TYPE =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  TERM_VECTORS_TYPE->setStoreTermVectors(true);
  TERM_VECTORS_TYPE->freeze();

  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  deque<std::shared_ptr<Document>> docs = deque<std::shared_ptr<Document>>();
  for (int i = 0; i < numDocs; i++) {
    int id = i * 10;
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(id),
                                            Field::Store::YES));
    doc->push_back(
        make_shared<StringField>(L"docs", L"#all#", Field::Store::NO));
    shared_ptr<PositionsTokenStream> positions =
        make_shared<PositionsTokenStream>();
    positions->setId(id);
    doc->push_back(make_shared<Field>(L"positions", positions, POSITIONS_TYPE));
    doc->push_back(make_shared<NumericDocValuesField>(L"numeric", id));
    wstring value = IntStream::range(0, id)
                        .mapToObj([&](any k) { Integer::toString(id); })
                        .collect(Collectors::joining(L" "));
    shared_ptr<TextField> norms =
        make_shared<TextField>(L"norms", value, Field::Store::NO);
    doc->push_back(norms);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"binary", make_shared<BytesRef>(Integer::toString(id))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedDocValuesField>(
        L"sorted", make_shared<BytesRef>(Integer::toString(id))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedSetDocValuesField>(
        L"multi_valued_string", make_shared<BytesRef>(Integer::toString(id))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedSetDocValuesField>(
        L"multi_valued_string",
        make_shared<BytesRef>(Integer::toString(id + 1))));
    doc->push_back(
        make_shared<SortedNumericDocValuesField>(L"multi_valued_numeric", id));
    doc->push_back(make_shared<SortedNumericDocValuesField>(
        L"multi_valued_numeric", id + 1));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<Field>(L"term_vectors", Integer::toString(id),
                                      TERM_VECTORS_TYPE));
    std::deque<char> bytes(4);
    NumericUtils::intToSortableBytes(id, bytes, 0);
    doc->push_back(make_shared<BinaryPoint>(L"points", bytes));
    docs.push_back(doc);
  }

  // Must use the same seed for both RandomIndexWriters so they behave
  // identically
  int64_t seed = random()->nextLong();

  // We add document alread in ID order for the first writer:
  shared_ptr<Directory> dir1 = newFSDirectory(createTempDir());

  shared_ptr<Random> random1 = make_shared<Random>(seed);
  shared_ptr<IndexWriterConfig> iwc1 = newIndexWriterConfig(random1, a);
  iwc1->setSimilarity(make_shared<NormsSimilarity>(
      iwc1->getSimilarity())); // for testing norms field
  // preserve docIDs
  iwc1->setMergePolicy(newLogMergePolicy());
  if (VERBOSE) {
    wcout << L"TEST: now index pre-sorted" << endl;
  }
  shared_ptr<RandomIndexWriter> w1 =
      make_shared<RandomIndexWriter>(random1, dir1, iwc1);
  for (auto doc : docs) {
    (std::static_pointer_cast<PositionsTokenStream>(
         (std::static_pointer_cast<Field>(doc->getField(L"positions")))
             ->tokenStreamValue()))
        ->setId(static_cast<Integer>(doc[L"id"]));
    w1->addDocument(doc);
  }

  // We shuffle documents, but set index sort, for the second writer:
  shared_ptr<Directory> dir2 = newFSDirectory(createTempDir());

  shared_ptr<Random> random2 = make_shared<Random>(seed);
  shared_ptr<IndexWriterConfig> iwc2 = newIndexWriterConfig(random2, a);
  iwc2->setSimilarity(make_shared<NormsSimilarity>(
      iwc2->getSimilarity())); // for testing norms field

  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"numeric", SortField::Type::INT));
  iwc2->setIndexSort(sort);

  Collections::shuffle(docs, random());
  if (VERBOSE) {
    wcout << L"TEST: now index with index-time sorting" << endl;
  }
  shared_ptr<RandomIndexWriter> w2 =
      make_shared<RandomIndexWriter>(random2, dir2, iwc2);
  int count = 0;
  int commitAtCount = TestUtil::nextInt(random(), 1, numDocs - 1);
  for (auto doc : docs) {
    (std::static_pointer_cast<PositionsTokenStream>(
         (std::static_pointer_cast<Field>(doc->getField(L"positions")))
             ->tokenStreamValue()))
        ->setId(static_cast<Integer>(doc[L"id"]));
    if (count++ == commitAtCount) {
      // Ensure forceMerge really does merge
      w2->commit();
    }
    w2->addDocument(doc);
  }
  if (VERBOSE) {
    wcout << L"TEST: now force merge" << endl;
  }
  w2->forceMerge(1);

  shared_ptr<DirectoryReader> r1 = w1->getReader();
  shared_ptr<DirectoryReader> r2 = w2->getReader();
  if (VERBOSE) {
    wcout << L"TEST: now compare r1=" << r1 << L" r2=" << r2 << endl;
  }
  TestUtil::assertEquals(sort, getOnlyLeafReader(r2)->getMetaData()->getSort());
  assertReaderEquals(L"left: sorted by hand; right: sorted by Lucene", r1, r2);
  IOUtils::close({w1, w2, r1, r2, dir1, dir2});
}

TestIndexSorting::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestIndexSorting> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexSorting::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestIndexSorting::RandomDoc::RandomDoc(int id)
    : id(id), intValue(LuceneTestCase::random()->nextInt()),
      intValues(std::deque<int>(numValues)),
      longValue(LuceneTestCase::random()->nextLong()),
      longValues(std::deque<int64_t>(numValues)),
      floatValue(LuceneTestCase::random()->nextFloat()),
      floatValues(std::deque<float>(numValues)),
      doubleValue(LuceneTestCase::random()->nextDouble()),
      doubleValues(std::deque<double>(numValues)),
      bytesValue(std::deque<char>(
          TestUtil::nextInt(LuceneTestCase::random(), 1, 50))),
      bytesValues(std::deque<std::deque<char>>(numValues))
{
  LuceneTestCase::random()->nextBytes(bytesValue);

  int numValues = LuceneTestCase::random()->nextInt(10);
  for (int i = 0; i < numValues; i++) {
    intValues[i] = LuceneTestCase::random()->nextInt();
    longValues[i] = LuceneTestCase::random()->nextLong();
    floatValues[i] = LuceneTestCase::random()->nextFloat();
    doubleValues[i] = LuceneTestCase::random()->nextDouble();
    bytesValues[i] =
        std::deque<char>(TestUtil::nextInt(LuceneTestCase::random(), 1, 50));
    LuceneTestCase::random()->nextBytes(bytesValue);
  }
}

shared_ptr<SortField> TestIndexSorting::randomIndexSortField()
{
  bool reversed = random()->nextBoolean();
  shared_ptr<SortField> sortField;
  switch (random()->nextInt(10)) {
  case 0:
    sortField = make_shared<SortField>(L"int", SortField::Type::INT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextInt());
    }
    break;
  case 1:
    sortField = make_shared<SortedNumericSortField>(
        L"multi_valued_int", SortField::Type::INT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextInt());
    }
    break;
  case 2:
    sortField =
        make_shared<SortField>(L"long", SortField::Type::LONG, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextLong());
    }
    break;
  case 3:
    sortField = make_shared<SortedNumericSortField>(
        L"multi_valued_long", SortField::Type::LONG, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextLong());
    }
    break;
  case 4:
    sortField =
        make_shared<SortField>(L"float", SortField::Type::FLOAT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextFloat());
    }
    break;
  case 5:
    sortField = make_shared<SortedNumericSortField>(
        L"multi_valued_float", SortField::Type::FLOAT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextFloat());
    }
    break;
  case 6:
    sortField =
        make_shared<SortField>(L"double", SortField::Type::DOUBLE, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextDouble());
    }
    break;
  case 7:
    sortField = make_shared<SortedNumericSortField>(
        L"multi_valued_double", SortField::Type::DOUBLE, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextDouble());
    }
    break;
  case 8:
    sortField =
        make_shared<SortField>(L"bytes", SortField::Type::STRING, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(SortField::STRING_LAST);
    }
    break;
  case 9:
    sortField =
        make_shared<SortedSetSortField>(L"multi_valued_bytes", reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(SortField::STRING_LAST);
    }
    break;
  default:
    sortField.reset();
    fail();
  }
  return sortField;
}

shared_ptr<Sort> TestIndexSorting::randomSort()
{
  // at least 2
  int numFields = TestUtil::nextInt(random(), 2, 4);
  std::deque<std::shared_ptr<SortField>> sortFields(numFields);
  for (int i = 0; i < numFields - 1; i++) {
    shared_ptr<SortField> sortField = randomIndexSortField();
    sortFields[i] = sortField;
  }

  // tie-break by id:
  sortFields[numFields - 1] =
      make_shared<SortField>(L"id", SortField::Type::INT);

  return make_shared<Sort>(sortFields);
}

void TestIndexSorting::testRandom3() 
{
  int numDocs = atLeast(1000);
  deque<std::shared_ptr<RandomDoc>> docs =
      deque<std::shared_ptr<RandomDoc>>();

  shared_ptr<Sort> sort = randomSort();
  if (VERBOSE) {
    wcout << L"TEST: numDocs=" << numDocs << L" use sort=" << sort << endl;
  }

  // no index sorting, all search-time sorting:
  shared_ptr<Directory> dir1 = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(dir1, iwc1);

  // use index sorting:
  shared_ptr<Directory> dir2 = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc2->setIndexSort(sort);
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc2);

  shared_ptr<Set<int>> toDelete = unordered_set<int>();

  double deleteChance = random()->nextDouble();

  for (int id = 0; id < numDocs; id++) {
    shared_ptr<RandomDoc> docValues = make_shared<RandomDoc>(id);
    docs.push_back(docValues);
    if (VERBOSE) {
      wcout << L"TEST: doc id=" << id << endl;
      wcout << L"  int=" << docValues->intValue << endl;
      wcout << L"  long=" << docValues->longValue << endl;
      wcout << L"  float=" << docValues->floatValue << endl;
      wcout << L"  double=" << docValues->doubleValue << endl;
      wcout << L"  bytes=" << make_shared<BytesRef>(docValues->bytesValue)
            << endl;
    }

    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(id),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    doc->push_back(
        make_shared<NumericDocValuesField>(L"int", docValues->intValue));
    doc->push_back(
        make_shared<NumericDocValuesField>(L"long", docValues->longValue));
    doc->push_back(
        make_shared<DoubleDocValuesField>(L"double", docValues->doubleValue));
    doc->push_back(
        make_shared<FloatDocValuesField>(L"float", docValues->floatValue));
    doc->push_back(make_shared<SortedDocValuesField>(
        L"bytes", make_shared<BytesRef>(docValues->bytesValue)));

    for (auto value : docValues->intValues) {
      doc->push_back(
          make_shared<SortedNumericDocValuesField>(L"multi_valued_int", value));
    }

    for (auto value : docValues->longValues) {
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"multi_valued_long", value));
    }

    for (auto value : docValues->floatValues) {
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"multi_valued_float", NumericUtils::floatToSortableInt(value)));
    }

    for (auto value : docValues->doubleValues) {
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"multi_valued_double", NumericUtils::doubleToSortableLong(value)));
    }

    for (auto value : docValues->bytesValues) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"multi_valued_bytes", make_shared<BytesRef>(value)));
    }

    w1->addDocument(doc);
    w2->addDocument(doc);
    if (random()->nextDouble() < deleteChance) {
      toDelete->add(id);
    }
  }
  for (auto id : toDelete) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    w1->deleteDocuments({make_shared<Term>(L"id", Integer::toString(id))});
    // C++ TODO: There is no native C++ equivalent to 'toString':
    w2->deleteDocuments({make_shared<Term>(L"id", Integer::toString(id))});
  }
  shared_ptr<DirectoryReader> r1 = DirectoryReader::open(w1);
  shared_ptr<IndexSearcher> s1 = newSearcher(r1);

  if (random()->nextBoolean()) {
    int maxSegmentCount = TestUtil::nextInt(random(), 1, 5);
    if (VERBOSE) {
      wcout << L"TEST: now forceMerge(" << maxSegmentCount << L")" << endl;
    }
    w2->forceMerge(maxSegmentCount);
  }

  shared_ptr<DirectoryReader> r2 = DirectoryReader::open(w2);
  shared_ptr<IndexSearcher> s2 = newSearcher(r2);

  /*
  System.out.println("TEST: full index:");
  SortedDocValues docValues = MultiDocValues.getSortedValues(r2, "bytes");
  for(int i=0;i<r2.maxDoc();i++) {
    System.out.println("  doc " + i + " id=" + r2.document(i).get("id") + "
  bytes=" + docValues.get(i));
  }
  */

  for (int iter = 0; iter < 100; iter++) {
    int numHits = TestUtil::nextInt(random(), 1, numDocs);
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << L" numHits=" << numHits << endl;
    }

    shared_ptr<TopFieldCollector> c1 =
        TopFieldCollector::create(sort, numHits, true, true, true, true);
    s1->search(make_shared<MatchAllDocsQuery>(), c1);
    shared_ptr<TopDocs> hits1 = c1->topDocs();

    shared_ptr<TopFieldCollector> c2 =
        TopFieldCollector::create(sort, numHits, true, true, true, false);
    s2->search(make_shared<MatchAllDocsQuery>(), c2);

    shared_ptr<TopDocs> hits2 = c2->topDocs();

    if (VERBOSE) {
      wcout << L"  topDocs query-time sort: totalHits=" << hits1->totalHits
            << endl;
      for (auto scoreDoc : hits1->scoreDocs) {
        wcout << L"    " << scoreDoc->doc << endl;
      }
      wcout << L"  topDocs index-time sort: totalHits=" << hits2->totalHits
            << endl;
      for (auto scoreDoc : hits2->scoreDocs) {
        wcout << L"    " << scoreDoc->doc << endl;
      }
    }

    assertTrue(hits2->totalHits <= hits1->totalHits);
    TestUtil::assertEquals(hits2->scoreDocs.size(), hits1->scoreDocs.size());
    for (int i = 0; i < hits2->scoreDocs.size(); i++) {
      shared_ptr<ScoreDoc> hit1 = hits1->scoreDocs[i];
      shared_ptr<ScoreDoc> hit2 = hits2->scoreDocs[i];
      TestUtil::assertEquals(r1->document(hit1->doc)->get(L"id"),
                             r2->document(hit2->doc)->get(L"id"));
      assertArrayEquals((std::static_pointer_cast<FieldDoc>(hit1))->fields,
                        (std::static_pointer_cast<FieldDoc>(hit2))->fields);
    }
  }

  IOUtils::close({r1, r2, w1, w2, dir1, dir2});
}

void TestIndexSorting::testTieBreak() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setIndexSort(make_shared<Sort>(
      make_shared<SortField>(L"foo", SortField::Type::STRING)));
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int id = 0; id < 1000; id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StoredField>(L"id", id));
    wstring value;
    if (id < 500) {
      value = L"bar2";
    } else {
      value = L"bar1";
    }
    doc->push_back(make_shared<SortedDocValuesField>(
        L"foo", make_shared<BytesRef>(value)));
    w->addDocument(doc);
    if (id == 500) {
      w->commit();
    }
  }
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  for (int docID = 0; docID < 1000; docID++) {
    int expectedID;
    if (docID < 500) {
      expectedID = 500 + docID;
    } else {
      expectedID = docID - 500;
    }
    TestUtil::assertEquals(
        expectedID,
        r->document(docID).getField(L"id").numericValue().intValue());
  }
  IOUtils::close({r, w, dir});
}

void TestIndexSorting::testIndexSortWithSparseField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"dense_int", SortField::Type::INT, true);
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Field> textField =
      newTextField(L"sparse_text", L"", Field::Store::NO);
  for (int i = 0; i < 128; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"dense_int", i));
    if (i < 64) {
      doc->push_back(make_shared<NumericDocValuesField>(L"sparse_int", i));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<BinaryDocValuesField>(
          L"sparse_binary", make_shared<BytesRef>(Integer::toString(i))));
      textField->setStringValue(L"foo");
      doc->push_back(textField);
    }
    w->addDocument(doc);
  }
  w->commit();
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(1, r->leaves()->size());
  shared_ptr<LeafReader> leafReader = r->leaves()->get(0).reader();

  shared_ptr<NumericDocValues> denseValues =
      leafReader->getNumericDocValues(L"dense_int");
  shared_ptr<NumericDocValues> sparseValues =
      leafReader->getNumericDocValues(L"sparse_int");
  shared_ptr<BinaryDocValues> sparseBinaryValues =
      leafReader->getBinaryDocValues(L"sparse_binary");
  shared_ptr<NumericDocValues> normsValues =
      leafReader->getNormValues(L"sparse_text");
  for (int docID = 0; docID < 128; docID++) {
    assertTrue(denseValues->advanceExact(docID));
    TestUtil::assertEquals(127 - docID,
                           static_cast<int>(denseValues->longValue()));
    if (docID >= 64) {
      assertTrue(denseValues->advanceExact(docID));
      assertTrue(sparseValues->advanceExact(docID));
      assertTrue(sparseBinaryValues->advanceExact(docID));
      assertTrue(normsValues->advanceExact(docID));
      TestUtil::assertEquals(1, normsValues->longValue());
      TestUtil::assertEquals(127 - docID,
                             static_cast<int>(sparseValues->longValue()));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(
          make_shared<BytesRef>(Integer::toString(127 - docID)),
          sparseBinaryValues->binaryValue());
    } else {
      assertFalse(sparseBinaryValues->advanceExact(docID));
      assertFalse(sparseValues->advanceExact(docID));
      assertFalse(normsValues->advanceExact(docID));
    }
  }
  IOUtils::close({r, w, dir});
}

void TestIndexSorting::testIndexSortOnSparseField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"sparse", SortField::Type::INT, false);
  sortField->setMissingValue(numeric_limits<int>::min());
  shared_ptr<Sort> indexSort = make_shared<Sort>(sortField);
  iwc->setIndexSort(indexSort);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 128; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (i < 64) {
      doc->push_back(make_shared<NumericDocValuesField>(L"sparse", i));
    }
    w->addDocument(doc);
  }
  w->commit();
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(1, r->leaves()->size());
  shared_ptr<LeafReader> leafReader = r->leaves()->get(0).reader();
  shared_ptr<NumericDocValues> sparseValues =
      leafReader->getNumericDocValues(L"sparse");
  for (int docID = 0; docID < 128; docID++) {
    if (docID >= 64) {
      assertTrue(sparseValues->advanceExact(docID));
      TestUtil::assertEquals(docID - 64,
                             static_cast<int>(sparseValues->longValue()));
    } else {
      assertFalse(sparseValues->advanceExact(docID));
    }
  }
  IOUtils::close({r, w, dir});
}
} // namespace org::apache::lucene::index