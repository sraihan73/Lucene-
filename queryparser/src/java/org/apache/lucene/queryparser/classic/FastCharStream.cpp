using namespace std;

#include "FastCharStream.h"

namespace org::apache::lucene::queryparser::classic
{

FastCharStream::FastCharStream(shared_ptr<Reader> r) { input = r; }

wchar_t FastCharStream::readChar() 
{
  if (bufferPosition >= bufferLength) {
    refill();
  }
  return buffer[bufferPosition++];
}

void FastCharStream::refill() 
{
  int newPosition = bufferLength - tokenStart;

  if (tokenStart == 0) {  // token won't fit in buffer
    if (buffer.empty()) { // first time: alloc buffer
      buffer = std::deque<wchar_t>(2048);
    } else if (bufferLength == buffer.size()) { // grow buffer
      std::deque<wchar_t> newBuffer(buffer.size() * 2);
      System::arraycopy(buffer, 0, newBuffer, 0, bufferLength);
      buffer = newBuffer;
    }
  } else { // shift token to front
    System::arraycopy(buffer, tokenStart, buffer, 0, newPosition);
  }

  bufferLength = newPosition; // update state
  bufferPosition = newPosition;
  bufferStart += tokenStart;
  tokenStart = 0;

  int charsRead = input->read(buffer, newPosition, buffer.size() - newPosition);
  if (charsRead == -1) {
    throw make_shared<IOException>(L"read past eof");
  } else {
    bufferLength += charsRead;
  }
}

wchar_t FastCharStream::BeginToken() 
{
  tokenStart = bufferPosition;
  return readChar();
}

void FastCharStream::backup(int amount) { bufferPosition -= amount; }

wstring FastCharStream::GetImage()
{
  return wstring(buffer, tokenStart, bufferPosition - tokenStart);
}

std::deque<wchar_t> FastCharStream::GetSuffix(int len)
{
  std::deque<wchar_t> value(len);
  System::arraycopy(buffer, bufferPosition - len, value, 0, len);
  return value;
}

void FastCharStream::Done()
{
  try {
    input->close();
  } catch (const IOException &e) {
  }
}

int FastCharStream::getColumn() { return bufferStart + bufferPosition; }

int FastCharStream::getLine() { return 1; }

int FastCharStream::getEndColumn() { return bufferStart + bufferPosition; }

int FastCharStream::getEndLine() { return 1; }

int FastCharStream::getBeginColumn() { return bufferStart + tokenStart; }

int FastCharStream::getBeginLine() { return 1; }
} // namespace org::apache::lucene::queryparser::classic