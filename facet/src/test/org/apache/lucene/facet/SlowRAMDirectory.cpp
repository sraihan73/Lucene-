using namespace std;

#include "SlowRAMDirectory.h"

namespace org::apache::lucene::facet
{
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

void SlowRAMDirectory::setSleepMillis(int sleepMillis)
{
  this->sleepMillis = sleepMillis;
}

SlowRAMDirectory::SlowRAMDirectory(int sleepMillis, shared_ptr<Random> random)
{
  this->sleepMillis = sleepMillis;
  this->random = random;
}

shared_ptr<IndexOutput>
SlowRAMDirectory::createOutput(const wstring &name,
                               shared_ptr<IOContext> context) 
{
  if (sleepMillis != -1) {
    return make_shared<SlowIndexOutput>(
        shared_from_this(), RAMDirectory::createOutput(name, context));
  }

  return RAMDirectory::createOutput(name, context);
}

shared_ptr<IndexInput>
SlowRAMDirectory::openInput(const wstring &name,
                            shared_ptr<IOContext> context) 
{
  if (sleepMillis != -1) {
    return make_shared<SlowIndexInput>(shared_from_this(),
                                       RAMDirectory::openInput(name, context));
  }
  return RAMDirectory::openInput(name, context);
}

void SlowRAMDirectory::doSleep(shared_ptr<Random> random, int length)
{
  int sTime =
      length < 10 ? sleepMillis : static_cast<int>(sleepMillis * log(length));
  if (random != nullptr) {
    sTime = random->nextInt(sTime);
  }
  try {
    delay(sTime);
  } catch (const InterruptedException &e) {
    throw make_shared<ThreadInterruptedException>(e);
  }
}

shared_ptr<Random> SlowRAMDirectory::forkRandom()
{
  if (random == nullptr) {
    return nullptr;
  }
  return make_shared<Random>(random->nextLong());
}

SlowRAMDirectory::SlowIndexInput::SlowIndexInput(
    shared_ptr<SlowRAMDirectory> outerInstance, shared_ptr<IndexInput> ii)
    : org::apache::lucene::store::IndexInput(L"SlowIndexInput(" + ii + L")"),
      outerInstance(outerInstance)
{
  this->rand = outerInstance->forkRandom();
  this->ii = ii;
}

char SlowRAMDirectory::SlowIndexInput::readByte() 
{
  if (numRead >= IO_SLEEP_THRESHOLD) {
    outerInstance->doSleep(rand, 0);
    numRead = 0;
  }
  ++numRead;
  return ii->readByte();
}

void SlowRAMDirectory::SlowIndexInput::readBytes(std::deque<char> &b,
                                                 int offset,
                                                 int len) 
{
  if (numRead >= IO_SLEEP_THRESHOLD) {
    outerInstance->doSleep(rand, len);
    numRead = 0;
  }
  numRead += len;
  ii->readBytes(b, offset, len);
}

shared_ptr<IndexInput> SlowRAMDirectory::SlowIndexInput::clone()
{
  return ii->clone();
}

shared_ptr<IndexInput>
SlowRAMDirectory::SlowIndexInput::slice(const wstring &sliceDescription,
                                        int64_t offset,
                                        int64_t length) 
{
  return ii->slice(sliceDescription, offset, length);
}

SlowRAMDirectory::SlowIndexInput::~SlowIndexInput() { delete ii; }

bool SlowRAMDirectory::SlowIndexInput::equals(any o) { return ii->equals(o); }

int64_t SlowRAMDirectory::SlowIndexInput::getFilePointer()
{
  return ii->getFilePointer();
}

int SlowRAMDirectory::SlowIndexInput::hashCode() { return ii->hashCode(); }

int64_t SlowRAMDirectory::SlowIndexInput::length() { return ii->length(); }

void SlowRAMDirectory::SlowIndexInput::seek(int64_t pos) 
{
  ii->seek(pos);
}

SlowRAMDirectory::SlowIndexOutput::SlowIndexOutput(
    shared_ptr<SlowRAMDirectory> outerInstance, shared_ptr<IndexOutput> io)
    : org::apache::lucene::store::IndexOutput(L"SlowIndexOutput(" + io + L")",
                                              io->getName()),
      rand(outerInstance->forkRandom()), outerInstance(outerInstance)
{
  this->io = io;
}

void SlowRAMDirectory::SlowIndexOutput::writeByte(char b) 
{
  if (numWrote >= IO_SLEEP_THRESHOLD) {
    outerInstance->doSleep(rand, 0);
    numWrote = 0;
  }
  ++numWrote;
  io->writeByte(b);
}

void SlowRAMDirectory::SlowIndexOutput::writeBytes(
    std::deque<char> &b, int offset, int length) 
{
  if (numWrote >= IO_SLEEP_THRESHOLD) {
    outerInstance->doSleep(rand, length);
    numWrote = 0;
  }
  numWrote += length;
  io->writeBytes(b, offset, length);
}

SlowRAMDirectory::SlowIndexOutput::~SlowIndexOutput() { delete io; }

int64_t SlowRAMDirectory::SlowIndexOutput::getFilePointer()
{
  return io->getFilePointer();
}

int64_t SlowRAMDirectory::SlowIndexOutput::getChecksum() 
{
  return io->getChecksum();
}
} // namespace org::apache::lucene::facet