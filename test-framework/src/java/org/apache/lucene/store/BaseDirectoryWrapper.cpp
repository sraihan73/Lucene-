using namespace std;

#include "BaseDirectoryWrapper.h"

namespace org::apache::lucene::store
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using TestUtil = org::apache::lucene::util::TestUtil;

BaseDirectoryWrapper::BaseDirectoryWrapper(shared_ptr<Directory> delegate_)
    : FilterDirectory(delegate_)
{
}

BaseDirectoryWrapper::~BaseDirectoryWrapper()
{
  if (isOpen_) {
    isOpen_ = false;
    if (checkIndexOnClose && DirectoryReader::indexExists(shared_from_this())) {
      TestUtil::checkIndex(shared_from_this(), crossCheckTermVectorsOnClose);
    }
  }
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
}

bool BaseDirectoryWrapper::isOpen() { return isOpen_; }

void BaseDirectoryWrapper::setCheckIndexOnClose(bool value)
{
  this->checkIndexOnClose = value;
}

bool BaseDirectoryWrapper::getCheckIndexOnClose() { return checkIndexOnClose; }

void BaseDirectoryWrapper::setCrossCheckTermVectorsOnClose(bool value)
{
  this->crossCheckTermVectorsOnClose = value;
}

bool BaseDirectoryWrapper::getCrossCheckTermVectorsOnClose()
{
  return crossCheckTermVectorsOnClose;
}
} // namespace org::apache::lucene::store