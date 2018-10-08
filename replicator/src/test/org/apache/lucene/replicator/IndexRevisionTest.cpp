using namespace std;

#include "IndexRevisionTest.h"

namespace org::apache::lucene::replicator
{
using Document = org::apache::lucene::document::Document;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using KeepOnlyLastCommitDeletionPolicy =
    org::apache::lucene::index::KeepOnlyLastCommitDeletionPolicy;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoSnapshotDeletionPolicy() throws
// Exception
void IndexRevisionTest::testNoSnapshotDeletionPolicy() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(make_shared<KeepOnlyLastCommitDeletionPolicy>());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  // should fail when IndexDeletionPolicy is not Snapshot
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<IndexRevision>(writer); });

  delete writer;
  IOUtils::close({dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoCommit() throws Exception
void IndexRevisionTest::testNoCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  // should fail when there are no commits to snapshot"
  expectThrows(IllegalStateException::typeid,
               [&]() { make_shared<IndexRevision>(writer); });

  delete writer;
  IOUtils::close({dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRevisionRelease() throws Exception
void IndexRevisionTest::testRevisionRelease() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  try {
    writer->addDocument(make_shared<Document>());
    writer->commit();
    shared_ptr<Revision> rev1 = make_shared<IndexRevision>(writer);
    // releasing that revision should not delete the files
    rev1->release();
    assertTrue(slowFileExists(dir, IndexFileNames::SEGMENTS + L"_1"));

    rev1 = make_shared<IndexRevision>(
        writer); // create revision again, so the files are snapshotted
    writer->addDocument(make_shared<Document>());
    writer->commit();
    assertNotNull(make_shared<IndexRevision>(writer));
    rev1->release(); // this release should trigger the delete of segments_1
    assertFalse(slowFileExists(dir, IndexFileNames::SEGMENTS + L"_1"));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({writer, dir});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSegmentsFileLast() throws Exception
void IndexRevisionTest::testSegmentsFileLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  try {
    writer->addDocument(make_shared<Document>());
    writer->commit();
    shared_ptr<Revision> rev = make_shared<IndexRevision>(writer);
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") java.util.Map<std::wstring,
    // java.util.List<RevisionFile>> sourceFiles = rev.getSourceFiles();
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>> sourceFiles =
        rev->getSourceFiles();
    assertEquals(1, sourceFiles.size());
    deque<std::shared_ptr<RevisionFile>> files =
        sourceFiles.values().begin()->next();
    wstring lastFile = files[files.size() - 1]->fileName;
    assertTrue(StringHelper::startsWith(lastFile, IndexFileNames::SEGMENTS));
    delete writer;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({dir});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpen() throws Exception
void IndexRevisionTest::testOpen() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  try {
    writer->addDocument(make_shared<Document>());
    writer->commit();
    shared_ptr<Revision> rev = make_shared<IndexRevision>(writer);
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") java.util.Map<std::wstring,
    // java.util.List<RevisionFile>> sourceFiles = rev.getSourceFiles();
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>> sourceFiles =
        rev->getSourceFiles();
    wstring source = sourceFiles.keySet().begin()->next();
    for (shared_ptr<RevisionFile> file : sourceFiles.values().begin()->next()) {
      shared_ptr<IndexInput> src =
          dir->openInput(file->fileName, IOContext::READONCE);
      shared_ptr<InputStream> in_ = rev->open(source, file->fileName);
      assertEquals(src->length(), in_->available());
      std::deque<char> srcBytes(static_cast<int>(src->length()));
      std::deque<char> inBytes(static_cast<int>(src->length()));
      int offset = 0;
      if (random()->nextBoolean()) {
        int skip = random()->nextInt(10);
        if (skip >= src->length()) {
          skip = 0;
        }
        in_->skip(skip);
        src->seek(skip);
        offset = skip;
      }
      src->readBytes(srcBytes, offset, srcBytes.size() - offset);
      in_->read(inBytes, offset, inBytes.size() - offset);
      assertArrayEquals(srcBytes, inBytes);
      IOUtils::close({src, in_});
    }
    delete writer;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({dir});
  }
}
} // namespace org::apache::lucene::replicator