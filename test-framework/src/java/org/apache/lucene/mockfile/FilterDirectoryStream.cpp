using namespace std;

#include "FilterDirectoryStream.h"

namespace org::apache::lucene::mockfile
{

FilterDirectoryStream::FilterDirectoryStream(
    shared_ptr<DirectoryStream<std::shared_ptr<Path>>> delegate_,
    shared_ptr<FileSystem> fileSystem)
    : delegate_(Objects::requireNonNull(delegate_)),
      fileSystem(Objects::requireNonNull(fileSystem))
{
}

FilterDirectoryStream::~FilterDirectoryStream() { delegate_->close(); }

shared_ptr<Iterator<std::shared_ptr<Path>>> FilterDirectoryStream::iterator()
{
  constexpr DirectoryStream<std::shared_ptr<Path>>::const_iterator
      delegateIterator = delegate_->begin();
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                  delegateIterator);
}

FilterDirectoryStream::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<FilterDirectoryStream> outerInstance,
    DirectoryStream<Path>::const_iterator delegateIterator)
{
  this->outerInstance = outerInstance;
  this->delegateIterator = delegateIterator;
}

bool FilterDirectoryStream::IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return delegateIterator->hasNext();
}

shared_ptr<Path> FilterDirectoryStream::IteratorAnonymousInnerClass::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return make_shared<FilterPath>(delegateIterator->next(),
                                 outerInstance->fileSystem);
}

void FilterDirectoryStream::IteratorAnonymousInnerClass::remove()
{
  delegateIterator->remove();
}
} // namespace org::apache::lucene::mockfile