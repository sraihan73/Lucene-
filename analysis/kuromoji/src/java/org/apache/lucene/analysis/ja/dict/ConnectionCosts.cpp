using namespace std;

#include "ConnectionCosts.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/InputStreamDataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "BinaryDictionary.h"

namespace org::apache::lucene::analysis::ja::dict
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataInput = org::apache::lucene::store::DataInput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring ConnectionCosts::FILENAME_SUFFIX = L".dat";
const wstring ConnectionCosts::HEADER = L"kuromoji_cc";

ConnectionCosts::ConnectionCosts() 
{
  shared_ptr<InputStream> is = nullptr;
  std::deque<std::deque<short>> costs;
  bool success = false;
  try {
    is = BinaryDictionary::getClassResource(getClass(), FILENAME_SUFFIX);
    is = make_shared<BufferedInputStream>(is);
    shared_ptr<DataInput> *const in_ = make_shared<InputStreamDataInput>(is);
    CodecUtil::checkHeader(in_, HEADER, VERSION, VERSION);
    int forwardSize = in_->readVInt();
    int backwardSize = in_->readVInt();
    // C++ NOTE: The following call to the 'RectangularVectors' helper class
    // reproduces the rectangular array initialization that is automatic in Java:
    // ORIGINAL LINE: costs = new short[backwardSize][forwardSize];
    costs = RectangularVectors::ReturnRectangularShortVector(backwardSize,
                                                             forwardSize);
    int accum = 0;
    for (int j = 0; j < costs.size(); j++) {
      const std::deque<short> a = costs[j];
      for (int i = 0; i < a.size(); i++) {
        accum += in_->readZInt();
        a[i] = static_cast<short>(accum);
      }
    }
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

  this->costs = costs;
}

int ConnectionCosts::get(int forwardId, int backwardId)
{
  return costs[backwardId][forwardId];
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
} // namespace org::apache::lucene::analysis::ja::dict