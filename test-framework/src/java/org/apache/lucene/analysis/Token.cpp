using namespace std;

#include "Token.h"

namespace org::apache::lucene::analysis
{
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using PackedTokenAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::PackedTokenAttributeImpl;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

Token::Token() {}

Token::Token(shared_ptr<std::wstring> text, int start, int end)
{
  append(text);
  setOffset(start, end);
}

Token::Token(shared_ptr<std::wstring> text, int posInc, int start, int end)
{
  append(text);
  setOffset(start, end);
  setPositionIncrement(posInc);
}

int Token::getFlags() { return flags; }

void Token::setFlags(int flags) { this->flags = flags; }

shared_ptr<BytesRef> Token::getPayload() { return this->payload; }

void Token::setPayload(shared_ptr<BytesRef> payload)
{
  this->payload = payload;
}

void Token::clear()
{
  PackedTokenAttributeImpl::clear();
  flags = 0;
  payload.reset();
}

bool Token::equals(any obj)
{
  if (obj == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<Token>(obj) != nullptr) {
    shared_ptr<Token> *const other = any_cast<std::shared_ptr<Token>>(obj);
    return (flags == other->flags &&
            (payload == nullptr ? other->payload == nullptr
                                : payload->equals(other->payload)) &&
            PackedTokenAttributeImpl::equals(obj));
  } else {
    return false;
  }
}

int Token::hashCode()
{
  int code = PackedTokenAttributeImpl::hashCode();
  code = code * 31 + flags;
  if (payload != nullptr) {
    code = code * 31 + payload->hashCode();
  }
  return code;
}

shared_ptr<Token> Token::clone()
{
  shared_ptr<Token> *const t =
      std::static_pointer_cast<Token>(PackedTokenAttributeImpl::clone());
  if (payload != nullptr) {
    t->payload = BytesRef::deepCopyOf(payload);
  }
  return t;
}

void Token::reinit(shared_ptr<Token> prototype)
{
  // this is a bad hack to emulate no cloning of payload!
  prototype->copyToWithoutPayloadClone(shared_from_this());
}

void Token::copyToWithoutPayloadClone(shared_ptr<AttributeImpl> target)
{
  PackedTokenAttributeImpl::copyTo(target);
  (std::static_pointer_cast<FlagsAttribute>(target))->setFlags(flags);
  (std::static_pointer_cast<PayloadAttribute>(target))->setPayload(payload);
}

void Token::copyTo(shared_ptr<AttributeImpl> target)
{
  PackedTokenAttributeImpl::copyTo(target);
  (std::static_pointer_cast<FlagsAttribute>(target))->setFlags(flags);
  (std::static_pointer_cast<PayloadAttribute>(target))
      ->setPayload((payload == nullptr) ? nullptr
                                        : BytesRef::deepCopyOf(payload));
}

void Token::reflectWith(AttributeReflector reflector)
{
  PackedTokenAttributeImpl::reflectWith(reflector);
  reflector(FlagsAttribute::typeid, L"flags", flags);
  reflector(PayloadAttribute::typeid, L"payload", payload);
}

const shared_ptr<org::apache::lucene::util::AttributeFactory>
    Token::TOKEN_ATTRIBUTE_FACTORY =
        org::apache::lucene::util::AttributeFactory::getStaticImplementation(
            org::apache::lucene::util::AttributeFactory::
                DEFAULT_ATTRIBUTE_FACTORY,
            Token::typeid);
} // namespace org::apache::lucene::analysis