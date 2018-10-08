using namespace std;

#include "CorruptingIndexOutput.h"

namespace org::apache::lucene::store
{

CorruptingIndexOutput::CorruptingIndexOutput(shared_ptr<Directory> dir,
                                             int64_t byteToCorrupt,
                                             shared_ptr<IndexOutput> out)
    : IndexOutput(L"CorruptingIndexOutput(" + out + L")", out->getName()),
      out(out), dir(dir), byteToCorrupt(byteToCorrupt)
{
}

wstring CorruptingIndexOutput::getName() { return out->getName(); }

CorruptingIndexOutput::~CorruptingIndexOutput()
{
  if (closed == false) {
    delete out;
    // NOTE: must corrupt after file is closed, because if we corrupt "inlined"
    // (as bytes are being written) the checksum sees the wrong bytes and is
    // "correct"!!
    corruptFile();
    closed = true;
  }
}

void CorruptingIndexOutput::corruptFile() 
{
  // Now corrupt the specfied byte:
  wstring newTempName;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(IndexOutput tmpOut =
  // dir.createTempOutput("tmp", "tmp", IOContext.DEFAULT); IndexInput in =
  // dir.openInput(out.getName(), IOContext.DEFAULT))
  {
    IndexOutput tmpOut =
        dir->createTempOutput(L"tmp", L"tmp", IOContext::DEFAULT);
    IndexInput in_ = dir->openInput(out->getName(), IOContext::DEFAULT);
    newTempName = tmpOut->getName();

    if (byteToCorrupt >= in_->length()) {
      throw invalid_argument(L"byteToCorrupt=" + to_wstring(byteToCorrupt) +
                             L" but file \"" + out->getName() +
                             L"\" is only length=" + to_wstring(in_->length()));
    }

    tmpOut->copyBytes(in_, byteToCorrupt);
    // Flip the 0th bit:
    tmpOut->writeByte(static_cast<char>(in_->readByte() ^ 1));
    tmpOut->copyBytes(in_, in_->length() - byteToCorrupt - 1);
  }

  // Delete original and copy corrupt version back:
  dir->deleteFile(out->getName());
  dir->copyFrom(dir, newTempName, out->getName(), IOContext::DEFAULT);
  dir->deleteFile(newTempName);
}

int64_t CorruptingIndexOutput::getFilePointer()
{
  return out->getFilePointer();
}

int64_t CorruptingIndexOutput::getChecksum() 
{
  return out->getChecksum() ^ 1;
}

wstring CorruptingIndexOutput::toString()
{
  return L"CorruptingIndexOutput(" + out + L")";
}

void CorruptingIndexOutput::writeByte(char b) 
{
  out->writeByte(b);
}

void CorruptingIndexOutput::writeBytes(std::deque<char> &b, int offset,
                                       int length) 
{
  for (int i = 0; i < length; i++) {
    writeByte(b[offset + i]);
  }
}
} // namespace org::apache::lucene::store