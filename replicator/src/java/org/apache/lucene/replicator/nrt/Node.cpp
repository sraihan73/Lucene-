using namespace std;

#include "Node.h"

namespace org::apache::lucene::replicator::nrt
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
bool Node::VERBOSE_FILES = true;
bool Node::VERBOSE_CONNECTIONS = false;
wstring Node::PRIMARY_GEN_KEY = L"__primaryGen";
wstring Node::VERSION_KEY = L"__version";
int64_t Node::globalStartNS = 0;

Node::Node(int id, shared_ptr<Directory> dir,
           shared_ptr<SearcherFactory> searcherFactory,
           shared_ptr<PrintStream> printStream)
    : id(id), dir(dir), searcherFactory(searcherFactory),
      printStream(printStream)
{
}

shared_ptr<ReferenceManager<std::shared_ptr<IndexSearcher>>>
Node::getSearcherManager()
{
  return mgr;
}

shared_ptr<Directory> Node::getDirectory() { return dir; }

wstring Node::toString()
{
  return getClass().getSimpleName() + L"(id=" + to_wstring(id) + L")";
}

void Node::nodeMessage(shared_ptr<PrintStream> printStream,
                       const wstring &message)
{
  if (printStream != nullptr) {
    int64_t now = System::nanoTime();
    printStream->println(
        wstring::format(Locale::ROOT, L"%5.3fs %5.1fs:           [%11s] %s",
                        (now - globalStartNS) / 1000000000.0,
                        (now - localStartNS) / 1000000000.0,
                        Thread::currentThread().getName(), message));
  }
}

void Node::nodeMessage(shared_ptr<PrintStream> printStream, int id,
                       const wstring &message)
{
  if (printStream != nullptr) {
    int64_t now = System::nanoTime();
    printStream->println(
        wstring::format(Locale::ROOT, L"%5.3fs %5.1fs:         N%d [%11s] %s",
                        (now - globalStartNS) / 1000000000.0,
                        (now - localStartNS) / 1000000000.0, id,
                        Thread::currentThread().getName(), message));
  }
}

void Node::message(const wstring &message)
{
  if (printStream != nullptr) {
    int64_t now = System::nanoTime();
    printStream->println(
        wstring::format(Locale::ROOT, L"%5.3fs %5.1fs: %7s %2s [%11s] %s",
                        (now - globalStartNS) / 1000000000.0,
                        (now - localStartNS) / 1000000000.0, state, name(),
                        Thread::currentThread().getName(), message));
  }
}

wstring Node::name()
{
  wchar_t mode =
      std::dynamic_pointer_cast<PrimaryNode>(shared_from_this()) != nullptr
          ? L'P'
          : L'R';
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return mode + Integer::toString(id);
}

int64_t Node::getCurrentSearchingVersion() 
{
  shared_ptr<IndexSearcher> searcher = mgr->acquire();
  try {
    return (std::static_pointer_cast<DirectoryReader>(
                searcher->getIndexReader()))
        ->getVersion();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(searcher);
  }
}

wstring Node::bytesToString(int64_t bytes)
{
  if (bytes < 1024) {
    return to_wstring(bytes) + L" b";
  } else if (bytes < 1024 * 1024) {
    return wstring::format(Locale::ROOT, L"%.1f KB", bytes / 1024.0);
  } else if (bytes < 1024 * 1024 * 1024) {
    return wstring::format(Locale::ROOT, L"%.1f MB", bytes / 1024.0 / 1024.0);
  } else {
    return wstring::format(Locale::ROOT, L"%.1f GB",
                           bytes / 1024.0 / 1024.0 / 1024.0);
  }
}

shared_ptr<FileMetaData>
Node::readLocalFileMetaData(const wstring &fileName) 
{

  unordered_map<wstring, std::shared_ptr<FileMetaData>> &cache =
      lastFileMetaData;
  shared_ptr<FileMetaData> result;
  if (cache.size() > 0) {
    // We may already have this file cached from the last NRT point:
    result = cache[fileName];
  } else {
    result.reset();
  }

  if (result == nullptr) {
    // Pull from the filesystem
    int64_t checksum;
    int64_t length;
    std::deque<char> header;
    std::deque<char> footer;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput(fileName, org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          fileName, org::apache::lucene::store::IOContext::DEFAULT);
      try {
        try {
          length = in_->length();
          header = CodecUtil::readIndexHeader(in_);
          footer = CodecUtil::readFooter(in_);
          checksum = CodecUtil::retrieveChecksum(in_);
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (EOFException | CorruptIndexException cie) {
          // File exists but is busted: we must copy it.  This happens when node
          // had crashed, corrupting an un-fsync'd file.  On init we try to
          // delete such unreferenced files, but virus checker can block that,
          // leaving this bad file.
          if (VERBOSE_FILES) {
            message(L"file " + fileName +
                    L": will copy [existing file is corrupt]");
          }
          return nullptr;
        }
        if (VERBOSE_FILES) {
          message(L"file " + fileName + L" has length=" +
                  bytesToString(length));
        }
      }
      // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
      catch (FileNotFoundException | NoSuchFileException e) {
        if (VERBOSE_FILES) {
          message(L"file " + fileName + L": will copy [file does not exist]");
        }
        return nullptr;
      }
    }

    // NOTE: checksum is redundant w/ footer, but we break it out separately
    // because when the bits cross the wire we need direct access to checksum
    // when copying to catch bit flips:
    result = make_shared<FileMetaData>(header, footer, length, checksum);
  }

  return result;
}
} // namespace org::apache::lucene::replicator::nrt