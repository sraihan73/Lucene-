using namespace std;

#include "TestCSVUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/util/CSVUtil.h"

namespace org::apache::lucene::analysis::ja
{
using CSVUtil = org::apache::lucene::analysis::ja::util::CSVUtil;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCSVUtil::testQuoteEscapeQuotes() 
{
  const wstring input =
      L"\"Let It Be\" is a song and album by the The Beatles.";
  const wstring expectedOutput = StringHelper::replace(input, L"\"", L"\"\"");
  implTestQuoteEscape(input, expectedOutput);
}

void TestCSVUtil::testQuoteEscapeComma() 
{
  const wstring input = L"To be, or not to be ...";
  const wstring expectedOutput =
      StringHelper::toString(L'"') + input + StringHelper::toString(L'"');
  implTestQuoteEscape(input, expectedOutput);
}

void TestCSVUtil::testQuoteEscapeQuotesAndComma() 
{
  const wstring input = L"\"To be, or not to be ...\" is a well-known phrase "
                        L"from Shakespeare's Hamlet.";
  const wstring expectedOutput =
      L'"' + StringHelper::replace(input, L"\"", L"\"\"") + L'"';
  implTestQuoteEscape(input, expectedOutput);
}

void TestCSVUtil::implTestQuoteEscape(
    const wstring &input, const wstring &expectedOutput) 
{
  const wstring actualOutput = CSVUtil::quoteEscape(input);
  assertEquals(expectedOutput, actualOutput);
}
} // namespace org::apache::lucene::analysis::ja