using namespace std;

#include "TestFilterWeight.h"

namespace org::apache::lucene::search
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeclaredMethodsOverridden() throws
// Exception
void TestFilterWeight::testDeclaredMethodsOverridden() 
{
  constexpr type_info subClass = FilterWeight::typeid;
  implTestDeclaredMethodsOverridden(subClass.getSuperclass(), subClass);
}

void TestFilterWeight::implTestDeclaredMethodsOverridden(
    type_info superClass, type_info subClass) 
{
  for (shared_ptr<Method> superClassMethod : superClass.getDeclaredMethods()) {
    constexpr int modifiers = superClassMethod->getModifiers();
    if (Modifier::isFinal(modifiers)) {
      continue;
    }
    if (Modifier::isStatic(modifiers)) {
      continue;
    }
    if (Arrays::asList(L"bulkScorer", L"scorerSupplier")
            ->contains(superClassMethod->getName())) {
      try {
        shared_ptr<Method> *const subClassMethod = subClass.getDeclaredMethod(
            superClassMethod->getName(), superClassMethod->getParameterTypes());
        fail(subClass + L" must not override\n'" + superClassMethod + L"'" +
             L" but it does override\n'" + subClassMethod + L"'");
      } catch (const NoSuchMethodException &e) {
        /* FilterWeight must not override the bulkScorer method
         * since as of July 2016 not all deriving classes use the
         * {code}return in.bulkScorer(content);{code}
         * implementation that FilterWeight.bulkScorer would use.
         */
        continue;
      }
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
} // namespace org::apache::lucene::search