using namespace std;

#include "ScriptAttributeImpl.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"

namespace org::apache::lucene::analysis::icu::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using com::ibm::icu::lang::UScript;

ScriptAttributeImpl::ScriptAttributeImpl() {}

int ScriptAttributeImpl::getCode() { return code; }

void ScriptAttributeImpl::setCode(int code) { this->code = code; }

wstring ScriptAttributeImpl::getName() { return UScript::getName(code); }

wstring ScriptAttributeImpl::getShortName()
{
  return UScript::getShortName(code);
}

void ScriptAttributeImpl::clear() { code = UScript::COMMON; }

void ScriptAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<ScriptAttribute> t =
      std::static_pointer_cast<ScriptAttribute>(target);
  t->setCode(code);
}

bool ScriptAttributeImpl::equals(any other)
{
  if (shared_from_this() == other) {
    return true;
  }

  if (std::dynamic_pointer_cast<ScriptAttributeImpl>(other) != nullptr) {
    return (any_cast<std::shared_ptr<ScriptAttributeImpl>>(other))->code ==
           code;
  }

  return false;
}

int ScriptAttributeImpl::hashCode() { return code; }

void ScriptAttributeImpl::reflectWith(AttributeReflector reflector)
{
  // when wordbreaking CJK, we use the 15924 code Japanese
  // (Han+Hiragana+Katakana) to mark runs of Chinese/Japanese. our use is
  // correct (as for chinese Han is a subset), but this is just to help prevent
  // confusion.
  wstring name = code == UScript::JAPANESE ? L"Chinese/Japanese" : getName();
  reflector(ScriptAttribute::typeid, L"script", name);
}
} // namespace org::apache::lucene::analysis::icu::tokenattributes