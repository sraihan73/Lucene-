using namespace std;

#include "TestJavascriptOperations.h"

namespace org::apache::lucene::expressions::js
{
using Expression = org::apache::lucene::expressions::Expression;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestJavascriptOperations::assertEvaluatesTo(
    const wstring &expression, int64_t expected) 
{
  shared_ptr<Expression> evaluator = JavascriptCompiler::compile(expression);
  int64_t actual = static_cast<int64_t>(evaluator->evaluate(nullptr));
  assertEquals(expected, actual);
}

void TestJavascriptOperations::testNegationOperation() 
{
  assertEvaluatesTo(L"-1", -1);
  assertEvaluatesTo(L"--1", 1);
  assertEvaluatesTo(L"-(-1)", 1);
  assertEvaluatesTo(L"-0", 0);
  assertEvaluatesTo(L"--0", 0);
}

void TestJavascriptOperations::testAddOperation() 
{
  assertEvaluatesTo(L"1+1", 2);
  assertEvaluatesTo(L"1+0.5+0.5", 2);
  assertEvaluatesTo(L"5+10", 15);
  assertEvaluatesTo(L"1+1+2", 4);
  assertEvaluatesTo(L"(1+1)+2", 4);
  assertEvaluatesTo(L"1+(1+2)", 4);
  assertEvaluatesTo(L"0+1", 1);
  assertEvaluatesTo(L"1+0", 1);
  assertEvaluatesTo(L"0+0", 0);
}

void TestJavascriptOperations::testSubtractOperation() 
{
  assertEvaluatesTo(L"1-1", 0);
  assertEvaluatesTo(L"5-10", -5);
  assertEvaluatesTo(L"1-0.5-0.5", 0);
  assertEvaluatesTo(L"1-1-2", -2);
  assertEvaluatesTo(L"(1-1)-2", -2);
  assertEvaluatesTo(L"1-(1-2)", 2);
  assertEvaluatesTo(L"0-1", -1);
  assertEvaluatesTo(L"1-0", 1);
  assertEvaluatesTo(L"0-0", 0);
}

void TestJavascriptOperations::testMultiplyOperation() 
{
  assertEvaluatesTo(L"1*1", 1);
  assertEvaluatesTo(L"5*10", 50);
  assertEvaluatesTo(L"50*0.1", 5);
  assertEvaluatesTo(L"1*1*2", 2);
  assertEvaluatesTo(L"(1*1)*2", 2);
  assertEvaluatesTo(L"1*(1*2)", 2);
  assertEvaluatesTo(L"10*0", 0);
  assertEvaluatesTo(L"0*0", 0);
}

void TestJavascriptOperations::testDivisionOperation() 
{
  assertEvaluatesTo(L"1*1", 1);
  assertEvaluatesTo(L"10/5", 2);
  assertEvaluatesTo(L"10/0.5", 20);
  assertEvaluatesTo(L"10/5/2", 1);
  assertEvaluatesTo(L"(27/9)/3", 1);
  assertEvaluatesTo(L"27/(9/3)", 9);
  assertEvaluatesTo(L"1/0", 9223372036854775807LL);
}

void TestJavascriptOperations::testModuloOperation() 
{
  assertEvaluatesTo(L"1%1", 0);
  assertEvaluatesTo(L"10%3", 1);
  assertEvaluatesTo(L"10%3%2", 1);
  assertEvaluatesTo(L"(27%10)%4", 3);
  assertEvaluatesTo(L"27%(9%5)", 3);
}

void TestJavascriptOperations::testLessThanOperation() 
{
  assertEvaluatesTo(L"1 < 1", 0);
  assertEvaluatesTo(L"2 < 1", 0);
  assertEvaluatesTo(L"1 < 2", 1);
  assertEvaluatesTo(L"2 < 1 < 3", 1);
  assertEvaluatesTo(L"2 < (1 < 3)", 0);
  assertEvaluatesTo(L"(2 < 1) < 1", 1);
  assertEvaluatesTo(L"-1 < -2", 0);
  assertEvaluatesTo(L"-1 < 0", 1);
}

void TestJavascriptOperations::testLessThanEqualsOperation() throw(
    runtime_error)
{
  assertEvaluatesTo(L"1 <= 1", 1);
  assertEvaluatesTo(L"2 <= 1", 0);
  assertEvaluatesTo(L"1 <= 2", 1);
  assertEvaluatesTo(L"1 <= 1 <= 0", 0);
  assertEvaluatesTo(L"-1 <= -1", 1);
  assertEvaluatesTo(L"-1 <= 0", 1);
  assertEvaluatesTo(L"-1 <= -2", 0);
  assertEvaluatesTo(L"-1 <= 0", 1);
}

void TestJavascriptOperations::testGreaterThanOperation() 
{
  assertEvaluatesTo(L"1 > 1", 0);
  assertEvaluatesTo(L"2 > 1", 1);
  assertEvaluatesTo(L"1 > 2", 0);
  assertEvaluatesTo(L"2 > 1 > 3", 0);
  assertEvaluatesTo(L"2 > (1 > 3)", 1);
  assertEvaluatesTo(L"(2 > 1) > 1", 0);
  assertEvaluatesTo(L"-1 > -2", 1);
  assertEvaluatesTo(L"-1 > 0", 0);
}

void TestJavascriptOperations::testGreaterThanEqualsOperation() throw(
    runtime_error)
{
  assertEvaluatesTo(L"1 >= 1", 1);
  assertEvaluatesTo(L"2 >= 1", 1);
  assertEvaluatesTo(L"1 >= 2", 0);
  assertEvaluatesTo(L"1 >= 1 >= 0", 1);
  assertEvaluatesTo(L"-1 >= -1", 1);
  assertEvaluatesTo(L"-1 >= 0", 0);
  assertEvaluatesTo(L"-1 >= -2", 1);
  assertEvaluatesTo(L"-1 >= 0", 0);
}

void TestJavascriptOperations::testEqualsOperation() 
{
  assertEvaluatesTo(L"1 == 1", 1);
  assertEvaluatesTo(L"0 == 0", 1);
  assertEvaluatesTo(L"-1 == -1", 1);
  assertEvaluatesTo(L"1.1 == 1.1", 1);
  assertEvaluatesTo(L"0.9 == 0.9", 1);
  assertEvaluatesTo(L"-0 == 0", 1);
  assertEvaluatesTo(L"0 == 1", 0);
  assertEvaluatesTo(L"1 == 2", 0);
  assertEvaluatesTo(L"-1 == 1", 0);
  assertEvaluatesTo(L"-1 == 0", 0);
  assertEvaluatesTo(L"-2 == 1", 0);
  assertEvaluatesTo(L"-2 == -1", 0);
}

void TestJavascriptOperations::testNotEqualsOperation() 
{
  assertEvaluatesTo(L"1 != 1", 0);
  assertEvaluatesTo(L"0 != 0", 0);
  assertEvaluatesTo(L"-1 != -1", 0);
  assertEvaluatesTo(L"1.1 != 1.1", 0);
  assertEvaluatesTo(L"0.9 != 0.9", 0);
  assertEvaluatesTo(L"-0 != 0", 0);
  assertEvaluatesTo(L"0 != 1", 1);
  assertEvaluatesTo(L"1 != 2", 1);
  assertEvaluatesTo(L"-1 != 1", 1);
  assertEvaluatesTo(L"-1 != 0", 1);
  assertEvaluatesTo(L"-2 != 1", 1);
  assertEvaluatesTo(L"-2 != -1", 1);
}

void TestJavascriptOperations::testBoolNotOperation() 
{
  assertEvaluatesTo(L"!1", 0);
  assertEvaluatesTo(L"!!1", 1);
  assertEvaluatesTo(L"!0", 1);
  assertEvaluatesTo(L"!!0", 0);
  assertEvaluatesTo(L"!-1", 0);
  assertEvaluatesTo(L"!2", 0);
  assertEvaluatesTo(L"!-2", 0);
}

void TestJavascriptOperations::testBoolAndOperation() 
{
  assertEvaluatesTo(L"1 && 1", 1);
  assertEvaluatesTo(L"1 && 0", 0);
  assertEvaluatesTo(L"0 && 1", 0);
  assertEvaluatesTo(L"0 && 0", 0);
  assertEvaluatesTo(L"-1 && -1", 1);
  assertEvaluatesTo(L"-1 && 0", 0);
  assertEvaluatesTo(L"0 && -1", 0);
  assertEvaluatesTo(L"-0 && -0", 0);
}

void TestJavascriptOperations::testBoolOrOperation() 
{
  assertEvaluatesTo(L"1 || 1", 1);
  assertEvaluatesTo(L"1 || 0", 1);
  assertEvaluatesTo(L"0 || 1", 1);
  assertEvaluatesTo(L"0 || 0", 0);
  assertEvaluatesTo(L"-1 || -1", 1);
  assertEvaluatesTo(L"-1 || 0", 1);
  assertEvaluatesTo(L"0 || -1", 1);
  assertEvaluatesTo(L"-0 || -0", 0);
}

void TestJavascriptOperations::testConditionalOperation() 
{
  assertEvaluatesTo(L"1 ? 2 : 3", 2);
  assertEvaluatesTo(L"-1 ? 2 : 3", 2);
  assertEvaluatesTo(L"0 ? 2 : 3", 3);
  assertEvaluatesTo(L"1 ? 2 ? 3 : 4 : 5", 3);
  assertEvaluatesTo(L"0 ? 2 ? 3 : 4 : 5", 5);
  assertEvaluatesTo(L"1 ? 0 ? 3 : 4 : 5", 4);
  assertEvaluatesTo(L"1 ? 2 : 3 ? 4 : 5", 2);
  assertEvaluatesTo(L"0 ? 2 : 3 ? 4 : 5", 4);
  assertEvaluatesTo(L"0 ? 2 : 0 ? 4 : 5", 5);
  assertEvaluatesTo(L"(1 ? 1 : 0) ? 3 : 4", 3);
  assertEvaluatesTo(L"(0 ? 1 : 0) ? 3 : 4", 4);
}

void TestJavascriptOperations::testBitShiftLeft() 
{
  assertEvaluatesTo(L"1 << 1", 2);
  assertEvaluatesTo(L"2 << 1", 4);
  assertEvaluatesTo(L"-1 << 31", -2147483648);
  assertEvaluatesTo(L"3 << 5", 96);
  assertEvaluatesTo(L"-5 << 3", -40);
  assertEvaluatesTo(L"4195 << 7", 536960);
  assertEvaluatesTo(L"4195 << 66", 16780);
  assertEvaluatesTo(L"4195 << 6", 268480);
  assertEvaluatesTo(L"4195 << 70", 268480);
  assertEvaluatesTo(L"-4195 << 70", -268480);
  assertEvaluatesTo(L"-15 << 62", 4611686018427387904LL);
}

void TestJavascriptOperations::testBitShiftRight() 
{
  assertEvaluatesTo(L"1 >> 1", 0);
  assertEvaluatesTo(L"2 >> 1", 1);
  assertEvaluatesTo(L"-1 >> 5", -1);
  assertEvaluatesTo(L"-2 >> 30", -1);
  assertEvaluatesTo(L"-5 >> 1", -3);
  assertEvaluatesTo(L"536960 >> 7", 4195);
  assertEvaluatesTo(L"16780 >> 66", 4195);
  assertEvaluatesTo(L"268480 >> 6", 4195);
  assertEvaluatesTo(L"268480 >> 70", 4195);
  assertEvaluatesTo(L"-268480 >> 70", -4195);
  assertEvaluatesTo(L"-2147483646 >> 1", -1073741823);
}

void TestJavascriptOperations::testBitShiftRightUnsigned() 
{
  assertEvaluatesTo(L"1 >>> 1", 0);
  assertEvaluatesTo(L"2 >>> 1", 1);
  assertEvaluatesTo(L"-1 >>> 37", 134217727);
  assertEvaluatesTo(L"-2 >>> 62", 3);
  assertEvaluatesTo(L"-5 >>> 33", 2147483647);
  assertEvaluatesTo(L"536960 >>> 7", 4195);
  assertEvaluatesTo(L"16780 >>> 66", 4195);
  assertEvaluatesTo(L"268480 >>> 6", 4195);
  assertEvaluatesTo(L"268480 >>> 70", 4195);
  assertEvaluatesTo(L"-268480 >>> 102", 67108863);
  assertEvaluatesTo(L"2147483648 >>> 1", 1073741824);
}

void TestJavascriptOperations::testBitwiseAnd() 
{
  assertEvaluatesTo(L"4 & 4", 4);
  assertEvaluatesTo(L"3 & 2", 2);
  assertEvaluatesTo(L"7 & 3", 3);
  assertEvaluatesTo(L"-1 & -1", -1);
  assertEvaluatesTo(L"-1 & 25", 25);
  assertEvaluatesTo(L"3 & 7", 3);
  assertEvaluatesTo(L"0 & 1", 0);
  assertEvaluatesTo(L"1 & 0", 0);
}

void TestJavascriptOperations::testBitwiseOr() 
{
  assertEvaluatesTo(L"4 | 4", 4);
  assertEvaluatesTo(L"5 | 2", 7);
  assertEvaluatesTo(L"7 | 3", 7);
  assertEvaluatesTo(L"-1 | -5", -1);
  assertEvaluatesTo(L"-1 | 25", -1);
  assertEvaluatesTo(L"-100 | 15", -97);
  assertEvaluatesTo(L"0 | 1", 1);
  assertEvaluatesTo(L"1 | 0", 1);
}

void TestJavascriptOperations::testBitwiseXor() 
{
  assertEvaluatesTo(L"4 ^ 4", 0);
  assertEvaluatesTo(L"5 ^ 2", 7);
  assertEvaluatesTo(L"15 ^ 3", 12);
  assertEvaluatesTo(L"-1 ^ -5", 4);
  assertEvaluatesTo(L"-1 ^ 25", -26);
  assertEvaluatesTo(L"-100 ^ 15", -109);
  assertEvaluatesTo(L"0 ^ 1", 1);
  assertEvaluatesTo(L"1 ^ 0", 1);
  assertEvaluatesTo(L"0 ^ 0", 0);
}

void TestJavascriptOperations::testBitwiseNot() 
{
  assertEvaluatesTo(L"~-5", 4);
  assertEvaluatesTo(L"~25", -26);
  assertEvaluatesTo(L"~0", -1);
  assertEvaluatesTo(L"~-1", 0);
}

void TestJavascriptOperations::testDecimalConst() 
{
  assertEvaluatesTo(L"0", 0);
  assertEvaluatesTo(L"1", 1);
  assertEvaluatesTo(L"123456789", 123456789);
  assertEvaluatesTo(L"5.6E2", 560);
  assertEvaluatesTo(L"5.6E+2", 560);
  assertEvaluatesTo(L"500E-2", 5);
}

void TestJavascriptOperations::testHexConst() 
{
  assertEvaluatesTo(L"0x0", 0);
  assertEvaluatesTo(L"0x1", 1);
  assertEvaluatesTo(L"0xF", 15);
  assertEvaluatesTo(L"0x1234ABCDEF", 78193085935LL);
  assertEvaluatesTo(L"1 << 0x1", 1 << 0x1);
  assertEvaluatesTo(L"1 << 0xA", 1 << 0xA);
  assertEvaluatesTo(L"0x1 << 2", 0x1 << 2);
  assertEvaluatesTo(L"0xA << 2", 0xA << 2);
}

void TestJavascriptOperations::testHexConst2() 
{
  assertEvaluatesTo(L"0X0", 0);
  assertEvaluatesTo(L"0X1", 1);
  assertEvaluatesTo(L"0XF", 15);
  assertEvaluatesTo(L"0X1234ABCDEF", 78193085935LL);
}

void TestJavascriptOperations::testOctalConst() 
{
  assertEvaluatesTo(L"00", 0);
  assertEvaluatesTo(L"01", 1);
  assertEvaluatesTo(L"010", 8);
  assertEvaluatesTo(L"0123456777", 21913087);
  assertEvaluatesTo(L"1 << 01", 1 << 01);
  assertEvaluatesTo(L"1 << 010", 1 << 010);
  assertEvaluatesTo(L"01 << 2", 01 << 2);
  assertEvaluatesTo(L"010 << 2", 010 << 2);
}
} // namespace org::apache::lucene::expressions::js