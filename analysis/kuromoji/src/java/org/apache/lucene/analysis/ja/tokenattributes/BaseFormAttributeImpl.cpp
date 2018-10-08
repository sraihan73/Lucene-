using namespace std;

#include "BaseFormAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../Token.h"

namespace org::apache::lucene::analysis::ja::tokenattributes
{
using Token = org::apache::lucene::analysis::ja::Token;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

wstring BaseFormAttributeImpl::getBaseForm()
{
  return token == nullptr ? L"" : token->getBaseForm();
}

void BaseFormAttributeImpl::setToken(shared_ptr<Token> token)
{
  this->token = token;
}

void BaseFormAttributeImpl::clear() { token.reset(); }

void BaseFormAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<BaseFormAttribute> t =
      std::static_pointer_cast<BaseFormAttribute>(target);
  t->setToken(token);
}

void BaseFormAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(BaseFormAttribute::typeid, L"baseForm", getBaseForm());
}
} // namespace org::apache::lucene::analysis::ja::tokenattributes