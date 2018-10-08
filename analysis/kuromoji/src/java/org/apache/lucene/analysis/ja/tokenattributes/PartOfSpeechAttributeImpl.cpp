using namespace std;

#include "PartOfSpeechAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../Token.h"
#include "../util/ToStringUtil.h"

namespace org::apache::lucene::analysis::ja::tokenattributes
{
using Token = org::apache::lucene::analysis::ja::Token;
using ToStringUtil = org::apache::lucene::analysis::ja::util::ToStringUtil;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

wstring PartOfSpeechAttributeImpl::getPartOfSpeech()
{
  return token == nullptr ? L"" : token->getPartOfSpeech();
}

void PartOfSpeechAttributeImpl::setToken(shared_ptr<Token> token)
{
  this->token = token;
}

void PartOfSpeechAttributeImpl::clear() { token.reset(); }

void PartOfSpeechAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<PartOfSpeechAttribute> t =
      std::static_pointer_cast<PartOfSpeechAttribute>(target);
  t->setToken(token);
}

void PartOfSpeechAttributeImpl::reflectWith(AttributeReflector reflector)
{
  wstring partOfSpeech = getPartOfSpeech();
  wstring partOfSpeechEN =
      partOfSpeech == L"" ? L"" : ToStringUtil::getPOSTranslation(partOfSpeech);
  reflector(PartOfSpeechAttribute::typeid, L"partOfSpeech", partOfSpeech);
  reflector(PartOfSpeechAttribute::typeid, L"partOfSpeech (en)",
            partOfSpeechEN);
}
} // namespace org::apache::lucene::analysis::ja::tokenattributes