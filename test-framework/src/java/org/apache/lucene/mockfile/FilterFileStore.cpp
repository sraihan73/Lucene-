using namespace std;

#include "FilterFileStore.h"

namespace org::apache::lucene::mockfile
{

FilterFileStore::FilterFileStore(shared_ptr<FileStore> delegate_,
                                 const wstring &scheme)
    : delegate_(Objects::requireNonNull(delegate_)),
      scheme(Objects::requireNonNull(scheme))
{
}

wstring FilterFileStore::name() { return delegate_->name(); }

wstring FilterFileStore::type() { return delegate_->type(); }

wstring FilterFileStore::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return delegate_->toString();
}

bool FilterFileStore::isReadOnly() { return delegate_->isReadOnly(); }

int64_t FilterFileStore::getTotalSpace() 
{
  return delegate_->getTotalSpace();
}

int64_t FilterFileStore::getUsableSpace() 
{
  return delegate_->getUsableSpace();
}

int64_t FilterFileStore::getUnallocatedSpace() 
{
  return delegate_->getUnallocatedSpace();
}

bool FilterFileStore::supportsFileAttributeView(type_info type)
{
  return delegate_->supportsFileAttributeView(type);
}

bool FilterFileStore::supportsFileAttributeView(const wstring &name)
{
  return delegate_->supportsFileAttributeView(name);
}

template <typename V>
V FilterFileStore::getFileStoreAttributeView(type_info<V> &type)
{
  static_assert(
      is_base_of<java.nio.file.attribute.FileStoreAttributeView, V>::value,
      L"V must inherit from java.nio.file.attribute.FileStoreAttributeView");

  return delegate_->getFileStoreAttributeView(type);
}

any FilterFileStore::getAttribute(const wstring &attribute) 
{
  return delegate_->getAttribute(attribute);
}
} // namespace org::apache::lucene::mockfile