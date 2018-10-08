using namespace std;

#include "Among.h"
#include "SnowballProgram.h"

namespace org::tartarus::snowball
{

Among::Among(const wstring &s, int substring_i, int result,
             const wstring &methodname,
             shared_ptr<MethodHandles::Lookup> methodobject)
    : s_size(s.length()), s(s.toCharArray()), substring_i(substring_i),
      result(result)
{
  if (methodname.isEmpty()) {
    this->method.reset();
  } else {
    constexpr type_info clazz =
        methodobject->lookupClass().asSubclass(SnowballProgram::typeid);
    try {
      this->method = methodobject
                         ->findVirtual(clazz, methodname,
                                       MethodType::methodType(bool ::typeid))
                         .asType(MethodType::methodType(
                             bool ::typeid, SnowballProgram::typeid));
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (NoSuchMethodException | IllegalAccessException e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new
      // RuntimeException(std::wstring.format(java.util.Locale.ENGLISH, "Snowball
      // program '%s' is broken, cannot access method: bool %s()",
      // clazz.getSimpleName(), methodname), e);
      throw runtime_error(wstring::format(Locale::ENGLISH,
                                          L"Snowball program '%s' is broken, "
                                          L"cannot access method: bool %s()",
                                          clazz.name(), methodname));
    }
  }
}
} // namespace org::tartarus::snowball