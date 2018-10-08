using namespace std;

#include "TestProtectedTermFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ProtectedTermFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestProtectedTermFilterFactory::testInform() 
{
  shared_ptr<ProtectedTermFilterFactory> factory =
      std::static_pointer_cast<ProtectedTermFilterFactory>(tokenFilterFactory(
          L"ProtectedTerm", {L"protected", L"protected-1.txt", L"ignoreCase",
                             L"true", L"wrappedFilters", L"lowercase"}));
  shared_ptr<CharArraySet> protectedTerms = factory->getProtectedTerms();
  assertTrue(L"protectedTerms is null and it shouldn't be",
             protectedTerms != nullptr);
  assertTrue(L"protectedTerms Size: " + to_wstring(protectedTerms->size()) +
                 L" is not: " + to_wstring(2),
             protectedTerms->size() == 2);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory =
      std::static_pointer_cast<ProtectedTermFilterFactory>(tokenFilterFactory(
          L"ProtectedTerm",
          {L"protected", L"protected-1.txt, protected-2.txt", L"ignoreCase",
           L"true", L"wrappedFilters", L"lowercase"}));
  protectedTerms = factory->getProtectedTerms();
  assertTrue(L"protectedTerms is null and it shouldn't be",
             protectedTerms != nullptr);
  assertTrue(L"protectedTerms Size: " + to_wstring(protectedTerms->size()) +
                 L" is not: " + to_wstring(4),
             protectedTerms->size() == 4);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  // defaults
  factory = std::static_pointer_cast<ProtectedTermFilterFactory>(
      tokenFilterFactory(L"ProtectedTerm", {L"protected", L"protected-1.txt"}));
  assertEquals(false, factory->isIgnoreCase());
}

void TestProtectedTermFilterFactory::testBasic() 
{
  wstring str = L"Foo Clara Bar David";
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(str);
  stream =
      tokenFilterFactory(L"ProtectedTerm",
                         {L"ignoreCase", L"true", L"protected",
                          L"protected-1.txt", L"wrappedFilters", L"lowercase"})
          ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Foo", L"clara", L"Bar", L"david"});
}

void TestProtectedTermFilterFactory::
    testMultipleWrappedFiltersWithParams() 
{
  wstring str = L"Foo Clara Bar David";
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(str);
  stream = tokenFilterFactory(L"ProtectedTerm",
                              {L"ignoreCase", L"true", L"protected",
                               L"protected-1.txt", L"wrappedFilters",
                               L"lowercase, truncate", L"truncate.prefixLength",
                               L"2"})
               ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"Foo", L"cl", L"Bar", L"da"});
}

void TestProtectedTermFilterFactory::
    testMultipleSameNamedFiltersWithParams() 
{
  wstring str = L"Foo Clara Bar David";
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(str);
  stream =
      tokenFilterFactory(
          L"ProtectedTerm",
          {L"ignoreCase", L"true", L"protected", L"protected-1.txt",
           L"wrappedFilters", L"truncate-A, reversestring, truncate-B",
           L"truncate-A.prefixLength", L"3", L"truncate-B.prefixLength", L"2"})
          ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"Foo", L"al", L"Bar", L"va"});

  // same-named wrapped filters, one with an ID and another without
  stream = whitespaceMockTokenizer(str);
  stream =
      tokenFilterFactory(
          L"ProtectedTerm",
          {L"ignoreCase", L"true", L"protected", L"protected-1.txt",
           L"wrappedFilters", L"truncate, reversestring, truncate-A",
           L"truncate.prefixLength", L"3", L"truncate-A.prefixLength", L"2"})
          ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"Foo", L"al", L"Bar", L"va"});

  // Case-insensitive wrapped "filter-id"
  stream = whitespaceMockTokenizer(str);
  stream =
      tokenFilterFactory(
          L"ProtectedTerm",
          {L"ignoreCase", L"true", L"protected", L"protected-1.txt",
           L"wrappedFilters", L"TRUNCATE-a, reversestring, truncate-b",
           L"truncate-A.prefixLength", L"3", L"TRUNCATE-B.prefixLength", L"2"})
          ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"Foo", L"al", L"Bar", L"va"});
}

void TestProtectedTermFilterFactory::testBogusArguments() 
{
  invalid_argument exception = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ProtectedTerm", {L"protected", L"protected-1.txt",
                                          L"bogusArg", L"bogusValue"});
  });
  assertTrue(exception.what()->contains(L"Unknown parameters"));

  // same-named wrapped filters
  exception = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ProtectedTerm",
                       {L"protected", L"protected-1.txt", L"wrappedFilters",
                        L"truncate, truncate"});
  });
  assertTrue(exception.what()->contains(L"wrappedFilters contains duplicate"));

  // case-insensitive same-named wrapped filters
  exception = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ProtectedTerm",
                       {L"protected", L"protected-1.txt", L"wrappedFilters",
                        L"TRUNCATE, truncate"});
  });
  assertTrue(exception.what()->contains(L"wrappedFilters contains duplicate"));

  // case-insensitive same-named wrapped filter IDs
  exception = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ProtectedTerm",
                       {L"protected", L"protected-1.txt", L"wrappedFilters",
                        L"truncate-ABC, truncate-abc"});
  });
  assertTrue(exception.what()->contains(L"wrappedFilters contains duplicate"));

  // mismatched wrapped filter and associated args
  exception = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ProtectedTerm",
                       {L"protected", L"protected-1.txt", L"wrappedFilters",
                        L"truncate-A, reversestring, truncate-B",
                        L"truncate.prefixLength", L"3",
                        L"truncate-A.prefixLength", L"2"});
  });
  assertTrue(exception.what()->contains(
      L"Unknown parameters: {truncate.prefixLength=3}"));

  // missing required arg(s) for wrapped filter
  wstring str = L"Foo Clara Bar David";
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(str);
  exception = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ProtectedTerm", {L"protected", L"protected-1.txt",
                                          L"wrappedFilters", L"length"})
        ->create(stream);
  });
  assertTrue(
      exception.what()->contains(L"Configuration Error: missing parameter"));
}
} // namespace org::apache::lucene::analysis::miscellaneous