using namespace std;

#include "DirectoryReader.h"

namespace org::apache::lucene::index
{
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Directory = org::apache::lucene::store::Directory;

shared_ptr<DirectoryReader>
DirectoryReader::open(shared_ptr<Directory> directory) 
{
  return StandardDirectoryReader::open(directory, nullptr);
}

shared_ptr<DirectoryReader>
DirectoryReader::open(shared_ptr<IndexWriter> writer) 
{
  return open(writer, true, false);
}

shared_ptr<DirectoryReader>
DirectoryReader::open(shared_ptr<IndexWriter> writer, bool applyAllDeletes,
                      bool writeAllDeletes) 
{
  return writer->getReader(applyAllDeletes, writeAllDeletes);
}

shared_ptr<DirectoryReader>
DirectoryReader::open(shared_ptr<IndexCommit> commit) 
{
  return StandardDirectoryReader::open(commit->getDirectory(), commit);
}

shared_ptr<DirectoryReader> DirectoryReader::openIfChanged(
    shared_ptr<DirectoryReader> oldReader) 
{
  shared_ptr<DirectoryReader> *const newReader = oldReader->doOpenIfChanged();
  assert(newReader != oldReader);
  return newReader;
}

shared_ptr<DirectoryReader> DirectoryReader::openIfChanged(
    shared_ptr<DirectoryReader> oldReader,
    shared_ptr<IndexCommit> commit) 
{
  shared_ptr<DirectoryReader> *const newReader =
      oldReader->doOpenIfChanged(commit);
  assert(newReader != oldReader);
  return newReader;
}

shared_ptr<DirectoryReader> DirectoryReader::openIfChanged(
    shared_ptr<DirectoryReader> oldReader,
    shared_ptr<IndexWriter> writer) 
{
  return openIfChanged(oldReader, writer, true);
}

shared_ptr<DirectoryReader>
DirectoryReader::openIfChanged(shared_ptr<DirectoryReader> oldReader,
                               shared_ptr<IndexWriter> writer,
                               bool applyAllDeletes) 
{
  shared_ptr<DirectoryReader> *const newReader =
      oldReader->doOpenIfChanged(writer, applyAllDeletes);
  assert(newReader != oldReader);
  return newReader;
}

deque<std::shared_ptr<IndexCommit>>
DirectoryReader::listCommits(shared_ptr<Directory> dir) 
{
  const std::deque<wstring> files = dir->listAll();

  deque<std::shared_ptr<IndexCommit>> commits =
      deque<std::shared_ptr<IndexCommit>>();

  shared_ptr<SegmentInfos> latest = SegmentInfos::readLatestCommit(dir);
  constexpr int64_t currentGen = latest->getGeneration();

  commits.push_back(
      make_shared<StandardDirectoryReader::ReaderCommit>(nullptr, latest, dir));

  for (int i = 0; i < files.size(); i++) {

    const wstring fileName = files[i];

    if (StringHelper::startsWith(fileName, IndexFileNames::SEGMENTS) &&
        fileName != IndexFileNames::OLD_SEGMENTS_GEN &&
        SegmentInfos::generationFromSegmentsFileName(fileName) < currentGen) {

      shared_ptr<SegmentInfos> sis;
      try {
        // IOException allowed to throw there, in case
        // segments_N is corrupt
        sis = SegmentInfos::readCommit(dir, fileName);
      }
      // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
      catch (FileNotFoundException | NoSuchFileException fnfe) {
        // LUCENE-948: on NFS (and maybe others), if
        // you have writers switching back and forth
        // between machines, it's very likely that the
        // dir listing will be stale and will claim a
        // file segments_X exists when in fact it
        // doesn't.  So, we catch this and handle it
        // as if the file does not exist
      }

      if (sis->size() > 0) {
        commits.push_back(make_shared<StandardDirectoryReader::ReaderCommit>(
            nullptr, sis, dir));
      }
    }
  }

  // Ensure that the commit points are sorted in ascending order.
  sort(commits.begin(), commits.end());

  return commits;
}

bool DirectoryReader::indexExists(shared_ptr<Directory> directory) throw(
    IOException)
{
  // LUCENE-2812, LUCENE-2727, LUCENE-4738: this logic will
  // return true in cases that should arguably be false,
  // such as only IW.prepareCommit has been called, or a
  // corrupt first commit, but it's too deadly to make
  // this logic "smarter" and risk accidentally returning
  // false due to various cases like file description
  // exhaustion, access denied, etc., because in that
  // case IndexWriter may delete the entire index.  It's
  // safer to err towards "index exists" than try to be
  // smart about detecting not-yet-fully-committed or
  // corrupt indices.  This means that IndexWriter will
  // throw an exception on such indices and the app must
  // resolve the situation manually:
  std::deque<wstring> files = directory->listAll();

  wstring prefix = IndexFileNames::SEGMENTS + L"_";
  for (auto file : files) {
    if (file.startsWith(prefix)) {
      return true;
    }
  }
  return false;
}

DirectoryReader::DirectoryReader(
    shared_ptr<Directory> directory,
    std::deque<std::shared_ptr<LeafReader>> &segmentReaders) 
    : BaseCompositeReader<LeafReader>(segmentReaders), directory(directory)
{
}

shared_ptr<Directory> DirectoryReader::directory()
{
  // Don't ensureOpen here -- in certain cases, when a
  // cloned/reopened reader needs to commit, it may call
  // this method on the closed original reader
  return directory_;
}
} // namespace org::apache::lucene::index