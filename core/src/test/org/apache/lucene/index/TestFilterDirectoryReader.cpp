using namespace std;

#include "TestFilterDirectoryReader.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using SubReaderWrapper =
    org::apache::lucene::index::FilterDirectoryReader::SubReaderWrapper;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

shared_ptr<LeafReader> TestFilterDirectoryReader::DummySubReaderWrapper::wrap(
    shared_ptr<LeafReader> reader)
{
  return reader;
}

TestFilterDirectoryReader::DummyFilterDirectoryReader::
    DummyFilterDirectoryReader(shared_ptr<DirectoryReader> in_) throw(
        IOException)
    : FilterDirectoryReader(in_, new DummySubReaderWrapper())
{
}

shared_ptr<DirectoryReader>
TestFilterDirectoryReader::DummyFilterDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<DummyFilterDirectoryReader>(in_);
}

shared_ptr<CacheHelper>
TestFilterDirectoryReader::DummyFilterDirectoryReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

void TestFilterDirectoryReader::testDoubleClose() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(w);
  shared_ptr<DirectoryReader> wrapped =
      make_shared<DummyFilterDirectoryReader>(reader);

  // Calling close() on the original reader and wrapped reader should only close
  // the original reader once (as per Closeable.close() contract that close() is
  // idempotent)
  deque<std::shared_ptr<DirectoryReader>> readers =
      Arrays::asList(reader, wrapped);
  Collections::shuffle(readers, random());
  IOUtils::close(readers);

  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index