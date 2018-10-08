using namespace std;

#include "ReadingAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../Token.h"

namespace org::apache::lucene::analysis::ko::tokenattributes
{
using Token = org::apache::lucene::analysis::ko::Token;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

wstring ReadingAttributeImpl::getReading()
{
  return token == nullptr ? L"" : token->getReading();
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
  reflector(ReadingAttribute::typeid, L"reading", getReading());
}
} // namespace org::apache::lucene::analysis::ko::tokenattributes