using namespace std;

#include "PartOfSpeechAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../Token.h"
#include "../dict/Dictionary.h"

namespace org::apache::lucene::analysis::ko::tokenattributes
{
using Type = org::apache::lucene::analysis::ko::POS::Type;
using Tag = org::apache::lucene::analysis::ko::POS::Tag;
using Token = org::apache::lucene::analysis::ko::Token;
using Morpheme = org::apache::lucene::analysis::ko::dict::Dictionary::Morpheme;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

Type PartOfSpeechAttributeImpl::getPOSType()
{
  return token == nullptr ? nullptr : token->getPOSType();
}

Tag PartOfSpeechAttributeImpl::getLeftPOS()
{
  return token == nullptr ? nullptr : token->getLeftPOS();
}

Tag PartOfSpeechAttributeImpl::getRightPOS()
{
  return token == nullptr ? nullptr : token->getRightPOS();
}

std::deque<std::shared_ptr<Morpheme>> PartOfSpeechAttributeImpl::getMorphemes()
{
  return token == nullptr ? nullptr : token->getMorphemes();
}

void PartOfSpeechAttributeImpl::setToken(shared_ptr<Token> token)
{
  this->token = token;
}

void PartOfSpeechAttributeImpl::clear() { token.reset(); }

void PartOfSpeechAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(PartOfSpeechAttribute::typeid, L"posType", getPOSType().name());
  Tag leftPOS = getLeftPOS();
  reflector(PartOfSpeechAttribute::typeid, L"leftPOS",
            leftPOS.name() + L"(" + leftPOS.description() + L")");
  Tag rightPOS = getRightPOS();
  reflector(PartOfSpeechAttribute::typeid, L"rightPOS",
            rightPOS.name() + L"(" + rightPOS.description() + L")");
  reflector(PartOfSpeechAttribute::typeid, L"morphemes",
            displayMorphemes(getMorphemes()));
}

wstring PartOfSpeechAttributeImpl::displayMorphemes(
    std::deque<std::shared_ptr<Morpheme>> &morphemes)
{
  if (morphemes.empty()) {
    return L"";
  }
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (auto morpheme : morphemes) {
    if (builder->length() > 0) {
      builder->append(L"+");
    }
    builder->append(morpheme->surfaceForm + L"/" + morpheme->posTag.name() +
                    L"(" + morpheme->posTag.description() + L")");
  }
  return builder->toString();
}

void PartOfSpeechAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<PartOfSpeechAttribute> t =
      std::static_pointer_cast<PartOfSpeechAttribute>(target);
  t->setToken(token);
}
} // namespace org::apache::lucene::analysis::ko::tokenattributes