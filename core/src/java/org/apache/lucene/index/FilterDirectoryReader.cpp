using namespace std;

#include "FilterDirectoryReader.h"

namespace org::apache::lucene::index
{

shared_ptr<DirectoryReader>
FilterDirectoryReader::unwrap(shared_ptr<DirectoryReader> reader)
{
  while (std::dynamic_pointer_cast<FilterDirectoryReader>(reader) != nullptr) {
    reader = (std::static_pointer_cast<FilterDirectoryReader>(reader))->in_;
  }
  return reader;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private LeafReader[] wrap(java.util.List<? extends
// LeafReader> readers)
std::deque<std::shared_ptr<LeafReader>>
FilterDirectoryReader::SubReaderWrapper::wrap(deque<T1> readers)
{
  std::deque<std::shared_ptr<LeafReader>> wrapped(readers.size());
  for (int i = 0; i < readers.size(); i++) {
    wrapped[i] = wrap(readers[i]);
  }
  return wrapped;
}

FilterDirectoryReader::SubReaderWrapper::SubReaderWrapper() {}

FilterDirectoryReader::FilterDirectoryReader(
    shared_ptr<DirectoryReader> in_,
    shared_ptr<SubReaderWrapper> wrapper) 
    : DirectoryReader(in_->directory(),
                      wrapper->wrap(in_->getSequentialSubReaders())),
      in_(in_)
{
}

shared_ptr<DirectoryReader> FilterDirectoryReader::wrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return in_ == nullptr ? nullptr : doWrapDirectoryReader(in_);
}

shared_ptr<DirectoryReader>
FilterDirectoryReader::doOpenIfChanged() 
{
  return wrapDirectoryReader(in_->doOpenIfChanged());
}

shared_ptr<DirectoryReader> FilterDirectoryReader::doOpenIfChanged(
    shared_ptr<IndexCommit> commit) 
{
  return wrapDirectoryReader(in_->doOpenIfChanged(commit));
}

shared_ptr<DirectoryReader>
FilterDirectoryReader::doOpenIfChanged(shared_ptr<IndexWriter> writer,
                                       bool applyAllDeletes) 
{
  return wrapDirectoryReader(in_->doOpenIfChanged(writer, applyAllDeletes));
}

int64_t FilterDirectoryReader::getVersion() { return in_->getVersion(); }

bool FilterDirectoryReader::isCurrent() 
{
  return in_->isCurrent();
}

shared_ptr<IndexCommit>
FilterDirectoryReader::getIndexCommit() 
{
  return in_->getIndexCommit();
}

void FilterDirectoryReader::doClose()  { in_->close(); }

shared_ptr<DirectoryReader> FilterDirectoryReader::getDelegate() { return in_; }
} // namespace org::apache::lucene::index