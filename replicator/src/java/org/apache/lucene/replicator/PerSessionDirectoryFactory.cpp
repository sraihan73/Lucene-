using namespace std;

#include "PerSessionDirectoryFactory.h"

namespace org::apache::lucene::replicator
{
using SourceDirectoryFactory =
    org::apache::lucene::replicator::ReplicationClient::SourceDirectoryFactory;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;

PerSessionDirectoryFactory::PerSessionDirectoryFactory(shared_ptr<Path> workDir)
    : workDir(workDir)
{
}

shared_ptr<Directory> PerSessionDirectoryFactory::getDirectory(
    const wstring &sessionID, const wstring &source) 
{
  shared_ptr<Path> sessionDir = workDir->resolve(sessionID);
  Files::createDirectories(sessionDir);
  shared_ptr<Path> sourceDir = sessionDir->resolve(source);
  Files::createDirectories(sourceDir);
  return FSDirectory::open(sourceDir);
}

void PerSessionDirectoryFactory::cleanupSession(const wstring &sessionID) throw(
    IOException)
{
  if (sessionID.isEmpty()) { // protect against deleting workDir entirely!
    throw invalid_argument(L"sessionID cannot be empty");
  }
  IOUtils::rm({workDir->resolve(sessionID)});
}
} // namespace org::apache::lucene::replicator