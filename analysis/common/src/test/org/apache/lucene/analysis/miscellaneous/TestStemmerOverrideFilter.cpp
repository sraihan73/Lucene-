using namespace std;

#include "TestStemmerOverrideFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharacterUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/en/PorterStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/StemmerOverrideFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using PorterStemFilter = org::apache::lucene::analysis::en::PorterStemFilter;
using StemmerOverrideMap = org::apache::lucene::analysis::miscellaneous::
    StemmerOverrideFilter::StemmerOverrideMap;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<KeywordTokenizer> TestStemmerOverrideFilter::keywordTokenizer(
    const wstring &data) 
{
  shared_ptr<KeywordTokenizer> tokenizer = make_shared<KeywordTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(data));
  return tokenizer;
}

void TestStemmerOverrideFilter::testOverride() 
{
  // lets make booked stem to books
  // the override filter will convert "booked" to "books",
  // but also mark it with KeywordAttribute so Porter will not change it.
  shared_ptr<StemmerOverrideFilter::Builder> builder =
      make_shared<StemmerOverrideFilter::Builder>();
  builder->add(L"booked", L"books");
  shared_ptr<Tokenizer> tokenizer = keywordTokenizer(L"booked");
  shared_ptr<TokenStream> stream = make_shared<PorterStemFilter>(
      make_shared<StemmerOverrideFilter>(tokenizer, builder->build()));
  assertTokenStreamContents(stream, std::deque<wstring>{L"books"});
}

void TestStemmerOverrideFilter::testIgnoreCase() 
{
  // lets make booked stem to books
  // the override filter will convert "booked" to "books",
  // but also mark it with KeywordAttribute so Porter will not change it.
  shared_ptr<StemmerOverrideFilter::Builder> builder =
      make_shared<StemmerOverrideFilter::Builder>(true);
  builder->add(L"boOkEd", L"books");
  shared_ptr<Tokenizer> tokenizer = keywordTokenizer(L"BooKeD");
  shared_ptr<TokenStream> stream = make_shared<PorterStemFilter>(
      make_shared<StemmerOverrideFilter>(tokenizer, builder->build()));
  assertTokenStreamContents(stream, std::deque<wstring>{L"books"});
}

void TestStemmerOverrideFilter::testNoOverrides() 
{
  shared_ptr<StemmerOverrideFilter::Builder> builder =
      make_shared<StemmerOverrideFilter::Builder>(true);
  shared_ptr<Tokenizer> tokenizer = keywordTokenizer(L"book");
  shared_ptr<TokenStream> stream = make_shared<PorterStemFilter>(
      make_shared<StemmerOverrideFilter>(tokenizer, builder->build()));
  assertTokenStreamContents(stream, std::deque<wstring>{L"book"});
}

void TestStemmerOverrideFilter::testRandomRealisticWhiteSpace() throw(
    IOException)
{
  unordered_map<wstring, wstring> map_obj = unordered_map<wstring, wstring>();
  shared_ptr<Set<wstring>> seen = unordered_set<wstring>();
  int numTerms = atLeast(50);
  bool ignoreCase = random()->nextBoolean();

  for (int i = 0; i < numTerms; i++) {
    wstring randomRealisticUnicodeString =
        TestUtil::randomRealisticUnicodeString(random());
    std::deque<wchar_t> charArray = randomRealisticUnicodeString.toCharArray();
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    for (int j = 0; j < charArray.size();) {
      int cp = Character::codePointAt(charArray, j, charArray.size());
      if (!isspace(cp)) {
        builder->appendCodePoint(cp);
      }
      j += Character::charCount(cp);
    }
    if (builder->length() > 0) {
      wstring inputValue = builder->toString();

      // Make sure we don't try to add two inputs that vary only by case:
      wstring seenInputValue;
      if (ignoreCase) {
        // TODO: can we simply use inputValue.toLowerCase(Locale.ROOT)???
        std::deque<wchar_t> buffer = inputValue.toCharArray();
        CharacterUtils::toLowerCase(buffer, 0, buffer.size());
        // C++ TODO: There is no native C++ equivalent to 'toString':
        seenInputValue = buffer.toString();
      } else {
        seenInputValue = inputValue;
      }

      if (seen->contains(seenInputValue) == false) {
        seen->add(seenInputValue);
        wstring value = TestUtil::randomSimpleString(random());
        map_obj.emplace(inputValue, value.isEmpty() ? L"a" : value);
      }
    }
  }
  if (map_obj.empty()) {
    map_obj.emplace(L"booked", L"books");
  }
  shared_ptr<StemmerOverrideFilter::Builder> builder =
      make_shared<StemmerOverrideFilter::Builder>(ignoreCase);
  shared_ptr<Set<unordered_map::Entry<wstring, wstring>>> entrySet =
      map_obj.entrySet();
  shared_ptr<StringBuilder> input = make_shared<StringBuilder>();
  deque<wstring> output = deque<wstring>();
  for (auto entry : entrySet) {
    builder->add(entry.first, entry.second);
    if (random()->nextBoolean() || output.empty()) {
      input->append(entry.first)->append(L" ");
      output.push_back(entry.second);
    }
  }
  shared_ptr<Tokenizer> tokenizer = make_shared<WhitespaceTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(input->toString()));
  shared_ptr<TokenStream> stream = make_shared<PorterStemFilter>(
      make_shared<StemmerOverrideFilter>(tokenizer, builder->build()));
  assertTokenStreamContents(stream, output.toArray(std::deque<wstring>(0)));
}

void TestStemmerOverrideFilter::testRandomRealisticKeyword() 
{
  unordered_map<wstring, wstring> map_obj = unordered_map<wstring, wstring>();
  int numTerms = atLeast(50);
  for (int i = 0; i < numTerms; i++) {
    wstring randomRealisticUnicodeString =
        TestUtil::randomRealisticUnicodeString(random());
    if (randomRealisticUnicodeString.length() > 0) {
      wstring value = TestUtil::randomSimpleString(random());
      map_obj.emplace(randomRealisticUnicodeString, value.isEmpty() ? L"a" : value);
    }
  }
  if (map_obj.empty()) {
    map_obj.emplace(L"booked", L"books");
  }
  // This test might fail if ignoreCase is true since the map_obj might have twice
  // the same key, once lowercased and once uppercased
  shared_ptr<StemmerOverrideFilter::Builder> builder =
      make_shared<StemmerOverrideFilter::Builder>(false);
  shared_ptr<Set<unordered_map::Entry<wstring, wstring>>> entrySet =
      map_obj.entrySet();
  for (auto entry : entrySet) {
    builder->add(entry.first, entry.second);
  }
  shared_ptr<StemmerOverrideMap> build = builder->build();
  for (auto entry : entrySet) {
    if (random()->nextBoolean()) {
      shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
      tokenizer->setReader(make_shared<StringReader>(entry.first));
      shared_ptr<TokenStream> stream = make_shared<PorterStemFilter>(
          make_shared<StemmerOverrideFilter>(tokenizer, build));
      assertTokenStreamContents(stream, std::deque<wstring>{entry.second});
    }
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous