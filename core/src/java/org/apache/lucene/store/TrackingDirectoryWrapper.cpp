using namespace std;

#include "TrackingDirectoryWrapper.h"

namespace org::apache::lucene::store
{

TrackingDirectoryWrapper::TrackingDirectoryWrapper(shared_ptr<Directory> in_)
    : FilterDirectory(in_)
{
}

void TrackingDirectoryWrapper::deleteFile(const wstring &name) throw(
    IOException)
{
  in_->deleteFile(name);
  createdFileNames->remove(name);
}

shared_ptr<IndexOutput> TrackingDirectoryWrapper::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> output = in_->createOutput(name, context);
  createdFileNames->add(name);
  return output;
}

shared_ptr<IndexOutput> TrackingDirectoryWrapper::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> tempOutput =
      in_->createTempOutput(prefix, suffix, context);
  createdFileNames->add(tempOutput->getName());
  return tempOutput;
}

void TrackingDirectoryWrapper::copyFrom(
    shared_ptr<Directory> from, const wstring &src, const wstring &dest,
    shared_ptr<IOContext> context) 
{
  in_->copyFrom(from, src, dest, context);
  createdFileNames->add(dest);
}

void TrackingDirectoryWrapper::rename(const wstring &source,
                                      const wstring &dest) 
{
  in_->rename(source, dest);
  {
    lock_guard<mutex> lock(createdFileNames);
    createdFileNames->add(dest);
    createdFileNames->remove(source);
  }
}

shared_ptr<Set<wstring>> TrackingDirectoryWrapper::getCreatedFiles()
{
  return unordered_set<>(createdFileNames);
}

void TrackingDirectoryWrapper::clearCreatedFiles()
{
  createdFileNames->clear();
}
} // namespace org::apache::lucene::store