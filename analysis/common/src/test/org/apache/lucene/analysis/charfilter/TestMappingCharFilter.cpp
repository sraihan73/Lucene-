using namespace std;

#include "TestMappingCharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/UnicodeUtil.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"

namespace org::apache::lucene::analysis::charfilter
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

void TestMappingCharFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  shared_ptr<NormalizeCharMap::Builder> builder =
      make_shared<NormalizeCharMap::Builder>();

  builder->add(L"aa", L"a");
  builder->add(L"bbb", L"b");
  builder->add(L"cccc", L"cc");

  builder->add(L"h", L"i");
  builder->add(L"j", L"jj");
  builder->add(L"k", L"kkk");
  builder->add(L"ll", L"llll");

  builder->add(L"empty", L"");

  // BMP (surrogate pair):
  builder->add(UnicodeUtil::newString(std::deque<int>{0x1D122}, 0, 1),
               L"fclef");

  builder->add(L"\uff01", L"full-width-exclamation");

  normMap = builder->build();
}

void TestMappingCharFilter::testReaderReset() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"x"));
  std::deque<wchar_t> buf(10);
  int len = cs->read(buf, 0, 10);
  TestUtil::assertEquals(1, len);
  TestUtil::assertEquals(L'x', buf[0]);
  len = cs->read(buf, 0, 10);
  TestUtil::assertEquals(-1, len);

  // rewind
  cs->reset();
  len = cs->read(buf, 0, 10);
  TestUtil::assertEquals(1, len);
  TestUtil::assertEquals(L'x', buf[0]);
}

void TestMappingCharFilter::testNothingChange() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"x"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"x"}, std::deque<int>{0},
                            std::deque<int>{1}, 1);
}

void TestMappingCharFilter::test1to1() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"h"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"i"}, std::deque<int>{0},
                            std::deque<int>{1}, 1);
}

void TestMappingCharFilter::test1to2() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"j"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"jj"},
                            std::deque<int>{0}, std::deque<int>{1}, 1);
}

void TestMappingCharFilter::test1to3() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"k"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"kkk"},
                            std::deque<int>{0}, std::deque<int>{1}, 1);
}

void TestMappingCharFilter::test2to4() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"ll"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"llll"},
                            std::deque<int>{0}, std::deque<int>{2}, 2);
}

void TestMappingCharFilter::test2to1() 
{
  shared_ptr<CharFilter> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(L"aa"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"a"}, std::deque<int>{0},
                            std::deque<int>{2}, 2);
}

void TestMappingCharFilter::test3to1() 
{
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<StringReader>(L"bbb"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"b"}, std::deque<int>{0},
                            std::deque<int>{3}, 3);
}

void TestMappingCharFilter::test4to2() 
{
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<StringReader>(L"cccc"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"cc"},
                            std::deque<int>{0}, std::deque<int>{4}, 4);
}

void TestMappingCharFilter::test5to0() 
{
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<StringReader>(L"empty"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>(0), std::deque<int>(),
                            std::deque<int>(), 5);
}

void TestMappingCharFilter::testNonBMPChar() 
{
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<StringReader>(
                   UnicodeUtil::newString(std::deque<int>{0x1D122}, 0, 1)));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"fclef"},
                            std::deque<int>{0}, std::deque<int>{2}, 2);
}

void TestMappingCharFilter::testFullWidthChar() 
{
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<StringReader>(L"\uff01"));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"full-width-exclamation"},
                            std::deque<int>{0}, std::deque<int>{1}, 1);
}

void TestMappingCharFilter::testTokenStream() 
{
  wstring testString = L"h i j k ll cccc bbb aa";
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<StringReader>(testString));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"i", L"i", L"jj", L"kkk",
                                                 L"llll", L"cc", L"b", L"a"},
                            std::deque<int>{0, 2, 4, 6, 8, 11, 16, 20},
                            std::deque<int>{1, 3, 5, 7, 10, 15, 19, 22},
                            testString.length());
}

void TestMappingCharFilter::testChained() 
{
  wstring testString = L"aaaa ll h";
  shared_ptr<CharFilter> cs = make_shared<MappingCharFilter>(
      normMap, make_shared<MappingCharFilter>(
                   normMap, make_shared<StringReader>(testString)));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"a", L"llllllll", L"i"},
                            std::deque<int>{0, 5, 8},
                            std::deque<int>{4, 7, 9}, testString.length());
}

void TestMappingCharFilter::testRandom() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  int numRounds = RANDOM_MULTIPLIER * 10000;
  checkRandomData(random(), analyzer, numRounds);
  delete analyzer;
}

TestMappingCharFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestMappingCharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMappingCharFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
TestMappingCharFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MappingCharFilter>(outerInstance->normMap, reader);
}

void TestMappingCharFilter::testFinalOffsetSpecialCase() 
{
  shared_ptr<NormalizeCharMap::Builder> *const builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"t", L"");
  // even though this below rule has no effect, the test passes if you remove
  // it!!
  builder->add(L"tmakdbl", L"c");

  shared_ptr<NormalizeCharMap> *const map_obj = builder->build();

  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), map_obj);

  wstring text = L"gzw f quaxot";
  checkAnalysisConsistency(random(), analyzer, false, text);
  delete analyzer;
}

TestMappingCharFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestMappingCharFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::charfilter::NormalizeCharMap>
            map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMappingCharFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
TestMappingCharFilter::AnalyzerAnonymousInnerClass2::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MappingCharFilter>(map_obj, reader);
}

void TestMappingCharFilter::testRandomMaps() 
{
  int numIterations = atLeast(3);
  for (int i = 0; i < numIterations; i++) {
    shared_ptr<NormalizeCharMap> *const map_obj = randomMap();
    shared_ptr<Analyzer> analyzer =
        make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), map_obj);
    int numRounds = 100;
    checkRandomData(random(), analyzer, numRounds);
    delete analyzer;
  }
}

TestMappingCharFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestMappingCharFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::charfilter::NormalizeCharMap>
            map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMappingCharFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
TestMappingCharFilter::AnalyzerAnonymousInnerClass3::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MappingCharFilter>(map_obj, reader);
}

shared_ptr<NormalizeCharMap> TestMappingCharFilter::randomMap()
{
  shared_ptr<Random> random = TestMappingCharFilter::random();
  shared_ptr<NormalizeCharMap::Builder> builder =
      make_shared<NormalizeCharMap::Builder>();
  // we can't add duplicate keys, or NormalizeCharMap gets angry
  shared_ptr<Set<wstring>> keys = unordered_set<wstring>();
  int num = random->nextInt(5);
  // System.out.println("NormalizeCharMap=");
  for (int i = 0; i < num; i++) {
    wstring key = TestUtil::randomSimpleString(random);
    if (!keys->contains(key) && key.length() != 0) {
      wstring value = TestUtil::randomSimpleString(random);
      builder->add(key, value);
      keys->add(key);
      // System.out.println("mapping: '" + key + "' => '" + value + "'");
    }
  }
  return builder->build();
}

void TestMappingCharFilter::testRandomMaps2() 
{
  shared_ptr<Random> *const random = TestMappingCharFilter::random();
  constexpr int numIterations = atLeast(3);
  for (int iter = 0; iter < numIterations; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST iter=" << iter << endl;
    }

    constexpr wchar_t endLetter =
        static_cast<wchar_t>(TestUtil::nextInt(random, L'b', L'z'));

    const unordered_map<wstring, wstring> map_obj =
        unordered_map<wstring, wstring>();
    shared_ptr<NormalizeCharMap::Builder> *const builder =
        make_shared<NormalizeCharMap::Builder>();
    constexpr int numMappings = atLeast(5);
    if (VERBOSE) {
      wcout << L"  mappings:" << endl;
    }
    while (map_obj.size() < numMappings) {
      const wstring key =
          TestUtil::randomSimpleStringRange(random, L'a', endLetter, 7);
      if (key.length() != 0 && map_obj.find(key) == map_obj.end()) {
        const wstring value = TestUtil::randomSimpleString(random);
        map_obj.emplace(key, value);
        builder->add(key, value);
        if (VERBOSE) {
          wcout << L"    " << key << L" -> " << value << endl;
        }
      }
    }

    shared_ptr<NormalizeCharMap> *const charMap = builder->build();

    if (VERBOSE) {
      wcout << L"  test random documents..." << endl;
    }

    for (int iter2 = 0; iter2 < 100; iter2++) {
      const wstring content = TestUtil::randomSimpleStringRange(
          random, L'a', endLetter, atLeast(1000));

      if (VERBOSE) {
        wcout << L"  content=" << content << endl;
      }

      // Do stupid dog-slow mapping:

      // Output string:
      shared_ptr<StringBuilder> *const output = make_shared<StringBuilder>();

      // Maps output offset to input offset:
      const deque<int> inputOffsets = deque<int>();

      int cumDiff = 0;
      int charIdx = 0;
      while (charIdx < content.length()) {

        int matchLen = -1;
        wstring matchRepl = L"";

        for (auto ent : map_obj) {
          const wstring match = ent.first;
          if (charIdx + match.length() <= content.length()) {
            constexpr int limit = charIdx + match.length();
            bool matches = true;
            for (int charIdx2 = charIdx; charIdx2 < limit; charIdx2++) {
              if (match[charIdx2 - charIdx] != content[charIdx2]) {
                matches = false;
                break;
              }
            }

            if (matches) {
              const wstring repl = ent.second;
              if (match.length() > matchLen) {
                // Greedy: longer match wins
                matchLen = match.length();
                matchRepl = repl;
              }
            }
          }
        }

        if (matchLen != -1) {
          // We found a match here!
          if (VERBOSE) {
            wcout << L"    match=" << content.substr(charIdx, matchLen)
                  << L" @ off=" << charIdx << L" repl=" << matchRepl << endl;
          }
          output->append(matchRepl);
          constexpr int minLen = min(matchLen, matchRepl.length());

          // Common part, directly maps back to input
          // offset:
          for (int outIdx = 0; outIdx < minLen; outIdx++) {
            inputOffsets.push_back(output->length() - matchRepl.length() +
                                   outIdx + cumDiff);
          }

          cumDiff += matchLen - matchRepl.length();
          charIdx += matchLen;

          if (matchRepl.length() < matchLen) {
            // Replacement string is shorter than matched
            // input: nothing to do
          } else if (matchRepl.length() > matchLen) {
            // Replacement string is longer than matched
            // input: for all the "extra" chars we map_obj
            // back to a single input offset:
            for (int outIdx = matchLen; outIdx < matchRepl.length(); outIdx++) {
              inputOffsets.push_back(output->length() + cumDiff - 1);
            }
          } else {
            // Same length: no change to offset
          }

          assert((inputOffsets.size() == output->length(),
                  L"inputOffsets.size()=" + inputOffsets.size() +
                      L" vs output.length()=" + to_wstring(output->length())));
        } else {
          inputOffsets.push_back(output->length() + cumDiff);
          output->append(content[charIdx]);
          charIdx++;
        }
      }

      const wstring expected = output->toString();
      if (VERBOSE) {
        wcout << L"    expected:";
        for (int charIdx2 = 0; charIdx2 < expected.length(); charIdx2++) {
          wcout << L" " << expected[charIdx2] << L"/" << inputOffsets[charIdx2];
        }
        wcout << endl;
      }

      shared_ptr<MappingCharFilter> *const mapFilter =
          make_shared<MappingCharFilter>(charMap,
                                         make_shared<StringReader>(content));

      shared_ptr<StringBuilder> *const actualBuilder =
          make_shared<StringBuilder>();
      const deque<int> actualInputOffsets = deque<int>();

      // Now consume the actual mapFilter, somewhat randomly:
      while (true) {
        if (random->nextBoolean()) {
          constexpr int ch = mapFilter->read();
          if (ch == -1) {
            break;
          }
          actualBuilder->append(static_cast<wchar_t>(ch));
        } else {
          const std::deque<wchar_t> buffer =
              std::deque<wchar_t>(TestUtil::nextInt(random, 1, 100));
          constexpr int off =
              buffer.size() == 1 ? 0 : random->nextInt(buffer.size() - 1);
          constexpr int count =
              mapFilter->read(buffer, off, buffer.size() - off);
          if (count == -1) {
            break;
          } else {
            actualBuilder->append(buffer, off, count);
          }
        }

        if (random->nextInt(10) == 7) {
          // Map offsets
          while (actualInputOffsets.size() < actualBuilder->length()) {
            actualInputOffsets.push_back(
                mapFilter->correctOffset(actualInputOffsets.size()));
          }
        }
      }

      // Finish mappping offsets
      while (actualInputOffsets.size() < actualBuilder->length()) {
        actualInputOffsets.push_back(
            mapFilter->correctOffset(actualInputOffsets.size()));
      }

      const wstring actual = actualBuilder->toString();

      // Verify:
      TestUtil::assertEquals(expected, actual);
      TestUtil::assertEquals(inputOffsets, actualInputOffsets);
    }
  }
}
} // namespace org::apache::lucene::analysis::charfilter