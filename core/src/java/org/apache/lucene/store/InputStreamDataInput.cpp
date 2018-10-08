using namespace std;

#include "InputStreamDataInput.h"

namespace org::apache::lucene::store
{
using DataInput = org::apache::lucene::store::DataInput;

InputStreamDataInput::InputStreamDataInput(shared_ptr<InputStream> is) : is(is)
{
}

char InputStreamDataInput::readByte() 
{
  int v = is->read();
  if (v == -1) {
    throw make_shared<EOFException>();
  }
  return static_cast<char>(v);
}

void InputStreamDataInput::readBytes(std::deque<char> &b, int offset,
                                     int len) 
{
  while (len > 0) {
    constexpr int cnt = is->read(b, offset, len);
    if (cnt < 0) {
      // Partially read the input, but no more data available in the stream.
      throw make_shared<EOFException>();
    }
    len -= cnt;
    offset += cnt;
  }
}

InputStreamDataInput::~InputStreamDataInput() { is->close(); }
} // namespace org::apache::lucene::store