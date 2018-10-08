using namespace std;

#include "TestCapitalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestCapitalizationFilterFactory::testCapitalization() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"kiTTEN");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization", {L"keep", L"and the it BIG",
                                                  L"onlyFirstWord", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Kitten"});
}

void TestCapitalizationFilterFactory::testCapitalization2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"and");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"And"});
}

void TestCapitalizationFilterFactory::testCapitalization3() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"AnD");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"And"});
}

void TestCapitalizationFilterFactory::testCapitalization4() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"AnD");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"false"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"And"});
}

void TestCapitalizationFilterFactory::testCapitalization5() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"big");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Big"});
}

void TestCapitalizationFilterFactory::testCapitalization6() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"BIG");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"BIG"});
}

void TestCapitalizationFilterFactory::testCapitalization7() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Hello thEre my Name is Ryan");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Hello there my name is ryan"});
}

void TestCapitalizationFilterFactory::testCapitalization8() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Hello thEre my Name is Ryan");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"false", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"Hello", L"There", L"My", L"Name", L"Is", L"Ryan"});
}

void TestCapitalizationFilterFactory::testCapitalization9() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Hello thEre my Name is Ryan");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"false", L"minWordLength", L"3",
                               L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"Hello", L"There", L"my", L"Name", L"is", L"Ryan"});
}

void TestCapitalizationFilterFactory::testCapitalization10() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"McKinley");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"false", L"minWordLength", L"3",
                               L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Mckinley"});
}

void TestCapitalizationFilterFactory::testCapitalization11() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"McKinley");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"false", L"minWordLength", L"3", L"okPrefix",
                               L"McK", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"McKinley"});
}

void TestCapitalizationFilterFactory::testCapitalization12() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"1st 2nd third");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"false", L"minWordLength", L"3", L"okPrefix",
                               L"McK", L"forceFirstLetter", L"false"})
               ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"1st", L"2nd", L"Third"});
}

void TestCapitalizationFilterFactory::testCapitalization13() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"the The the");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"and the it BIG", L"onlyFirstWord",
                               L"false", L"minWordLength", L"3", L"okPrefix",
                               L"McK", L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"The The the"});
}

void TestCapitalizationFilterFactory::testKeepIgnoreCase() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"kiTTEN");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"kitten", L"keepIgnoreCase", L"true",
                               L"onlyFirstWord", L"true", L"forceFirstLetter",
                               L"true"})
               ->create(stream);

  assertTokenStreamContents(stream, std::deque<wstring>{L"KiTTEN"});
}

void TestCapitalizationFilterFactory::testKeepIgnoreCase2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"kiTTEN");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keep", L"kitten", L"keepIgnoreCase", L"true",
                               L"onlyFirstWord", L"true", L"forceFirstLetter",
                               L"false"})
               ->create(stream);

  assertTokenStreamContents(stream, std::deque<wstring>{L"kiTTEN"});
}

void TestCapitalizationFilterFactory::testKeepIgnoreCase3() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"kiTTEN");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization",
                              {L"keepIgnoreCase", L"true", L"onlyFirstWord",
                               L"true", L"forceFirstLetter", L"false"})
               ->create(stream);

  assertTokenStreamContents(stream, std::deque<wstring>{L"Kitten"});
}

void TestCapitalizationFilterFactory::testMinWordLength() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"helo testing");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization", {L"onlyFirstWord", L"true",
                                                  L"minWordLength", L"5"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"helo", L"Testing"});
}

void TestCapitalizationFilterFactory::testMaxWordCount() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"one two three four");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization", {L"maxWordCount", L"2"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"One", L"Two", L"Three", L"Four"});
}

void TestCapitalizationFilterFactory::testMaxWordCount2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"one two three four");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization", {L"maxWordCount", L"2"})
               ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"one two three four"});
}

void TestCapitalizationFilterFactory::testMaxTokenLength() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization", {L"maxTokenLength", L"2"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this", L"is", L"A", L"test"});
}

void TestCapitalizationFilterFactory::testForceFirstLetterWithKeep() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"kitten");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Capitalization", {L"keep", L"kitten",
                                                  L"forceFirstLetter", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Kitten"});
}

void TestCapitalizationFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Capitalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestCapitalizationFilterFactory::testInvalidArguments() throw(
    runtime_error)
{
  for (auto arg : std::deque<wstring>{L"minWordLength", L"maxTokenLength",
                                       L"maxWordCount"}) {
    invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
      shared_ptr<Reader> reader =
          make_shared<StringReader>(L"foo foobar super-duper-trooper");
      shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
      tokenFilterFactory(L"Capitalization",
                         {L"keep", L"and the it BIG", L"onlyFirstWord",
                          L"false", arg, L"-3", L"okPrefix", L"McK",
                          L"forceFirstLetter", L"true"})
          ->create(stream);
    });
    assertTrue(expected.what()->contains(
                   arg + L" must be greater than or equal to zero") ||
               expected.what()->contains(arg + L" must be greater than zero"));
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous