using namespace std;

#include "ReadingAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../Token.h"
#include "../util/ToStringUtil.h"

namespace org::apache::lucene::analysis::ja::tokenattributes
{
using Token = org::apache::lucene::analysis::ja::Token;
using ToStringUtil = org::apache::lucene::analysis::ja::util::ToStringUtil;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

wstring ReadingAttributeImpl::getReading()
{
  return token == nullptr ? L"" : token->getReading();
}

wstring ReadingAttributeImpl::getPronunciation()
{
  return token == nullptr ? L"" : token->getPronunciation();
}

void ReadingAttributeImpl::setToken(shared_ptr<Token> token)
{
  this->token = token;
}

void ReadingAttributeImpl::clear() { token.reset(); }

void ReadingAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<ReadingAttribute> t =
      std::static_pointer_cast<ReadingAttribute>(target);
  t->setToken(token);
}

void ReadingAttributeImpl::reflectWith(AttributeReflector reflector)
{
  wstring reading = getReading();
  wstring readingEN =
      reading == L"" ? L"" : ToStringUtil::getRomanization(reading);
  wstring pronunciation = getPronunciation();
  wstring pronunciationEN =
      pronunciation == L"" ? L"" : ToStringUtil::getRomanization(pronunciation);
  reflector(ReadingAttribute::typeid, L"reading", reading);
  reflector(ReadingAttribute::typeid, L"reading (en)", readingEN);
  reflector(ReadingAttribute::typeid, L"pronunciation", pronunciation);
  reflector(ReadingAttribute::typeid, L"pronunciation (en)", pronunciationEN);
}
} // namespace org::apache::lucene::analysis::ja::tokenattributes