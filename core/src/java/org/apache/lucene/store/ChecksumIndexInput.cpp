using namespace std;

#include "ChecksumIndexInput.h"

namespace org::apache::lucene::store
{

ChecksumIndexInput::ChecksumIndexInput(const wstring &resourceDescription)
    : IndexInput(resourceDescription)
{
}

void ChecksumIndexInput::seek(int64_t pos) 
{
  constexpr int64_t curFP = getFilePointer();
  constexpr int64_t skip = pos - curFP;
  if (skip < 0) {
    throw make_shared<IllegalStateException>(
        getClass() + L" cannot seek backwards (pos=" + to_wstring(pos) +
        L" getFilePointer()=" + to_wstring(curFP) + L")");
  }
  skipBytes(skip);
}
} // namespace org::apache::lucene::store