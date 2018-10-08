using namespace std;

#include "OutputStreamDataOutput.h"

namespace org::apache::lucene::store
{

OutputStreamDataOutput::OutputStreamDataOutput(shared_ptr<OutputStream> os)
    : os(os)
{
}

void OutputStreamDataOutput::writeByte(char b) 
{
  os->write(b);
}

void OutputStreamDataOutput::writeBytes(std::deque<char> &b, int offset,
                                        int length) 
{
  os->write(b, offset, length);
}

OutputStreamDataOutput::~OutputStreamDataOutput() { os->close(); }
} // namespace org::apache::lucene::store