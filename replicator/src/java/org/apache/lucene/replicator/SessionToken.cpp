using namespace std;

#include "SessionToken.h"

namespace org::apache::lucene::replicator
{

SessionToken::SessionToken(shared_ptr<DataInput> in_) 
    : id(in_->readUTF()), version(in_->readUTF()),
      sourceFiles(unordered_map<>())
{
  int numSources = in_->readInt();
  while (numSources > 0) {
    wstring source = in_->readUTF();
    int numFiles = in_->readInt();
    deque<std::shared_ptr<RevisionFile>> files =
        deque<std::shared_ptr<RevisionFile>>(numFiles);
    for (int i = 0; i < numFiles; i++) {
      wstring fileName = in_->readUTF();
      shared_ptr<RevisionFile> file = make_shared<RevisionFile>(fileName);
      file->size = in_->readLong();
      files.push_back(file);
    }
    this->sourceFiles.emplace(source, files);
    --numSources;
  }
}

SessionToken::SessionToken(const wstring &id, shared_ptr<Revision> revision)
    : id(id), version(revision->getVersion()),
      sourceFiles(revision->getSourceFiles())
{
}

void SessionToken::serialize(shared_ptr<DataOutput> out) 
{
  out->writeUTF(id);
  out->writeUTF(version);
  out->writeInt(sourceFiles.size());
  for (auto e : sourceFiles) {
    out->writeUTF(e.first);
    deque<std::shared_ptr<RevisionFile>> files = e.second;
    out->writeInt(files.size());
    for (auto file : files) {
      out->writeUTF(file->fileName);
      out->writeLong(file->size);
    }
  }
}

wstring SessionToken::toString()
{
  return L"id=" + id + L" version=" + version + L" files=" + sourceFiles;
}
} // namespace org::apache::lucene::replicator