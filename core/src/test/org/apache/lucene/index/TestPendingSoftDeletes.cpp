using namespace std;

#include "TestPendingSoftDeletes.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;

shared_ptr<PendingSoftDeletes> TestPendingSoftDeletes::newPendingDeletes(
    shared_ptr<SegmentCommitInfo> commitInfo)
{
  return make_shared<PendingSoftDeletes>(L"_soft_deletes", commitInfo);
}

void TestPendingSoftDeletes::testHardDeleteSoftDeleted() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig()
               ->setSoftDeletesField(L"_soft_deletes")
               ->setMaxBufferedDocs(10)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  writer->commit();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->leaves()->size());
  shared_ptr<SegmentReader> segmentReader =
      std::static_pointer_cast<SegmentReader>(
          reader->leaves()->get(0).reader());
  shared_ptr<SegmentCommitInfo> segmentInfo = segmentReader->getSegmentInfo();
  shared_ptr<PendingSoftDeletes> pendingSoftDeletes =
      newPendingDeletes(segmentInfo);
  pendingSoftDeletes->onNewReader(segmentReader, segmentInfo);
  assertEquals(0, pendingSoftDeletes->numPendingDeletes());
  assertEquals(1, pendingSoftDeletes->getDelCount());
  assertTrue(pendingSoftDeletes->getLiveDocs()->get(0));
  assertFalse(pendingSoftDeletes->getLiveDocs()->get(1));
  assertTrue(pendingSoftDeletes->getLiveDocs()->get(2));
  assertNull(pendingSoftDeletes->getHardLiveDocs());
  assertTrue(pendingSoftDeletes->delete (1));
  assertEquals(0, pendingSoftDeletes->numPendingDeletes());
  assertEquals(
      -1, pendingSoftDeletes->pendingDeleteCount); // transferred the delete
  assertEquals(1, pendingSoftDeletes->getDelCount());
  IOUtils::close({reader, writer, dir});
}

void TestPendingSoftDeletes::testDeleteSoft() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig()
               ->setSoftDeletesField(L"_soft_deletes")
               ->setMaxBufferedDocs(10)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  writer->commit();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->leaves()->size());
  shared_ptr<SegmentReader> segmentReader =
      std::static_pointer_cast<SegmentReader>(
          reader->leaves()->get(0).reader());
  shared_ptr<SegmentCommitInfo> segmentInfo = segmentReader->getSegmentInfo();
  shared_ptr<PendingSoftDeletes> pendingSoftDeletes =
      newPendingDeletes(segmentInfo);
  pendingSoftDeletes->onNewReader(segmentReader, segmentInfo);
  assertEquals(0, pendingSoftDeletes->numPendingDeletes());
  assertEquals(1, pendingSoftDeletes->getDelCount());
  assertTrue(pendingSoftDeletes->getLiveDocs()->get(0));
  assertFalse(pendingSoftDeletes->getLiveDocs()->get(1));
  assertTrue(pendingSoftDeletes->getLiveDocs()->get(2));
  assertNull(pendingSoftDeletes->getHardLiveDocs());
  // pass reader again
  shared_ptr<Bits> liveDocs = pendingSoftDeletes->getLiveDocs();
  pendingSoftDeletes->onNewReader(segmentReader, segmentInfo);
  assertEquals(0, pendingSoftDeletes->numPendingDeletes());
  assertEquals(1, pendingSoftDeletes->getDelCount());
  assertSame(liveDocs, pendingSoftDeletes->getLiveDocs());

  // now apply a hard delete
  writer->deleteDocuments({make_shared<Term>(L"id", L"1")});
  writer->commit();
  IOUtils::close({reader});
  reader = DirectoryReader::open(dir);
  assertEquals(1, reader->leaves()->size());
  segmentReader = std::static_pointer_cast<SegmentReader>(
      reader->leaves()->get(0).reader());
  segmentInfo = segmentReader->getSegmentInfo();
  pendingSoftDeletes = newPendingDeletes(segmentInfo);
  pendingSoftDeletes->onNewReader(segmentReader, segmentInfo);
  assertEquals(0, pendingSoftDeletes->numPendingDeletes());
  assertEquals(2, pendingSoftDeletes->getDelCount());
  assertFalse(pendingSoftDeletes->getLiveDocs()->get(0));
  assertFalse(pendingSoftDeletes->getLiveDocs()->get(1));
  assertTrue(pendingSoftDeletes->getLiveDocs()->get(2));
  assertNotNull(pendingSoftDeletes->getHardLiveDocs());
  assertFalse(pendingSoftDeletes->getHardLiveDocs()->get(0));
  assertTrue(pendingSoftDeletes->getHardLiveDocs()->get(1));
  assertTrue(pendingSoftDeletes->getHardLiveDocs()->get(2));
  IOUtils::close({reader, writer, dir});
}

void TestPendingSoftDeletes::testApplyUpdates() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<SegmentInfo> si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"test", 10, false,
      Codec::getDefault(), Collections::emptyMap(), StringHelper::randomId(),
      unordered_map<>(), nullptr);
  shared_ptr<SegmentCommitInfo> commitInfo =
      make_shared<SegmentCommitInfo>(si, 0, 0, -1, -1, -1);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  for (int i = 0; i < si->maxDoc(); i++) {
    writer->addDocument(make_shared<Document>());
  }
  writer->forceMerge(1);
  writer->commit();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->leaves()->size());
  shared_ptr<SegmentReader> segmentReader =
      std::static_pointer_cast<SegmentReader>(
          reader->leaves()->get(0).reader());
  shared_ptr<PendingSoftDeletes> deletes = newPendingDeletes(commitInfo);
  deletes->onNewReader(segmentReader, commitInfo);
  reader->close();
  delete writer;
  shared_ptr<FieldInfo> fieldInfo = make_shared<FieldInfo>(
      L"_soft_deletes", 1, false, false, false, IndexOptions::NONE,
      DocValuesType::NUMERIC, 0, Collections::emptyMap(), 0, 0, true);
  deque<int> docsDeleted =
      Arrays::asList(1, 3, 7, 8, DocIdSetIterator::NO_MORE_DOCS);
  deque<std::shared_ptr<DocValuesFieldUpdates>> updates =
      Arrays::asList(singleUpdate(docsDeleted, 10, true));
  for (auto update : updates) {
    deletes->onDocValuesUpdate(fieldInfo, update->begin());
  }
  assertEquals(0, deletes->numPendingDeletes());
  assertEquals(4, deletes->getDelCount());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertFalse(deletes->getLiveDocs()->get(1));
  assertTrue(deletes->getLiveDocs()->get(2));
  assertFalse(deletes->getLiveDocs()->get(3));
  assertTrue(deletes->getLiveDocs()->get(4));
  assertTrue(deletes->getLiveDocs()->get(5));
  assertTrue(deletes->getLiveDocs()->get(6));
  assertFalse(deletes->getLiveDocs()->get(7));
  assertFalse(deletes->getLiveDocs()->get(8));
  assertTrue(deletes->getLiveDocs()->get(9));

  docsDeleted = Arrays::asList(1, 2, DocIdSetIterator::NO_MORE_DOCS);
  updates = Arrays::asList(singleUpdate(docsDeleted, 10, true));
  fieldInfo = make_shared<FieldInfo>(L"_soft_deletes", 1, false, false, false,
                                     IndexOptions::NONE, DocValuesType::NUMERIC,
                                     1, Collections::emptyMap(), 0, 0, true);
  for (auto update : updates) {
    deletes->onDocValuesUpdate(fieldInfo, update->begin());
  }
  assertEquals(0, deletes->numPendingDeletes());
  assertEquals(5, deletes->getDelCount());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertFalse(deletes->getLiveDocs()->get(1));
  assertFalse(deletes->getLiveDocs()->get(2));
  assertFalse(deletes->getLiveDocs()->get(3));
  assertTrue(deletes->getLiveDocs()->get(4));
  assertTrue(deletes->getLiveDocs()->get(5));
  assertTrue(deletes->getLiveDocs()->get(6));
  assertFalse(deletes->getLiveDocs()->get(7));
  assertFalse(deletes->getLiveDocs()->get(8));
  assertTrue(deletes->getLiveDocs()->get(9));
}

void TestPendingSoftDeletes::testUpdateAppliedOnlyOnce() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig()
               ->setSoftDeletesField(L"_soft_deletes")
               ->setMaxBufferedDocs(3)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  writer->commit();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->leaves()->size());
  shared_ptr<SegmentReader> segmentReader =
      std::static_pointer_cast<SegmentReader>(
          reader->leaves()->get(0).reader());
  shared_ptr<SegmentCommitInfo> segmentInfo = segmentReader->getSegmentInfo();
  shared_ptr<PendingDeletes> deletes = newPendingDeletes(segmentInfo);
  deletes->onNewReader(segmentReader, segmentInfo);
  shared_ptr<FieldInfo> fieldInfo = make_shared<FieldInfo>(
      L"_soft_deletes", 1, false, false, false, IndexOptions::NONE,
      DocValuesType::NUMERIC, segmentInfo->getNextDocValuesGen(),
      Collections::emptyMap(), 0, 0, true);
  deque<int> docsDeleted = Arrays::asList(1, DocIdSetIterator::NO_MORE_DOCS);
  deque<std::shared_ptr<DocValuesFieldUpdates>> updates =
      Arrays::asList(singleUpdate(docsDeleted, 3, true));
  for (auto update : updates) {
    deletes->onDocValuesUpdate(fieldInfo, update->begin());
  }
  assertEquals(0, deletes->numPendingDeletes());
  assertEquals(1, deletes->getDelCount());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertFalse(deletes->getLiveDocs()->get(1));
  assertTrue(deletes->getLiveDocs()->get(2));
  shared_ptr<Bits> liveDocs = deletes->getLiveDocs();
  deletes->onNewReader(segmentReader, segmentInfo);
  // no changes we don't apply updates twice
  assertSame(liveDocs, deletes->getLiveDocs());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertFalse(deletes->getLiveDocs()->get(1));
  assertTrue(deletes->getLiveDocs()->get(2));
  assertEquals(0, deletes->numPendingDeletes());
  assertEquals(1, deletes->getDelCount());
  IOUtils::close({reader, writer, dir});
}

void TestPendingSoftDeletes::testResetOnUpdate() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig()
               ->setSoftDeletesField(L"_soft_deletes")
               ->setMaxBufferedDocs(3)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"2"), doc,
      {make_shared<NumericDocValuesField>(L"_soft_deletes", 1)});
  writer->commit();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->leaves()->size());
  shared_ptr<SegmentReader> segmentReader =
      std::static_pointer_cast<SegmentReader>(
          reader->leaves()->get(0).reader());
  shared_ptr<SegmentCommitInfo> segmentInfo = segmentReader->getSegmentInfo();
  shared_ptr<PendingDeletes> deletes = newPendingDeletes(segmentInfo);
  deletes->onNewReader(segmentReader, segmentInfo);
  shared_ptr<FieldInfo> fieldInfo = make_shared<FieldInfo>(
      L"_soft_deletes", 1, false, false, false, IndexOptions::NONE,
      DocValuesType::NUMERIC, segmentInfo->getNextDocValuesGen(),
      Collections::emptyMap(), 0, 0, true);
  deque<std::shared_ptr<DocValuesFieldUpdates>> updates =
      Arrays::asList(singleUpdate(
          Arrays::asList(0, 1, DocIdSetIterator::NO_MORE_DOCS), 3, false));
  for (auto update : updates) {
    deletes->onDocValuesUpdate(fieldInfo, update->begin());
  }
  assertEquals(0, deletes->numPendingDeletes());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertTrue(deletes->getLiveDocs()->get(1));
  assertTrue(deletes->getLiveDocs()->get(2));
  shared_ptr<Bits> liveDocs = deletes->getLiveDocs();
  deletes->onNewReader(segmentReader, segmentInfo);
  // no changes we keep this update
  assertSame(liveDocs, deletes->getLiveDocs());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertTrue(deletes->getLiveDocs()->get(1));
  assertTrue(deletes->getLiveDocs()->get(2));
  assertEquals(0, deletes->numPendingDeletes());

  segmentInfo->advanceDocValuesGen();
  fieldInfo = make_shared<FieldInfo>(L"_soft_deletes", 1, false, false, false,
                                     IndexOptions::NONE, DocValuesType::NUMERIC,
                                     segmentInfo->getNextDocValuesGen(),
                                     Collections::emptyMap(), 0, 0, true);
  updates = Arrays::asList(
      singleUpdate(Arrays::asList(1, DocIdSetIterator::NO_MORE_DOCS), 3, true));
  for (auto update : updates) {
    deletes->onDocValuesUpdate(fieldInfo, update->begin());
  }
  // no changes we keep this update
  assertNotSame(liveDocs, deletes->getLiveDocs());
  assertTrue(deletes->getLiveDocs()->get(0));
  assertFalse(deletes->getLiveDocs()->get(1));
  assertTrue(deletes->getLiveDocs()->get(2));
  assertEquals(0, deletes->numPendingDeletes());
  assertEquals(1, deletes->getDelCount());
  IOUtils::close({reader, writer, dir});
}

shared_ptr<DocValuesFieldUpdates>
TestPendingSoftDeletes::singleUpdate(deque<int> &docsChanged, int maxDoc,
                                     bool hasValue)
{
  return make_shared<DocValuesFieldUpdatesAnonymousInnerClass>(
      shared_from_this(), maxDoc, docsChanged, hasValue);
}

TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    DocValuesFieldUpdatesAnonymousInnerClass(
        shared_ptr<TestPendingSoftDeletes> outerInstance, int maxDoc,
        deque<int> &docsChanged, bool hasValue)
    : DocValuesFieldUpdates(maxDoc, 0, L"_soft_deletes", DocValuesType::NUMERIC)
{
  this->outerInstance = outerInstance;
  this->docsChanged = docsChanged;
  this->hasValue = hasValue;
}

void TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::add(
    int doc, int64_t value)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::add(
    int doc, shared_ptr<BytesRef> value)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::add(
    int docId, shared_ptr<Iterator> iterator)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Iterator>
TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<DocValuesFieldUpdatesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  iter = outerInstance->docsChanged.begin();
  doc = -1;
}

int TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::nextDoc()
{
  return doc = iter->next();
}

int64_t TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::longValue()
{
  return 1;
}

shared_ptr<BytesRef>
TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::binaryValue()
{
  throw make_shared<UnsupportedOperationException>();
}

int TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::docID()
{
  return doc;
}

int64_t TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::delGen()
{
  return 0;
}

bool TestPendingSoftDeletes::DocValuesFieldUpdatesAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasValue()
{
  return outerInstance->hasValue;
}
} // namespace org::apache::lucene::index