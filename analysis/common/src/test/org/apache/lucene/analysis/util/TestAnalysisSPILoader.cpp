using namespace std;

#include "TestAnalysisSPILoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/HTMLStripCharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizerFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/RemoveDuplicatesTokenFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenizerFactory.h"

namespace org::apache::lucene::analysis::util
{
using HTMLStripCharFilterFactory =
    org::apache::lucene::analysis::charfilter::HTMLStripCharFilterFactory;
using LowerCaseFilterFactory =
    org::apache::lucene::analysis::core::LowerCaseFilterFactory;
using WhitespaceTokenizerFactory =
    org::apache::lucene::analysis::core::WhitespaceTokenizerFactory;
using RemoveDuplicatesTokenFilterFactory = org::apache::lucene::analysis::
    miscellaneous::RemoveDuplicatesTokenFilterFactory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Version = org::apache::lucene::util::Version;

unordered_map<wstring, wstring> TestAnalysisSPILoader::versionArgOnly()
{
  return make_shared<HashMapAnonymousInnerClass>(shared_from_this());
}

TestAnalysisSPILoader::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<TestAnalysisSPILoader> outerInstance)
{
  this->outerInstance = outerInstance;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->put(L"luceneMatchVersion", Version::LATEST->toString());
}

void TestAnalysisSPILoader::testLookupTokenizer()
{
  assertSame(
      WhitespaceTokenizerFactory::typeid,
      TokenizerFactory::forName(L"Whitespace", versionArgOnly())->getClass());
  assertSame(
      WhitespaceTokenizerFactory::typeid,
      TokenizerFactory::forName(L"WHITESPACE", versionArgOnly())->getClass());
  assertSame(
      WhitespaceTokenizerFactory::typeid,
      TokenizerFactory::forName(L"whitespace", versionArgOnly())->getClass());
}

void TestAnalysisSPILoader::testBogusLookupTokenizer()
{
  expectThrows(invalid_argument::typeid, [&]() {
    TokenizerFactory::forName(L"sdfsdfsdfdsfsdfsdf",
                              unordered_map<wstring, wstring>());
  });

  expectThrows(invalid_argument::typeid, [&]() {
    TokenizerFactory::forName(L"!(**#$U*#$*",
                              unordered_map<wstring, wstring>());
  });
}

void TestAnalysisSPILoader::testLookupTokenizerClass()
{
  assertSame(WhitespaceTokenizerFactory::typeid,
             TokenizerFactory::lookupClass(L"Whitespace"));
  assertSame(WhitespaceTokenizerFactory::typeid,
             TokenizerFactory::lookupClass(L"WHITESPACE"));
  assertSame(WhitespaceTokenizerFactory::typeid,
             TokenizerFactory::lookupClass(L"whitespace"));
}

void TestAnalysisSPILoader::testBogusLookupTokenizerClass()
{
  expectThrows(invalid_argument::typeid,
               [&]() { TokenizerFactory::lookupClass(L"sdfsdfsdfdsfsdfsdf"); });

  expectThrows(invalid_argument::typeid,
               [&]() { TokenizerFactory::lookupClass(L"!(**#$U*#$*"); });
}

void TestAnalysisSPILoader::testAvailableTokenizers()
{
  assertTrue(TokenizerFactory::availableTokenizers()->contains(L"whitespace"));
}

void TestAnalysisSPILoader::testLookupTokenFilter()
{
  assertSame(
      LowerCaseFilterFactory::typeid,
      TokenFilterFactory::forName(L"Lowercase", versionArgOnly())->getClass());
  assertSame(
      LowerCaseFilterFactory::typeid,
      TokenFilterFactory::forName(L"LOWERCASE", versionArgOnly())->getClass());
  assertSame(
      LowerCaseFilterFactory::typeid,
      TokenFilterFactory::forName(L"lowercase", versionArgOnly())->getClass());

  assertSame(RemoveDuplicatesTokenFilterFactory::typeid,
             TokenFilterFactory::forName(L"RemoveDuplicates", versionArgOnly())
                 ->getClass());
  assertSame(RemoveDuplicatesTokenFilterFactory::typeid,
             TokenFilterFactory::forName(L"REMOVEDUPLICATES", versionArgOnly())
                 ->getClass());
  assertSame(RemoveDuplicatesTokenFilterFactory::typeid,
             TokenFilterFactory::forName(L"removeduplicates", versionArgOnly())
                 ->getClass());
}

void TestAnalysisSPILoader::testBogusLookupTokenFilter()
{
  expectThrows(invalid_argument::typeid, [&]() {
    TokenFilterFactory::forName(L"sdfsdfsdfdsfsdfsdf",
                                unordered_map<wstring, wstring>());
  });

  expectThrows(invalid_argument::typeid, [&]() {
    TokenFilterFactory::forName(L"!(**#$U*#$*",
                                unordered_map<wstring, wstring>());
  });
}

void TestAnalysisSPILoader::testLookupTokenFilterClass()
{
  assertSame(LowerCaseFilterFactory::typeid,
             TokenFilterFactory::lookupClass(L"Lowercase"));
  assertSame(LowerCaseFilterFactory::typeid,
             TokenFilterFactory::lookupClass(L"LOWERCASE"));
  assertSame(LowerCaseFilterFactory::typeid,
             TokenFilterFactory::lookupClass(L"lowercase"));

  assertSame(RemoveDuplicatesTokenFilterFactory::typeid,
             TokenFilterFactory::lookupClass(L"RemoveDuplicates"));
  assertSame(RemoveDuplicatesTokenFilterFactory::typeid,
             TokenFilterFactory::lookupClass(L"REMOVEDUPLICATES"));
  assertSame(RemoveDuplicatesTokenFilterFactory::typeid,
             TokenFilterFactory::lookupClass(L"removeduplicates"));
}

void TestAnalysisSPILoader::testBogusLookupTokenFilterClass()
{
  expectThrows(invalid_argument::typeid, [&]() {
    TokenFilterFactory::lookupClass(L"sdfsdfsdfdsfsdfsdf");
  });

  expectThrows(invalid_argument::typeid,
               [&]() { TokenFilterFactory::lookupClass(L"!(**#$U*#$*"); });
}

void TestAnalysisSPILoader::testAvailableTokenFilters()
{
  assertTrue(
      TokenFilterFactory::availableTokenFilters()->contains(L"lowercase"));
  assertTrue(TokenFilterFactory::availableTokenFilters()->contains(
      L"removeduplicates"));
}

void TestAnalysisSPILoader::testLookupCharFilter()
{
  assertSame(
      HTMLStripCharFilterFactory::typeid,
      CharFilterFactory::forName(L"HTMLStrip", versionArgOnly())->getClass());
  assertSame(
      HTMLStripCharFilterFactory::typeid,
      CharFilterFactory::forName(L"HTMLSTRIP", versionArgOnly())->getClass());
  assertSame(
      HTMLStripCharFilterFactory::typeid,
      CharFilterFactory::forName(L"htmlstrip", versionArgOnly())->getClass());
}

void TestAnalysisSPILoader::testBogusLookupCharFilter()
{
  expectThrows(invalid_argument::typeid, [&]() {
    CharFilterFactory::forName(L"sdfsdfsdfdsfsdfsdf",
                               unordered_map<wstring, wstring>());
  });

  expectThrows(invalid_argument::typeid, [&]() {
    CharFilterFactory::forName(L"!(**#$U*#$*",
                               unordered_map<wstring, wstring>());
  });
}

void TestAnalysisSPILoader::testLookupCharFilterClass()
{
  assertSame(HTMLStripCharFilterFactory::typeid,
             CharFilterFactory::lookupClass(L"HTMLStrip"));
  assertSame(HTMLStripCharFilterFactory::typeid,
             CharFilterFactory::lookupClass(L"HTMLSTRIP"));
  assertSame(HTMLStripCharFilterFactory::typeid,
             CharFilterFactory::lookupClass(L"htmlstrip"));
}

void TestAnalysisSPILoader::testBogusLookupCharFilterClass()
{
  expectThrows(invalid_argument::typeid, [&]() {
    CharFilterFactory::lookupClass(L"sdfsdfsdfdsfsdfsdf");
  });

  expectThrows(invalid_argument::typeid,
               [&]() { CharFilterFactory::lookupClass(L"!(**#$U*#$*"); });
}

void TestAnalysisSPILoader::testAvailableCharFilters()
{
  assertTrue(CharFilterFactory::availableCharFilters()->contains(L"htmlstrip"));
}
} // namespace org::apache::lucene::analysis::util