using namespace std;

#include "InflectionAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../Token.h"
#include "../util/ToStringUtil.h"

namespace org::apache::lucene::analysis::ja::tokenattributes
{
using Token = org::apache::lucene::analysis::ja::Token;
using ToStringUtil = org::apache::lucene::analysis::ja::util::ToStringUtil;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

wstring InflectionAttributeImpl::getInflectionType()
{
  return token == nullptr ? L"" : token->getInflectionType();
}

wstring InflectionAttributeImpl::getInflectionForm()
{
  return token == nullptr ? L"" : token->getInflectionForm();
}

void InflectionAttributeImpl::setToken(shared_ptr<Token> token)
{
  this->token = token;
}

void InflectionAttributeImpl::clear() { token.reset(); }

void InflectionAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<InflectionAttribute> t =
      std::static_pointer_cast<InflectionAttribute>(target);
  t->setToken(token);
}

void InflectionAttributeImpl::reflectWith(AttributeReflector reflector)
{
  wstring type = getInflectionType();
  wstring typeEN =
      type == L"" ? L"" : ToStringUtil::getInflectionTypeTranslation(type);
  reflector(InflectionAttribute::typeid, L"inflectionType", type);
  reflector(InflectionAttribute::typeid, L"inflectionType (en)", typeEN);
  wstring form = getInflectionForm();
  wstring formEN =
      form == L"" ? L"" : ToStringUtil::getInflectedFormTranslation(form);
  reflector(InflectionAttribute::typeid, L"inflectionForm", form);
  reflector(InflectionAttribute::typeid, L"inflectionForm (en)", formEN);
}
} // namespace org::apache::lucene::analysis::ja::tokenattributes