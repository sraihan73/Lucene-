using namespace std;

#include "DateRecognizerFilterFactoryTest.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/DateRecognizerFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

void DateRecognizerFilterFactoryTest::testBadLanguageTagThrowsException()
{
  expectThrows(runtime_error::typeid, [&]() {
    const unordered_map<wstring, wstring> args =
        unordered_map<wstring, wstring>();
    args.put(DateRecognizerFilterFactory::LOCALE, L"en_US");
    make_shared<DateRecognizerFilterFactory>(args);
  });
}

void DateRecognizerFilterFactoryTest::testGoodLocaleParsesWell()
{
  const unordered_map<wstring, wstring> args =
      unordered_map<wstring, wstring>();
  args.emplace(DateRecognizerFilterFactory::LOCALE, L"en-US");
  make_shared<DateRecognizerFilterFactory>(args);
}
} // namespace org::apache::lucene::analysis::miscellaneous