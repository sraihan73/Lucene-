using namespace std;

#include "IndexAndTaxonomyRevisionTest.h"

namespace org::apache::lucene::replicator
{
using Document = org::apache::lucene::document::Document;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using SnapshotDirectoryTaxonomyWriter = org::apache::lucene::replicator::
    IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Test;

shared_ptr<Document> IndexAndTaxonomyRevisionTest::newDocument(
    shared_ptr<TaxonomyWriter> taxoWriter) 
{
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"A", L"1"));
  return config->build(taxoWriter, doc);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoCommit() throws Exception
void IndexAndTaxonomyRevisionTest::testNoCommit() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> indexWriter =
      make_shared<IndexWriter>(indexDir, conf);

  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<SnapshotDirectoryTaxonomyWriter> taxoWriter =
      make_shared<SnapshotDirectoryTaxonomyWriter>(taxoDir);
  // should fail when there are no commits to snapshot
  expectThrows(IllegalStateException::typeid, [&]() {
    make_shared<IndexAndTaxonomyRevision>(indexWriter, taxoWriter);
  });

  delete indexWriter;
  IOUtils::close({taxoWriter, taxoDir, indexDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRevisionRelease() throws Exception
void IndexAndTaxonomyRevisionTest::testRevisionRelease() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> indexWriter =
      make_shared<IndexWriter>(indexDir, conf);

  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<SnapshotDirectoryTaxonomyWriter> taxoWriter =
      make_shared<SnapshotDirectoryTaxonomyWriter>(taxoDir);
  try {
    indexWriter->addDocument(newDocument(taxoWriter));
    indexWriter->commit();
    taxoWriter->commit();
    shared_ptr<Revision> rev1 =
        make_shared<IndexAndTaxonomyRevision>(indexWriter, taxoWriter);
    // releasing that revision should not delete the files
    rev1->release();
    assertTrue(slowFileExists(indexDir, IndexFileNames::SEGMENTS + L"_1"));
    assertTrue(slowFileExists(taxoDir, IndexFileNames::SEGMENTS + L"_1"));

    rev1 = make_shared<IndexAndTaxonomyRevision>(
        indexWriter,
        taxoWriter); // create revision again, so the files are snapshotted
    indexWriter->addDocument(newDocument(taxoWriter));
    indexWriter->commit();
    taxoWriter->commit();
    assertNotNull(
        make_shared<IndexAndTaxonomyRevision>(indexWriter, taxoWriter));
    rev1->release(); // this release should trigger the delete of segments_1
    assertFalse(slowFileExists(indexDir, IndexFileNames::SEGMENTS + L"_1"));
    delete indexWriter;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({indexWriter, taxoWriter, taxoDir, indexDir});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSegmentsFileLast() throws Exception
void IndexAndTaxonomyRevisionTest::testSegmentsFileLast() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> indexWriter =
      make_shared<IndexWriter>(indexDir, conf);

  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<SnapshotDirectoryTaxonomyWriter> taxoWriter =
      make_shared<SnapshotDirectoryTaxonomyWriter>(taxoDir);
  try {
    indexWriter->addDocument(newDocument(taxoWriter));
    indexWriter->commit();
    taxoWriter->commit();
    shared_ptr<Revision> rev =
        make_shared<IndexAndTaxonomyRevision>(indexWriter, taxoWriter);
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>> sourceFiles =
        rev->getSourceFiles();
    assertEquals(2, sourceFiles.size());
    for (auto files : sourceFiles) {
      wstring lastFile = files.second->get(files.second->size() - 1).fileName;
      assertTrue(StringHelper::startsWith(lastFile, IndexFileNames::SEGMENTS));
    }
    delete indexWriter;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({indexWriter, taxoWriter, taxoDir, indexDir});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpen() throws Exception
void IndexAndTaxonomyRevisionTest::testOpen() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  shared_ptr<IndexWriter> indexWriter =
      make_shared<IndexWriter>(indexDir, conf);

  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<SnapshotDirectoryTaxonomyWriter> taxoWriter =
      make_shared<SnapshotDirectoryTaxonomyWriter>(taxoDir);
  try {
    indexWriter->addDocument(newDocument(taxoWriter));
    indexWriter->commit();
    taxoWriter->commit();
    shared_ptr<Revision> rev =
        make_shared<IndexAndTaxonomyRevision>(indexWriter, taxoWriter);
    for (auto e : rev->getSourceFiles()) {
      wstring source = e.first;
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("resource")
      // org.apache.lucene.store.Directory dir =
      // source.equals(IndexAndTaxonomyRevision.INDEX_SOURCE) ? indexDir :
      // taxoDir;
      shared_ptr<Directory> dir =
          source == IndexAndTaxonomyRevision::INDEX_SOURCE ? indexDir : taxoDir;
      for (shared_ptr<RevisionFile> file : e.second) {
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
    }
    delete indexWriter;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({indexWriter, taxoWriter, taxoDir, indexDir});
  }
}
} // namespace org::apache::lucene::replicator