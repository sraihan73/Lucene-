using namespace std;

#include "TrackingTmpOutputDirectoryWrapper.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

TrackingTmpOutputDirectoryWrapper::TrackingTmpOutputDirectoryWrapper(
    shared_ptr<Directory> in_)
    : org::apache::lucene::store::FilterDirectory(in_)
{
}

shared_ptr<IndexOutput> TrackingTmpOutputDirectoryWrapper::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> output =
      FilterDirectory::createTempOutput(name, L"", context);
  fileNames.emplace(name, output->getName());
  return output;
}

shared_ptr<IndexInput> TrackingTmpOutputDirectoryWrapper::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  wstring tmpName = fileNames[name];
  return FilterDirectory::openInput(tmpName, context);
}

unordered_map<wstring, wstring>
TrackingTmpOutputDirectoryWrapper::getTemporaryFiles()
{
  return fileNames;
}
} // namespace org::apache::lucene::index