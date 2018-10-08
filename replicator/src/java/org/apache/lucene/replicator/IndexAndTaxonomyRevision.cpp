using namespace std;

#include "IndexAndTaxonomyRevision.h"

namespace org::apache::lucene::replicator
{
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexDeletionPolicy = org::apache::lucene::index::IndexDeletionPolicy;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::
    SnapshotDirectoryTaxonomyWriter(
        shared_ptr<Directory> directory, IndexWriterConfig::OpenMode openMode,
        shared_ptr<TaxonomyWriterCache> cache) 
    : org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter(
          directory, openMode, cache)
{
}

IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::
    SnapshotDirectoryTaxonomyWriter(
        shared_ptr<Directory> directory,
        IndexWriterConfig::OpenMode openMode) 
    : org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter(
          directory, openMode)
{
}

IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::
    SnapshotDirectoryTaxonomyWriter(shared_ptr<Directory> d) 
    : org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter(
          d)
{
}

shared_ptr<IndexWriterConfig>
IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::
    createIndexWriterConfig(IndexWriterConfig::OpenMode openMode)
{
  shared_ptr<IndexWriterConfig> conf =
      DirectoryTaxonomyWriter::createIndexWriterConfig(openMode);
  sdp = make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy());
  conf->setIndexDeletionPolicy(sdp);
  return conf;
}

shared_ptr<IndexWriter>
IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::openIndexWriter(
    shared_ptr<Directory> directory,
    shared_ptr<IndexWriterConfig> config) 
{
  writer = DirectoryTaxonomyWriter::openIndexWriter(directory, config);
  return writer;
}

shared_ptr<SnapshotDeletionPolicy>
IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::getDeletionPolicy()
{
  return sdp;
}

shared_ptr<IndexWriter>
IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter::getIndexWriter()
{
  return writer;
}

const wstring IndexAndTaxonomyRevision::INDEX_SOURCE = L"index";
const wstring IndexAndTaxonomyRevision::TAXONOMY_SOURCE = L"taxo";

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
IndexAndTaxonomyRevision::revisionFiles(
    shared_ptr<IndexCommit> indexCommit,
    shared_ptr<IndexCommit> taxoCommit) 
{
  unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>> files =
      unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>();
  files.emplace(
      INDEX_SOURCE,
      IndexRevision::revisionFiles(indexCommit).values().begin()->next());
  files.emplace(
      TAXONOMY_SOURCE,
      IndexRevision::revisionFiles(taxoCommit).values().begin()->next());
  return files;
}

wstring
IndexAndTaxonomyRevision::revisionVersion(shared_ptr<IndexCommit> indexCommit,
                                          shared_ptr<IndexCommit> taxoCommit)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(indexCommit->getGeneration(), RADIX) + L":" +
         Long::toString(taxoCommit->getGeneration(), RADIX);
}

IndexAndTaxonomyRevision::IndexAndTaxonomyRevision(
    shared_ptr<IndexWriter> indexWriter,
    shared_ptr<SnapshotDirectoryTaxonomyWriter> taxoWriter) 
    : indexWriter(indexWriter), taxoWriter(taxoWriter),
      indexCommit(indexSDP->snapshot()), taxoCommit(taxoSDP->snapshot()),
      indexSDP(std::static_pointer_cast<SnapshotDeletionPolicy>(delPolicy)),
      taxoSDP(taxoWriter->getDeletionPolicy()),
      version(revisionVersion(indexCommit, taxoCommit)),
      sourceFiles(revisionFiles(indexCommit, taxoCommit))
{
  shared_ptr<IndexDeletionPolicy> delPolicy =
      indexWriter->getConfig()->getIndexDeletionPolicy();
  if (!(std::dynamic_pointer_cast<SnapshotDeletionPolicy>(delPolicy) !=
        nullptr)) {
    throw invalid_argument(
        L"IndexWriter must be created with SnapshotDeletionPolicy");
  }
}

int IndexAndTaxonomyRevision::compareTo(const wstring &version)
{
  const std::deque<wstring> parts = version.split(L":");
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: final long indexGen = Long.parseLong(parts[0], RADIX);
  constexpr int64_t indexGen = int64_t ::valueOf(parts[0], RADIX);
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: final long taxoGen = Long.parseLong(parts[1], RADIX);
  constexpr int64_t taxoGen = int64_t ::valueOf(parts[1], RADIX);
  constexpr int64_t indexCommitGen = indexCommit->getGeneration();
  constexpr int64_t taxoCommitGen = taxoCommit->getGeneration();

  // if the index generation is not the same as this commit's generation,
  // compare by it. Otherwise, compare by the taxonomy generation.
  if (indexCommitGen < indexGen) {
    return -1;
  } else if (indexCommitGen > indexGen) {
    return 1;
  } else {
    return taxoCommitGen < taxoGen ? -1 : (taxoCommitGen > taxoGen ? 1 : 0);
  }
}

int IndexAndTaxonomyRevision::compareTo(shared_ptr<Revision> o)
{
  shared_ptr<IndexAndTaxonomyRevision> other =
      std::static_pointer_cast<IndexAndTaxonomyRevision>(o);
  int cmp = indexCommit->compareTo(other->indexCommit);
  return cmp != 0 ? cmp : taxoCommit->compareTo(other->taxoCommit);
}

wstring IndexAndTaxonomyRevision::getVersion() { return version; }

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
IndexAndTaxonomyRevision::getSourceFiles()
{
  return sourceFiles;
}

shared_ptr<InputStream>
IndexAndTaxonomyRevision::open(const wstring &source,
                               const wstring &fileName) 
{
  assert((source == INDEX_SOURCE || source == TAXONOMY_SOURCE,
          L"invalid source; expected=(" + INDEX_SOURCE + L" or " +
              TAXONOMY_SOURCE + L") got=" + source));
  shared_ptr<IndexCommit> ic =
      source == INDEX_SOURCE ? indexCommit : taxoCommit;
  return make_shared<IndexInputInputStream>(
      ic->getDirectory()->openInput(fileName, IOContext::READONCE));
}

void IndexAndTaxonomyRevision::release() 
{
  try {
    indexSDP->release(indexCommit);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    taxoSDP->release(taxoCommit);
  }

  try {
    indexWriter->deleteUnusedFiles();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    taxoWriter->getIndexWriter()->deleteUnusedFiles();
  }
}

wstring IndexAndTaxonomyRevision::toString()
{
  return L"IndexAndTaxonomyRevision version=" + version + L" files=" +
         sourceFiles;
}
} // namespace org::apache::lucene::replicator