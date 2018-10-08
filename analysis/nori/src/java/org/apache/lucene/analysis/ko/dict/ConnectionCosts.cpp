using namespace std;

#include "ConnectionCosts.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/InputStreamDataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "BinaryDictionary.h"

namespace org::apache::lucene::analysis::ko::dict
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataInput = org::apache::lucene::store::DataInput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring ConnectionCosts::FILENAME_SUFFIX = L".dat";
const wstring ConnectionCosts::HEADER = L"ko_cc";

ConnectionCosts::ConnectionCosts() 
{
  shared_ptr<InputStream> is = nullptr;
  shared_ptr<ByteBuffer> buffer = nullptr;
  bool success = false;
  try {
    is = BinaryDictionary::getClassResource(getClass(), FILENAME_SUFFIX);
    is = make_shared<BufferedInputStream>(is);
    shared_ptr<DataInput> *const in_ = make_shared<InputStreamDataInput>(is);
    CodecUtil::checkHeader(in_, HEADER, VERSION, VERSION);
    this->forwardSize = in_->readVInt();
    int backwardSize = in_->readVInt();
    int size = forwardSize * backwardSize;

    // copy the matrix into a direct byte buffer
    shared_ptr<ByteBuffer> *const tmpBuffer =
        ByteBuffer::allocateDirect(size * 2);
    int accum = 0;
    for (int j = 0; j < backwardSize; j++) {
      for (int i = 0; i < forwardSize; i++) {
        accum += in_->readZInt();
        tmpBuffer->putShort(static_cast<short>(accum));
      }
    }
    buffer = tmpBuffer->asReadOnlyBuffer();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({is});
    } else {
      IOUtils::closeWhileHandlingException({is});
    }
  }
  this->buffer = buffer;
}

int ConnectionCosts::get(int forwardId, int backwardId)
{
  // map_obj 2d matrix into a single dimension short array
  int offset = (backwardId * forwardSize + forwardId) * 2;
  return buffer->getShort(offset);
}

shared_ptr<ConnectionCosts> ConnectionCosts::getInstance()
{
  return SingletonHolder::INSTANCE;
}

const shared_ptr<ConnectionCosts> ConnectionCosts::SingletonHolder::INSTANCE;

ConnectionCosts::SingletonHolder::StaticConstructor::StaticConstructor()
{
  try {
    INSTANCE = make_shared<ConnectionCosts>();
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot load ConnectionCosts.",
    // ioe);
    throw runtime_error(L"Cannot load ConnectionCosts.");
  }
}

SingletonHolder::StaticConstructor
    ConnectionCosts::SingletonHolder::staticConstructor;
} // namespace org::apache::lucene::analysis::ko::dict