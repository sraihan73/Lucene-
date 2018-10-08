using namespace std;

#include "OutputStreamIndexOutput.h"

namespace org::apache::lucene::store
{

OutputStreamIndexOutput::OutputStreamIndexOutput(
    const wstring &resourceDescription, const wstring &name,
    shared_ptr<OutputStream> out, int bufferSize)
    : IndexOutput(resourceDescription, name),
      os(make_shared<BufferedOutputStream>(
          make_shared<CheckedOutputStream>(out, crc), bufferSize))
{
}

void OutputStreamIndexOutput::writeByte(char b) 
{
  os->write(b);
  bytesWritten++;
}

void OutputStreamIndexOutput::writeBytes(std::deque<char> &b, int offset,
                                         int length) 
{
  os->write(b, offset, length);
  bytesWritten += length;
}

OutputStreamIndexOutput::~OutputStreamIndexOutput()
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final java.io.OutputStream o = os)
  {
    shared_ptr<java::io::OutputStream> *const o = os;
    // We want to make sure that os.flush() was running before close:
    // BufferedOutputStream may ignore IOExceptions while flushing on close().
    // We keep this also in Java 8, although it claims to be fixed there,
    // because there are more bugs around this! See:
    // # https://bugs.openjdk.java.net/browse/JDK-7015589
    // # https://bugs.openjdk.java.net/browse/JDK-8054565
    if (!flushedOnClose) {
      flushedOnClose = true; // set this BEFORE calling flush!
      o->flush();
    }
  }
}

int64_t OutputStreamIndexOutput::getFilePointer() { return bytesWritten; }

int64_t OutputStreamIndexOutput::getChecksum() 
{
  os->flush();
  return crc->getValue();
}
} // namespace org::apache::lucene::store