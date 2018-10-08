using namespace std;

#include "KeywordAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

KeywordAttributeImpl::KeywordAttributeImpl() {}

void KeywordAttributeImpl::clear() { keyword = false; }

void KeywordAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<KeywordAttribute> attr =
      std::static_pointer_cast<KeywordAttribute>(target);
  attr->setKeyword(keyword);
}

int KeywordAttributeImpl::hashCode() { return keyword ? 31 : 37; }

bool KeywordAttributeImpl::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<KeywordAttributeImpl> *const other =
      any_cast<std::shared_ptr<KeywordAttributeImpl>>(obj);
  return keyword == other->keyword;
}

bool KeywordAttributeImpl::isKeyword() { return keyword; }

void KeywordAttributeImpl::setKeyword(bool isKeyword) { keyword = isKeyword; }

void KeywordAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(KeywordAttribute::typeid, L"keyword", keyword);
}
} // namespace org::apache::lucene::analysis::tokenattributes