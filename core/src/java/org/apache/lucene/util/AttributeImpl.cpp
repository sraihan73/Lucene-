using namespace std;

#include "AttributeImpl.h"

namespace org::apache::lucene::util
{

void AttributeImpl::end() { clear(); }

wstring AttributeImpl::reflectAsString(bool const prependAttClass)
{
  shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>();
  reflectWith([&](attClass, key, value) {
    if (buffer->length() > 0) {
      buffer->append(L',');
    }
    if (prependAttClass) {
      buffer->append(attClass::getName())->append(L'#');
    }
    buffer->append(key)->append(L'=')->append((value == nullptr) ? L"null"
                                                                 : value);
  });
  return buffer->toString();
}

shared_ptr<AttributeImpl> AttributeImpl::clone()
{
  shared_ptr<AttributeImpl> clone = nullptr;
  try {
    clone = std::static_pointer_cast<AttributeImpl>(__super::clone());
  } catch (const CloneNotSupportedException &e) {
    throw runtime_error(e); // shouldn't happen
  }
  return clone;
}
} // namespace org::apache::lucene::util