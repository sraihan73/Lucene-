using namespace std;

#include "TestJavascriptCompiler.h"

namespace org::apache::lucene::expressions::js
{
using Expression = org::apache::lucene::expressions::Expression;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestJavascriptCompiler::testValidCompiles() 
{
  assertNotNull(JavascriptCompiler::compile(L"100"));
  assertNotNull(JavascriptCompiler::compile(L"valid0+100"));
  assertNotNull(JavascriptCompiler::compile(L"valid0+\n100"));
  assertNotNull(JavascriptCompiler::compile(L"logn(2, 20+10-5.0)"));
}

void TestJavascriptCompiler::testValidVariables() 
{
  doTestValidVariable(L"object.valid0");
  doTestValidVariable(L"object0.object1.valid1");
  doTestValidVariable(L"array0[1]");
  doTestValidVariable(L"array0[1].x");
  doTestValidVariable(L"multiarray[0][0]");
  doTestValidVariable(L"multiarray[0][0].x");
  doTestValidVariable(L"strindex['hello']");
  doTestValidVariable(L"strindex[\"hello\"]", L"strindex['hello']");
  doTestValidVariable(L"empty['']");
  doTestValidVariable(L"empty[\"\"]", L"empty['']");
  doTestValidVariable(
      L"strindex['\u304A\u65E9\u3046\u3054\u3056\u3044\u307E\u3059']");
  doTestValidVariable(
      L"strindex[\"\u304A\u65E9\u3046\u3054\u3056\u3044\u307E\u3059\"]",
      L"strindex['\u304A\u65E9\u3046\u3054\u3056\u3044\u307E\u3059']");
  doTestValidVariable(L"escapes['\\\\\\'']");
  doTestValidVariable(L"escapes[\"\\\\\\\"\"]", L"escapes['\\\\\"']");
  doTestValidVariable(L"mixed[23]['key'].sub.sub");
  doTestValidVariable(L"mixed[23]['key'].sub.sub[1]");
  doTestValidVariable(L"mixed[23]['key'].sub.sub[1].sub");
  doTestValidVariable(L"mixed[23]['key'].sub.sub[1].sub['abc']");
  doTestValidVariable(L"method.method()");
  doTestValidVariable(L"method.getMethod()");
  doTestValidVariable(L"method.METHOD()");
  doTestValidVariable(L"method['key'].method()");
  doTestValidVariable(L"method['key'].getMethod()");
  doTestValidVariable(L"method['key'].METHOD()");
  doTestValidVariable(L"method[23][\"key\"].method()",
                      L"method[23]['key'].method()");
  doTestValidVariable(L"method[23][\"key\"].getMethod()",
                      L"method[23]['key'].getMethod()");
  doTestValidVariable(L"method[23][\"key\"].METHOD()",
                      L"method[23]['key'].METHOD()");
}

void TestJavascriptCompiler::doTestValidVariable(const wstring &variable) throw(
    runtime_error)
{
  doTestValidVariable(variable, variable);
}

void TestJavascriptCompiler::doTestValidVariable(
    const wstring &variable, const wstring &output) 
{
  shared_ptr<Expression> e = JavascriptCompiler::compile(variable);
  assertNotNull(e);
  assertEquals(1, e->variables.size());
  assertEquals(output, e->variables[0]);
}

void TestJavascriptCompiler::testInvalidVariables() 
{
  doTestInvalidVariable(L"object.0invalid");
  doTestInvalidVariable(L"0.invalid");
  doTestInvalidVariable(L"object..invalid");
  doTestInvalidVariable(L".invalid");
  doTestInvalidVariable(L"negative[-1]");
  doTestInvalidVariable(L"float[1.0]");
  doTestInvalidVariable(L"missing_end['abc]");
  doTestInvalidVariable(L"missing_end[\"abc]");
  doTestInvalidVariable(L"missing_begin[abc']");
  doTestInvalidVariable(L"missing_begin[abc\"]");
  doTestInvalidVariable(L"dot_needed[1]sub");
  doTestInvalidVariable(L"dot_needed[1]sub");
  doTestInvalidVariable(L"opposite_escape['\\\"']");
  doTestInvalidVariable(L"opposite_escape[\"\\'\"]");
}

void TestJavascriptCompiler::doTestInvalidVariable(const wstring &variable)
{
  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(variable); });
}

void TestJavascriptCompiler::testInvalidLexer() 
{
  shared_ptr<ParseException> expected = expectThrows(
      ParseException::typeid, [&]() { JavascriptCompiler::compile(L"\n ."); });
  assertTrue(expected->getMessage()->contains(
      L"unexpected character '.' on line (2) position (1)"));
}

void TestJavascriptCompiler::testInvalidCompiles() 
{
  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"100 100"); });

  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"7*/-8"); });

  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"0y1234"); });

  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"500EE"); });

  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"500.5EE"); });
}

void TestJavascriptCompiler::testEmpty()
{
  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L""); });

  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"()"); });

  expectThrows(ParseException::typeid,
               [&]() { JavascriptCompiler::compile(L"   \r\n   \n \t"); });
}

void TestJavascriptCompiler::testNull() 
{
  expectThrows(NullPointerException::typeid,
               [&]() { JavascriptCompiler::compile(L""); });
}

void TestJavascriptCompiler::testWrongArity() 
{
  shared_ptr<ParseException> expected =
      expectThrows(ParseException::typeid, [&]() {
        JavascriptCompiler::compile(L"tan()");
        fail();
      });
  assertEquals(L"Invalid expression 'tan()': Expected (1) arguments for "
               L"function call (tan), but found (0).",
               expected->getMessage());
  assertEquals(expected->getErrorOffset(), 0);

  expected = expectThrows(ParseException::typeid,
                          [&]() { JavascriptCompiler::compile(L"tan(1, 1)"); });
  assertTrue(expected->getMessage()->contains(L"arguments for function call"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { JavascriptCompiler::compile(L" tan()"); });
  assertEquals(L"Invalid expression ' tan()': Expected (1) arguments for "
               L"function call (tan), but found (0).",
               expected->getMessage());
  assertEquals(expected->getErrorOffset(), 1);

  expected = expectThrows(ParseException::typeid,
                          [&]() { JavascriptCompiler::compile(L"1 + tan()"); });
  assertEquals(L"Invalid expression '1 + tan()': Expected (1) arguments for "
               L"function call (tan), but found (0).",
               expected->getMessage());
  assertEquals(expected->getErrorOffset(), 4);
}

void TestJavascriptCompiler::testVariableNormalization() 
{
  // multiple double quotes
  shared_ptr<Expression> x = JavascriptCompiler::compile(L"foo[\"a\"][\"b\"]");
  assertEquals(L"foo['a']['b']", x->variables[0]);

  // single and double in the same var
  x = JavascriptCompiler::compile(L"foo['a'][\"b\"]");
  assertEquals(L"foo['a']['b']", x->variables[0]);

  // escapes remain the same in single quoted strings
  x = JavascriptCompiler::compile(L"foo['\\\\\\'\"']");
  assertEquals(L"foo['\\\\\\'\"']", x->variables[0]);

  // single quotes are escaped
  x = JavascriptCompiler::compile(L"foo[\"'\"]");
  assertEquals(L"foo['\\'']", x->variables[0]);

  // double quotes are unescaped
  x = JavascriptCompiler::compile(L"foo[\"\\\"\"]");
  assertEquals(L"foo['\"']", x->variables[0]);

  // backslash escapes are kept the same
  x = JavascriptCompiler::compile(L"foo['\\\\'][\"\\\\\"]");
  assertEquals(L"foo['\\\\']['\\\\']", x->variables[0]);
}
} // namespace org::apache::lucene::expressions::js