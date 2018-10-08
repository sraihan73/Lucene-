using namespace std;

#include "BufferedChecksumIndexInput.h"

namespace org::apache::lucene::store
{

BufferedChecksumIndexInput::BufferedChecksumIndexInput(
    shared_ptr<IndexInput> main)
    : ChecksumIndexInput(L"BufferedChecksumIndexInput(" + main + L")"),
      main(main), digest(make_shared<BufferedChecksum>(make_shared<CRC32>()))
{
}

char BufferedChecksumIndexInput::readByte() 
{
  constexpr char b = main->readByte();
  digest->update(b);
  return b;
}

void BufferedChecksumIndexInput::readBytes(std::deque<char> &b, int offset,
                                           int len) 
{
  main->readBytes(b, offset, len);
  digest->update(b, offset, len);
}

int64_t BufferedChecksumIndexInput::getChecksum()
{
  return digest->getValue();
}

BufferedChecksumIndexInput::~BufferedChecksumIndexInput() { delete main; }

int64_t BufferedChecksumIndexInput::getFilePointer()
{
  return main->getFilePointer();
}

int64_t BufferedChecksumIndexInput::length() { return main->length(); }

shared_ptr<IndexInput> BufferedChecksumIndexInput::clone()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<IndexInput>
BufferedChecksumIndexInput::slice(const wstring &sliceDescription,
                                  int64_t offset,
                                  int64_t length) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::store