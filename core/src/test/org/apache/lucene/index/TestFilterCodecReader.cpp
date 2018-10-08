using namespace std;

#include "TestFilterCodecReader.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFilterCodecReader::testDeclaredMethodsOverridden() 
{
  constexpr type_info subClass = FilterCodecReader::typeid;
  implTestDeclaredMethodsOverridden(subClass.getSuperclass(), subClass);
}

void TestFilterCodecReader::implTestDeclaredMethodsOverridden(
    type_info superClass, type_info subClass) 
{
  for (shared_ptr<Method> superClassMethod : superClass.getDeclaredMethods()) {
    constexpr int modifiers = superClassMethod->getModifiers();
    if (Modifier::isPrivate(modifiers)) {
      continue;
    }
    if (Modifier::isFinal(modifiers)) {
      continue;
    }
    if (Modifier::isStatic(modifiers)) {
      continue;
    }
    try {
      shared_ptr<Method> *const subClassMethod = subClass.getDeclaredMethod(
          superClassMethod->getName(), superClassMethod->getParameterTypes());
      assertEquals(L"getReturnType() difference",
                   superClassMethod->getReturnType(),
                   subClassMethod->getReturnType());
    } catch (const NoSuchMethodException &e) {
      fail(subClass + L" needs to override '" + superClassMethod + L"'");
    }
  }
}
} // namespace org::apache::lucene::index