using namespace std;

#include "TestSizeBoundedForceMerge.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSizeBoundedForceMerge::addDocs(shared_ptr<IndexWriter> writer,
                                        int numDocs) 
{
  addDocs(writer, numDocs, false);
}

void TestSizeBoundedForceMerge::addDocs(shared_ptr<IndexWriter> writer,
                                        int numDocs,
                                        bool withID) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (withID) {
      doc->push_back(make_shared<StringField>(L"id", L"" + to_wstring(i),
                                              Field::Store::NO));
    }
    writer->addDocument(doc);
  }
  writer->commit();
}

shared_ptr<IndexWriterConfig> TestSizeBoundedForceMerge::newWriterConfig()
{
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  conf->setRAMBufferSizeMB(IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);
  // prevent any merges by default.
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  return conf;
}

void TestSizeBoundedForceMerge::testByteSizeLimit() 
{
  // tests that the max merge size constraint is applied during forceMerge.
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  // Prepare an index w/ several small segments and a large one.
  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  constexpr int numSegments = 15;
  for (int i = 0; i < numSegments; i++) {
    int numDocs = i == 7 ? 30 : 1;
    addDocs(writer, numDocs);
  }
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  double min = sis->info(0)->sizeInBytes();

  conf = newWriterConfig();
  shared_ptr<LogByteSizeMergePolicy> lmp =
      make_shared<LogByteSizeMergePolicy>();
  lmp->setMaxMergeMBForForcedMerge((min + 1) / (1 << 20));
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  // Should only be 3 segments in the index, because one of them exceeds the
  // size limit
  sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(3, sis->size());
}

void TestSizeBoundedForceMerge::testNumDocsLimit() 
{
  // tests that the max merge docs constraint is applied during forceMerge.
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  // Prepare an index w/ several small segments and a large one.
  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 5);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  // Should only be 3 segments in the index, because one of them exceeds the
  // size limit
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(3, sis->size());
}

void TestSizeBoundedForceMerge::testLastSegmentTooLarge() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 5);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(2, sis->size());
}

void TestSizeBoundedForceMerge::testFirstSegmentTooLarge() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 5);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(2, sis->size());
}

void TestSizeBoundedForceMerge::testAllSegmentsSmall() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(1, sis->size());
}

void TestSizeBoundedForceMerge::testAllSegmentsLarge() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(2);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(3, sis->size());
}

void TestSizeBoundedForceMerge::testOneLargeOneSmall() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 5);
  addDocs(writer, 3);
  addDocs(writer, 5);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(4, sis->size());
}

void TestSizeBoundedForceMerge::testMergeFactor() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 3);
  addDocs(writer, 5);
  addDocs(writer, 3);
  addDocs(writer, 3);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  lmp->setMergeFactor(2);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  // Should only be 4 segments in the index, because of the merge factor and
  // max merge docs settings.
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(4, sis->size());
}

void TestSizeBoundedForceMerge::testSingleMergeableSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3);
  addDocs(writer, 5);
  addDocs(writer, 3);

  // delete the last document, so that the last segment is merged.
  writer->deleteDocuments({make_shared<Term>(L"id", L"10")});
  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  // Verify that the last segment does not have deletions.
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(3, sis->size());
  assertFalse(sis->info(2)->hasDeletions());
}

void TestSizeBoundedForceMerge::testSingleNonMergeableSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 3, true);

  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(3);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  // Verify that the last segment does not have deletions.
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(1, sis->size());
}

void TestSizeBoundedForceMerge::testSingleMergeableTooLargeSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();

  shared_ptr<IndexWriterConfig> conf = newWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  addDocs(writer, 5, true);

  // delete the last document

  writer->deleteDocuments({make_shared<Term>(L"id", L"4")});
  delete writer;

  conf = newWriterConfig();
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMaxMergeDocs(2);
  conf->setMergePolicy(lmp);

  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  // Verify that the last segment does not have deletions.
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(1, sis->size());
  assertTrue(sis->info(0)->hasDeletions());
}
} // namespace org::apache::lucene::index