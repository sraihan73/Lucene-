using namespace std;

#include "TokenInfoDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/InputStreamDataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "TokenInfoFST.h"

namespace org::apache::lucene::analysis::ja::dict
{
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using IOUtils = org::apache::lucene::util::IOUtils;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
const wstring TokenInfoDictionary::FST_FILENAME_SUFFIX = L"$fst.dat";

TokenInfoDictionary::TokenInfoDictionary() 
    : BinaryDictionary()
{
  shared_ptr<InputStream> is = nullptr;
  shared_ptr<FST<int64_t>> fst = nullptr;
  bool success = false;
  try {
    is = getResource(FST_FILENAME_SUFFIX);
    is = make_shared<BufferedInputStream>(is);
    fst = make_shared<FST<int64_t>>(make_shared<InputStreamDataInput>(is),
                                      PositiveIntOutputs::getSingleton());
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
  // TODO: some way to configure?
  this->fst = make_shared<TokenInfoFST>(fst, true);
}

shared_ptr<TokenInfoFST> TokenInfoDictionary::getFST() { return fst; }

shared_ptr<TokenInfoDictionary> TokenInfoDictionary::getInstance()
{
  return SingletonHolder::INSTANCE;
}

const shared_ptr<TokenInfoDictionary>
    TokenInfoDictionary::SingletonHolder::INSTANCE;

TokenInfoDictionary::SingletonHolder::StaticConstructor::StaticConstructor()
{
  try {
    INSTANCE = make_shared<TokenInfoDictionary>();
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot load
    // TokenInfoDictionary.", ioe);
    throw runtime_error(L"Cannot load TokenInfoDictionary.");
  }
}

SingletonHolder::StaticConstructor
    TokenInfoDictionary::SingletonHolder::staticConstructor;
} // namespace org::apache::lucene::analysis::ja::dict