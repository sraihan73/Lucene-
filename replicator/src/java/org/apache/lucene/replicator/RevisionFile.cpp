using namespace std;

#include "RevisionFile.h"

namespace org::apache::lucene::replicator
{

RevisionFile::RevisionFile(const wstring &fileName) : fileName(fileName)
{
  if (fileName == L"" || fileName.isEmpty()) {
    throw invalid_argument(L"fileName must not be null or empty");
  }
}

bool RevisionFile::equals(any obj)
{
  shared_ptr<RevisionFile> other = any_cast<std::shared_ptr<RevisionFile>>(obj);
  return fileName == other->fileName && size == other->size;
}

int RevisionFile::hashCode()
{
  return fileName.hashCode() ^
         static_cast<int>(size ^
                          (static_cast<int64_t>(
                              static_cast<uint64_t>(size) >> 32)));
}

wstring RevisionFile::toString()
{
  return L"fileName=" + fileName + L" size=" + to_wstring(size);
}
} // namespace org::apache::lucene::replicator