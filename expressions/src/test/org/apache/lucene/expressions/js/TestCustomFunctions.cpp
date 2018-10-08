using namespace std;

#include "TestCustomFunctions.h"

namespace org::apache::lucene::expressions::js
{
using Expression = org::apache::lucene::expressions::Expression;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::objectweb::asm_::ClassWriter;
using org::objectweb::asm_::Opcodes;
using org::objectweb::asm_::Type;
using org::objectweb::asm_::commons::GeneratorAdapter;
double TestCustomFunctions::DELTA = 0.0000001;

void TestCustomFunctions::testEmpty() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      Collections::emptyMap();
  shared_ptr<ParseException> expected =
      expectThrows(ParseException::typeid, [&]() {
        JavascriptCompiler::compile(L"sqrt(20)", functions,
                                    getClass().getClassLoader());
      });
  assertEquals(
      L"Invalid expression 'sqrt(20)': Unrecognized function call (sqrt).",
      expected->getMessage());
  assertEquals(expected->getErrorOffset(), 0);
}

void TestCustomFunctions::testDefaultList() 
{
  unordered_map<wstring, std::shared_ptr<Method>> &functions =
      JavascriptCompiler::DEFAULT_FUNCTIONS;
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      L"sqrt(20)", functions, getClass().getClassLoader());
  assertEquals(sqrt(20), expr->evaluate(nullptr), DELTA);
}

double TestCustomFunctions::zeroArgMethod() { return 5; }

void TestCustomFunctions::testNoArgMethod() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo", getClass().getMethod(L"zeroArgMethod"));
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      L"foo()", functions, getClass().getClassLoader());
  assertEquals(5, expr->evaluate(nullptr), DELTA);
}

double TestCustomFunctions::oneArgMethod(double arg1) { return 3 + arg1; }

void TestCustomFunctions::testOneArgMethod() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo",
                    getClass().getMethod(L"oneArgMethod", double ::typeid));
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      L"foo(3)", functions, getClass().getClassLoader());
  assertEquals(6, expr->evaluate(nullptr), DELTA);
}

double TestCustomFunctions::threeArgMethod(double arg1, double arg2,
                                           double arg3)
{
  return arg1 + arg2 + arg3;
}

void TestCustomFunctions::testThreeArgMethod() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo",
                    getClass().getMethod(L"threeArgMethod", double ::typeid,
                                         double ::typeid, double ::typeid));
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      L"foo(3, 4, 5)", functions, getClass().getClassLoader());
  assertEquals(12, expr->evaluate(nullptr), DELTA);
}

void TestCustomFunctions::testTwoMethods() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo", getClass().getMethod(L"zeroArgMethod"));
  functions.emplace(L"bar",
                    getClass().getMethod(L"oneArgMethod", double ::typeid));
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      L"foo() + bar(3)", functions, getClass().getClassLoader());
  assertEquals(11, expr->evaluate(nullptr), DELTA);
}

void TestCustomFunctions::testInvalidVariableMethods()
{
  shared_ptr<ParseException> expected =
      expectThrows(ParseException::typeid,
                   [&]() { JavascriptCompiler::compile(L"method()"); });
  assertEquals(
      L"Invalid expression 'method()': Unrecognized function call (method).",
      expected->getMessage());
  assertEquals(0, expected->getErrorOffset());

  expected = expectThrows(ParseException::typeid, [&]() {
    JavascriptCompiler::compile(L"method.method(1)");
  });
  assertEquals(L"Invalid expression 'method.method(1)': Unrecognized function "
               L"call (method.method).",
               expected->getMessage());
  assertEquals(0, expected->getErrorOffset());

  expected = expectThrows(ParseException::typeid, [&]() {
    JavascriptCompiler::compile(L"1 + method()");
  });
  assertEquals(L"Invalid expression '1 + method()': Unrecognized function call "
               L"(method).",
               expected->getMessage());
  assertEquals(4, expected->getErrorOffset());
}

wstring TestCustomFunctions::bogusReturnType() { return L"bogus!"; }

void TestCustomFunctions::testWrongReturnType() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo", getClass().getMethod(L"bogusReturnType"));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"foo()", functions,
                                getClass().getClassLoader());
  });
  assertTrue(expected.what()->contains(L"does not return a double"));
}

double TestCustomFunctions::bogusParameterType(const wstring &s) { return 0; }

void TestCustomFunctions::testWrongParameterType() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(
      L"foo", getClass().getMethod(L"bogusParameterType", wstring::typeid));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"foo(2)", functions,
                                getClass().getClassLoader());
  });
  assertTrue(expected.what()->contains(L"must take only double parameters"));
}

double TestCustomFunctions::nonStaticMethod() { return 0; }

void TestCustomFunctions::testWrongNotStatic() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo", getClass().getMethod(L"nonStaticMethod"));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"foo()", functions,
                                getClass().getClassLoader());
  });
  assertTrue(expected.what()->contains(L"is not static"));
}

double TestCustomFunctions::nonPublicMethod() { return 0; }

void TestCustomFunctions::testWrongNotPublic() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo", getClass().getDeclaredMethod(L"nonPublicMethod"));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"foo()", functions,
                                getClass().getClassLoader());
  });
  assertTrue(expected.what()->contains(L"not public"));
}

double TestCustomFunctions::NestedNotPublic::method() { return 0; }

void TestCustomFunctions::testWrongNestedNotPublic() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo", NestedNotPublic::typeid->getMethod(L"method"));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"foo()", functions,
                                getClass().getClassLoader());
  });
  assertTrue(expected.what()->contains(L"not public"));
}

TestCustomFunctions::Loader::Loader(shared_ptr<ClassLoader> parent)
    : ClassLoader(parent)
{
}

type_info TestCustomFunctions::Loader::createFakeClass()
{
  wstring className = TestCustomFunctions::typeid->getName() + L"$Foo";
  shared_ptr<ClassWriter> classWriter = make_shared<ClassWriter>(
      ClassWriter::COMPUTE_FRAMES | ClassWriter::COMPUTE_MAXS);
  classWriter->visit(Opcodes::V1_5,
                     ACC_PUBLIC | ACC_SUPER | ACC_FINAL | ACC_SYNTHETIC,
                     StringHelper::replace(className, L'.', L'/'), nullptr,
                     Type::getInternalName(any::typeid), nullptr);

  shared_ptr<org::objectweb::asm_::commons::Method> m =
      org::objectweb::asm_::commons::Method::getMethod(L"void <init>()");
  shared_ptr<GeneratorAdapter> constructor = make_shared<GeneratorAdapter>(
      ACC_PRIVATE | ACC_SYNTHETIC, m, nullptr, nullptr, classWriter);
  constructor->loadThis();
  constructor->loadArgs();
  constructor->invokeConstructor(Type::getType(any::typeid), m);
  constructor->returnValue();
  constructor->endMethod();

  shared_ptr<GeneratorAdapter> gen = make_shared<GeneratorAdapter>(
      ACC_STATIC | ACC_PUBLIC | ACC_SYNTHETIC,
      org::objectweb::asm_::commons::Method::getMethod(L"double bar()"),
      nullptr, nullptr, classWriter);
  gen->push(2.0);
  gen->returnValue();
  gen->endMethod();

  std::deque<char> bc = classWriter->toByteArray();
  return defineClass(className, bc, 0, bc.size());
}

void TestCustomFunctions::testClassLoader() 
{
  shared_ptr<ClassLoader> thisLoader = getClass().getClassLoader();
  shared_ptr<Loader> childLoader = make_shared<Loader>(thisLoader);
  type_info fooClass = childLoader->createFakeClass();

  shared_ptr<Method> barMethod = fooClass.getMethod(L"bar");
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      Collections::singletonMap(L"bar", barMethod);
  assertNotSame(thisLoader, fooClass.getClassLoader());
  assertNotSame(thisLoader, barMethod->getDeclaringClass().getClassLoader());

  // this should pass:
  shared_ptr<Expression> expr =
      JavascriptCompiler::compile(L"bar()", functions, childLoader);
  assertEquals(2.0, expr->evaluate(nullptr), DELTA);

  // use our classloader, not the foreign one, which should fail!
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"bar()", functions, thisLoader);
  });
  assertTrue(
      expected.what()->contains(L"is not declared by a class which is "
                                L"accessible by the given parent ClassLoader"));

  // mix foreign and default functions
  unordered_map<wstring, std::shared_ptr<Method>> mixedFunctions =
      unordered_map<wstring, std::shared_ptr<Method>>(
          JavascriptCompiler::DEFAULT_FUNCTIONS);
  mixedFunctions.putAll(functions);
  expr = JavascriptCompiler::compile(L"bar()", mixedFunctions, childLoader);
  assertEquals(2.0, expr->evaluate(nullptr), DELTA);
  expr = JavascriptCompiler::compile(L"sqrt(20)", mixedFunctions, childLoader);
  assertEquals(sqrt(20), expr->evaluate(nullptr), DELTA);

  // use our classloader, not the foreign one, which should fail!
  expected = expectThrows(invalid_argument::typeid, [&]() {
    JavascriptCompiler::compile(L"bar()", mixedFunctions, thisLoader);
  });
  assertTrue(
      expected.what()->contains(L"is not declared by a class which is "
                                L"accessible by the given parent ClassLoader"));
}

wstring TestCustomFunctions::MESSAGE = L"This should not happen but it happens";

double TestCustomFunctions::StaticThrowingException::method()
{
  throw make_shared<ArithmeticException>(MESSAGE);
}

void TestCustomFunctions::testThrowingException() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo",
                    StaticThrowingException::typeid->getMethod(L"method"));
  wstring source = L"3 * foo() / 5";
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      source, functions, getClass().getClassLoader());
  shared_ptr<ArithmeticException> expected = expectThrows(
      ArithmeticException::typeid, [&]() { expr->evaluate(nullptr); });
  assertEquals(MESSAGE, expected->getMessage());
  shared_ptr<StringWriter> sw = make_shared<StringWriter>();
  shared_ptr<PrintWriter> pw = make_shared<PrintWriter>(sw);
  expected->printStackTrace(pw);
  pw->flush();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(sw->toString()->contains(
      L"JavascriptCompiler$CompiledExpression.evaluate(" + source + L")"));
}

void TestCustomFunctions::testNamespaces() 
{
  unordered_map<wstring, std::shared_ptr<Method>> functions =
      unordered_map<wstring, std::shared_ptr<Method>>();
  functions.emplace(L"foo.bar", getClass().getMethod(L"zeroArgMethod"));
  wstring source = L"foo.bar()";
  shared_ptr<Expression> expr = JavascriptCompiler::compile(
      source, functions, getClass().getClassLoader());
  assertEquals(5, expr->evaluate(nullptr), DELTA);
}
} // namespace org::apache::lucene::expressions::js