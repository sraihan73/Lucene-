using namespace std;

#include "TestJavascriptFunction.h"

namespace org::apache::lucene::expressions::js
{
using Expression = org::apache::lucene::expressions::Expression;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
double TestJavascriptFunction::DELTA = 0.0000001;

void TestJavascriptFunction::assertEvaluatesTo(
    const wstring &expression, double expected) 
{
  shared_ptr<Expression> evaluator = JavascriptCompiler::compile(expression);
  double actual = evaluator->evaluate(nullptr);
  assertEquals(expected, actual, DELTA);
}

void TestJavascriptFunction::testAbsMethod() 
{
  assertEvaluatesTo(L"abs(0)", 0);
  assertEvaluatesTo(L"abs(119)", 119);
  assertEvaluatesTo(L"abs(119)", 119);
  assertEvaluatesTo(L"abs(1)", 1);
  assertEvaluatesTo(L"abs(-1)", 1);
}

void TestJavascriptFunction::testAcosMethod() 
{
  assertEvaluatesTo(L"acos(-1)", M_PI);
  assertEvaluatesTo(L"acos(-0.8660254)", M_PI * 5 / 6);
  assertEvaluatesTo(L"acos(-0.7071068)", M_PI * 3 / 4);
  assertEvaluatesTo(L"acos(-0.5)", M_PI * 2 / 3);
  assertEvaluatesTo(L"acos(0)", M_PI / 2);
  assertEvaluatesTo(L"acos(0.5)", M_PI / 3);
  assertEvaluatesTo(L"acos(0.7071068)", M_PI / 4);
  assertEvaluatesTo(L"acos(0.8660254)", M_PI / 6);
  assertEvaluatesTo(L"acos(1)", 0);
}

void TestJavascriptFunction::testAcoshMethod() 
{
  assertEvaluatesTo(L"acosh(1)", 0);
  assertEvaluatesTo(L"acosh(2.5)", 1.5667992369724109);
  assertEvaluatesTo(L"acosh(1234567.89)", 14.719378760739708);
}

void TestJavascriptFunction::testAsinMethod() 
{
  assertEvaluatesTo(L"asin(-1)", -M_PI / 2);
  assertEvaluatesTo(L"asin(-0.8660254)", -M_PI / 3);
  assertEvaluatesTo(L"asin(-0.7071068)", -M_PI / 4);
  assertEvaluatesTo(L"asin(-0.5)", -M_PI / 6);
  assertEvaluatesTo(L"asin(0)", 0);
  assertEvaluatesTo(L"asin(0.5)", M_PI / 6);
  assertEvaluatesTo(L"asin(0.7071068)", M_PI / 4);
  assertEvaluatesTo(L"asin(0.8660254)", M_PI / 3);
  assertEvaluatesTo(L"asin(1)", M_PI / 2);
}

void TestJavascriptFunction::testAsinhMethod() 
{
  assertEvaluatesTo(L"asinh(-1234567.89)", -14.719378760740035);
  assertEvaluatesTo(L"asinh(-2.5)", -1.6472311463710958);
  assertEvaluatesTo(L"asinh(-1)", -0.8813735870195429);
  assertEvaluatesTo(L"asinh(0)", 0);
  assertEvaluatesTo(L"asinh(1)", 0.8813735870195429);
  assertEvaluatesTo(L"asinh(2.5)", 1.6472311463710958);
  assertEvaluatesTo(L"asinh(1234567.89)", 14.719378760740035);
}

void TestJavascriptFunction::testAtanMethod() 
{
  assertEvaluatesTo(L"atan(-1.732050808)", -M_PI / 3);
  assertEvaluatesTo(L"atan(-1)", -M_PI / 4);
  assertEvaluatesTo(L"atan(-0.577350269)", -M_PI / 6);
  assertEvaluatesTo(L"atan(0)", 0);
  assertEvaluatesTo(L"atan(0.577350269)", M_PI / 6);
  assertEvaluatesTo(L"atan(1)", M_PI / 4);
  assertEvaluatesTo(L"atan(1.732050808)", M_PI / 3);
}

void TestJavascriptFunction::testAtan2Method() 
{
  assertEvaluatesTo(L"atan2(+0,+0)", +0.0);
  assertEvaluatesTo(L"atan2(+0,-0)", +M_PI);
  assertEvaluatesTo(L"atan2(-0,+0)", -0.0);
  assertEvaluatesTo(L"atan2(-0,-0)", -M_PI);
  assertEvaluatesTo(L"atan2(2,2)", M_PI / 4);
  assertEvaluatesTo(L"atan2(-2,2)", -M_PI / 4);
  assertEvaluatesTo(L"atan2(2,-2)", M_PI * 3 / 4);
  assertEvaluatesTo(L"atan2(-2,-2)", -M_PI * 3 / 4);
}

void TestJavascriptFunction::testAtanhMethod() 
{
  assertEvaluatesTo(L"atanh(-1)", -numeric_limits<double>::infinity());
  assertEvaluatesTo(L"atanh(-0.5)", -0.5493061443340549);
  assertEvaluatesTo(L"atanh(0)", 0);
  assertEvaluatesTo(L"atanh(0.5)", 0.5493061443340549);
  assertEvaluatesTo(L"atanh(1)", numeric_limits<double>::infinity());
}

void TestJavascriptFunction::testCeilMethod() 
{
  assertEvaluatesTo(L"ceil(0)", 0);
  assertEvaluatesTo(L"ceil(0.1)", 1);
  assertEvaluatesTo(L"ceil(0.9)", 1);
  assertEvaluatesTo(L"ceil(25.2)", 26);
  assertEvaluatesTo(L"ceil(-0.1)", 0);
  assertEvaluatesTo(L"ceil(-0.9)", 0);
  assertEvaluatesTo(L"ceil(-1.1)", -1);
}

void TestJavascriptFunction::testCosMethod() 
{
  assertEvaluatesTo(L"cos(0)", 1);
  assertEvaluatesTo(L"cos(" + M_PI / 2 + L")", 0);
  assertEvaluatesTo(L"cos(" + -M_PI / 2 + L")", 0);
  assertEvaluatesTo(L"cos(" + M_PI / 4 + L")", 0.7071068);
  assertEvaluatesTo(L"cos(" + -M_PI / 4 + L")", 0.7071068);
  assertEvaluatesTo(L"cos(" + M_PI * 2 / 3 + L")", -0.5);
  assertEvaluatesTo(L"cos(" + -M_PI * 2 / 3 + L")", -0.5);
  assertEvaluatesTo(L"cos(" + M_PI / 6 + L")", 0.8660254);
  assertEvaluatesTo(L"cos(" + -M_PI / 6 + L")", 0.8660254);
}

void TestJavascriptFunction::testCoshMethod() 
{
  assertEvaluatesTo(L"cosh(0)", 1);
  assertEvaluatesTo(L"cosh(-1)", 1.5430806348152437);
  assertEvaluatesTo(L"cosh(1)", 1.5430806348152437);
  assertEvaluatesTo(L"cosh(-0.5)", 1.1276259652063807);
  assertEvaluatesTo(L"cosh(0.5)", 1.1276259652063807);
  assertEvaluatesTo(L"cosh(-12.3456789)", 114982.09728671524);
  assertEvaluatesTo(L"cosh(12.3456789)", 114982.09728671524);
}

void TestJavascriptFunction::testExpMethod() 
{
  assertEvaluatesTo(L"exp(0)", 1);
  assertEvaluatesTo(L"exp(-1)", 0.36787944117);
  assertEvaluatesTo(L"exp(1)", 2.71828182846);
  assertEvaluatesTo(L"exp(-0.5)", 0.60653065971);
  assertEvaluatesTo(L"exp(0.5)", 1.6487212707);
  assertEvaluatesTo(L"exp(-12.3456789)", 0.0000043485);
  assertEvaluatesTo(L"exp(12.3456789)", 229964.194569);
}

void TestJavascriptFunction::testFloorMethod() 
{
  assertEvaluatesTo(L"floor(0)", 0);
  assertEvaluatesTo(L"floor(0.1)", 0);
  assertEvaluatesTo(L"floor(0.9)", 0);
  assertEvaluatesTo(L"floor(25.2)", 25);
  assertEvaluatesTo(L"floor(-0.1)", -1);
  assertEvaluatesTo(L"floor(-0.9)", -1);
  assertEvaluatesTo(L"floor(-1.1)", -2);
}

void TestJavascriptFunction::testHaversinMethod() 
{
  assertEvaluatesTo(L"haversin(40.7143528,-74.0059731,40.759011,-73.9844722)",
                    5.285885589128259);
}

void TestJavascriptFunction::testLnMethod() 
{
  assertEvaluatesTo(L"ln(0)", -numeric_limits<double>::infinity());
  assertEvaluatesTo(L"ln(" + M_E + L")", 1);
  assertEvaluatesTo(L"ln(-1)", NAN);
  assertEvaluatesTo(L"ln(1)", 0);
  assertEvaluatesTo(L"ln(0.5)", -0.69314718056);
  assertEvaluatesTo(L"ln(12.3456789)", 2.51330611521);
}

void TestJavascriptFunction::testLog10Method() 
{
  assertEvaluatesTo(L"log10(0)", -numeric_limits<double>::infinity());
  assertEvaluatesTo(L"log10(1)", 0);
  assertEvaluatesTo(L"log10(-1)", NAN);
  assertEvaluatesTo(L"log10(0.5)", -0.3010299956639812);
  assertEvaluatesTo(L"log10(12.3456789)", 1.0915149771692705);
}

void TestJavascriptFunction::testLognMethod() 
{
  assertEvaluatesTo(L"logn(2, 0)", -numeric_limits<double>::infinity());
  assertEvaluatesTo(L"logn(2, 1)", 0);
  assertEvaluatesTo(L"logn(2, -1)", NAN);
  assertEvaluatesTo(L"logn(2, 0.5)", -1);
  assertEvaluatesTo(L"logn(2, 12.3456789)", 3.6259342686489378);
  assertEvaluatesTo(L"logn(2.5, 0)", -numeric_limits<double>::infinity());
  assertEvaluatesTo(L"logn(2.5, 1)", 0);
  assertEvaluatesTo(L"logn(2.5, -1)", NAN);
  assertEvaluatesTo(L"logn(2.5, 0.5)", -0.75647079736603);
  assertEvaluatesTo(L"logn(2.5, 12.3456789)", 2.7429133874016745);
}

void TestJavascriptFunction::testMaxMethod() 
{
  assertEvaluatesTo(L"max(0, 0)", 0);
  assertEvaluatesTo(L"max(1, 0)", 1);
  assertEvaluatesTo(L"max(0, -1)", 0);
  assertEvaluatesTo(L"max(-1, 0)", 0);
  assertEvaluatesTo(L"max(25, 23)", 25);
}

void TestJavascriptFunction::testMinMethod() 
{
  assertEvaluatesTo(L"min(0, 0)", 0);
  assertEvaluatesTo(L"min(1, 0)", 0);
  assertEvaluatesTo(L"min(0, -1)", -1);
  assertEvaluatesTo(L"min(-1, 0)", -1);
  assertEvaluatesTo(L"min(25, 23)", 23);
}

void TestJavascriptFunction::testPowMethod() 
{
  assertEvaluatesTo(L"pow(0, 0)", 1);
  assertEvaluatesTo(L"pow(0.1, 2)", 0.01);
  assertEvaluatesTo(L"pow(0.9, -1)", 1.1111111111111112);
  assertEvaluatesTo(L"pow(2.2, -2.5)", 0.13929749224447147);
  assertEvaluatesTo(L"pow(5, 3)", 125);
  assertEvaluatesTo(L"pow(-0.9, 5)", -0.59049);
  assertEvaluatesTo(L"pow(-1.1, 2)", 1.21);
}

void TestJavascriptFunction::testSinMethod() 
{
  assertEvaluatesTo(L"sin(0)", 0);
  assertEvaluatesTo(L"sin(" + M_PI / 2 + L")", 1);
  assertEvaluatesTo(L"sin(" + -M_PI / 2 + L")", -1);
  assertEvaluatesTo(L"sin(" + M_PI / 4 + L")", 0.7071068);
  assertEvaluatesTo(L"sin(" + -M_PI / 4 + L")", -0.7071068);
  assertEvaluatesTo(L"sin(" + M_PI * 2 / 3 + L")", 0.8660254);
  assertEvaluatesTo(L"sin(" + -M_PI * 2 / 3 + L")", -0.8660254);
  assertEvaluatesTo(L"sin(" + M_PI / 6 + L")", 0.5);
  assertEvaluatesTo(L"sin(" + -M_PI / 6 + L")", -0.5);
}

void TestJavascriptFunction::testSinhMethod() 
{
  assertEvaluatesTo(L"sinh(0)", 0);
  assertEvaluatesTo(L"sinh(-1)", -1.1752011936438014);
  assertEvaluatesTo(L"sinh(1)", 1.1752011936438014);
  assertEvaluatesTo(L"sinh(-0.5)", -0.52109530549);
  assertEvaluatesTo(L"sinh(0.5)", 0.52109530549);
  assertEvaluatesTo(L"sinh(-12.3456789)", -114982.09728236674);
  assertEvaluatesTo(L"sinh(12.3456789)", 114982.09728236674);
}

void TestJavascriptFunction::testSqrtMethod() 
{
  assertEvaluatesTo(L"sqrt(0)", 0);
  assertEvaluatesTo(L"sqrt(-1)", NAN);
  assertEvaluatesTo(L"sqrt(0.49)", 0.7);
  assertEvaluatesTo(L"sqrt(49)", 7);
}

void TestJavascriptFunction::testTanMethod() 
{
  assertEvaluatesTo(L"tan(0)", 0);
  assertEvaluatesTo(L"tan(-1)", -1.55740772465);
  assertEvaluatesTo(L"tan(1)", 1.55740772465);
  assertEvaluatesTo(L"tan(-0.5)", -0.54630248984);
  assertEvaluatesTo(L"tan(0.5)", 0.54630248984);
  assertEvaluatesTo(L"tan(-1.3)", -3.60210244797);
  assertEvaluatesTo(L"tan(1.3)", 3.60210244797);
}

void TestJavascriptFunction::testTanhMethod() 
{
  assertEvaluatesTo(L"tanh(0)", 0);
  assertEvaluatesTo(L"tanh(-1)", -0.76159415595);
  assertEvaluatesTo(L"tanh(1)", 0.76159415595);
  assertEvaluatesTo(L"tanh(-0.5)", -0.46211715726);
  assertEvaluatesTo(L"tanh(0.5)", 0.46211715726);
  assertEvaluatesTo(L"tanh(-12.3456789)", -0.99999999996);
  assertEvaluatesTo(L"tanh(12.3456789)", 0.99999999996);
}
} // namespace org::apache::lucene::expressions::js