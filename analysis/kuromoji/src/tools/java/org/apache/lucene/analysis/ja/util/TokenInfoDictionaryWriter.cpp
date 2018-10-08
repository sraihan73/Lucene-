using namespace std;

#include "TokenInfoDictionaryWriter.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/dict/TokenInfoDictionary.h"

namespace org::apache::lucene::analysis::ja::util
{
using TokenInfoDictionary =
    org::apache::lucene::analysis::ja::dict::TokenInfoDictionary;
using FST = org::apache::lucene::util::fst::FST;

TokenInfoDictionaryWriter::TokenInfoDictionaryWriter(int size)
    : BinaryDictionaryWriter(TokenInfoDictionary::class, size)
{
}

void TokenInfoDictionaryWriter::setFST(shared_ptr<FST<int64_t>> fst)
{
  this->fst = fst;
}

void TokenInfoDictionaryWriter::write(const wstring &baseDir) 
{
  BinaryDictionaryWriter::write(baseDir);
  writeFST(getBaseFileName(baseDir) + TokenInfoDictionary::FST_FILENAME_SUFFIX);
}

void TokenInfoDictionaryWriter::writeFST(const wstring &filename) throw(
    IOException)
{
  shared_ptr<Path> p = Paths->get(filename);
  Files::createDirectories(p->getParent());
  fst->save(p);
}
} // namespace org::apache::lucene::analysis::ja::util