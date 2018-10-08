using namespace std;

#include "VirusCheckingFS.h"

namespace org::apache::lucene::mockfile
{
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

VirusCheckingFS::VirusCheckingFS(shared_ptr<FileSystem> delegate_,
                                 int64_t salt)
    : FilterFileSystemProvider(L"viruschecking://", delegate_),
      state(make_shared<AtomicLong>(salt))
{
}

void VirusCheckingFS::enable() { enabled = true; }

bool VirusCheckingFS::isEnabled() { return enabled; }

void VirusCheckingFS::disable() { enabled = false; }

void VirusCheckingFS::delete_(shared_ptr<Path> path) 
{

  // Fake but deterministic and hopefully portable like-randomness:
  int64_t hash = state->incrementAndGet() * path->getFileName().hashCode();

  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (enabled && Files::exists(path) &&
      path->getFileName()->toString()->equals(IndexWriter::WRITE_LOCK_NAME) ==
          false &&
      (hash % 5) == 1) {
    if (LuceneTestCase::VERBOSE) {
      wcout << L"NOTE: VirusCheckingFS now refusing to delete " << path << endl;
    }
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.AccessDeniedException(
        L"VirusCheckingFS is randomly refusing to delete file \"" + path +
        L"\"");
  }
  FilterFileSystemProvider::delete (path);
}
} // namespace org::apache::lucene::mockfile