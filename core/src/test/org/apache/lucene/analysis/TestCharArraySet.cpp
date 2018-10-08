using namespace std;

#include "TestCharArraySet.h"

namespace org::apache::lucene::analysis
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
std::deque<wstring> const TestCharArraySet::TEST_STOP_WORDS = {
    L"a",    L"an",  L"and",   L"are",  L"as",    L"at",    L"be",
    L"but",  L"by",  L"for",   L"if",   L"in",    L"into",  L"is",
    L"it",   L"no",  L"not",   L"of",   L"on",    L"or",    L"such",
    L"that", L"the", L"their", L"then", L"there", L"these", L"they",
    L"this", L"to",  L"was",   L"will", L"with"};

void TestCharArraySet::testRehash() 
{
  shared_ptr<CharArraySet> cas = make_shared<CharArraySet>(0, true);
  for (int i = 0; i < TEST_STOP_WORDS.size(); i++) {
    cas->add(TEST_STOP_WORDS[i]);
  }
  assertEquals(TEST_STOP_WORDS.size(), cas->size());
  for (int i = 0; i < TEST_STOP_WORDS.size(); i++) {
    assertTrue(cas->contains(TEST_STOP_WORDS[i]));
  }
}

void TestCharArraySet::testNonZeroOffset()
{
  std::deque<wstring> words = {L"Hello", L"World", L"this",
                                L"is",    L"a",     L"test"};
  std::deque<wchar_t> findme = (wstring(L"xthisy")).toCharArray();
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(10, true);
  set->addAll(Arrays::asList(words));
  assertTrue(set->contains(findme, 1, 4));
  assertTrue(set->contains(wstring(findme, 1, 4)));

  // test unmodifiable
  set = CharArraySet::unmodifiableSet(set);
  assertTrue(set->contains(findme, 1, 4));
  assertTrue(set->contains(wstring(findme, 1, 4)));
}

void TestCharArraySet::testObjectContains()
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(10, true);
  optional<int> val = static_cast<Integer>(1);
  set->add(val);
  assertTrue(set->contains(val));
  assertTrue(set->contains(optional<int>(1))); // another integer
  assertTrue(set->contains(L"1"));
  assertTrue(set->contains(std::deque<wchar_t>{L'1'}));
  // test unmodifiable
  set = CharArraySet::unmodifiableSet(set);
  assertTrue(set->contains(val));
  assertTrue(set->contains(optional<int>(1))); // another integer
  assertTrue(set->contains(L"1"));
  assertTrue(set->contains(std::deque<wchar_t>{L'1'}));
}

void TestCharArraySet::testClear()
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(10, true);
  set->addAll(Arrays::asList(TEST_STOP_WORDS));
  assertEquals(L"Not all words added", TEST_STOP_WORDS.size(), set->size());
  set->clear();
  assertEquals(L"not empty", 0, set->size());
  for (int i = 0; i < TEST_STOP_WORDS.size(); i++) {
    assertFalse(set->contains(TEST_STOP_WORDS[i]));
  }
  set->addAll(Arrays::asList(TEST_STOP_WORDS));
  assertEquals(L"Not all words added", TEST_STOP_WORDS.size(), set->size());
  for (int i = 0; i < TEST_STOP_WORDS.size(); i++) {
    assertTrue(set->contains(TEST_STOP_WORDS[i]));
  }
}

void TestCharArraySet::testModifyOnUnmodifiable()
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(10, true);
  set->addAll(Arrays::asList(TEST_STOP_WORDS));
  constexpr int size = set->size();
  set = CharArraySet::unmodifiableSet(set);
  assertEquals(L"Set size changed due to unmodifiableSet call", size,
               set->size());
  wstring NOT_IN_SET = L"SirGallahad";
  assertFalse(L"Test std::wstring already exists in set", set->contains(NOT_IN_SET));

  try {
    set->add(NOT_IN_SET.toCharArray());
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable set",
                set->contains(NOT_IN_SET));
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }

  try {
    set->add(NOT_IN_SET);
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable set",
                set->contains(NOT_IN_SET));
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }

  try {
    set->add(make_shared<StringBuilder>(NOT_IN_SET));
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable set",
                set->contains(NOT_IN_SET));
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }

  try {
    set->clear();
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Changed unmodifiable set", set->contains(NOT_IN_SET));
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }
  try {
    set->add(static_cast<any>(NOT_IN_SET));
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable set",
                set->contains(NOT_IN_SET));
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }

  // This test was changed in 3.1, as a contains() call on the given deque
  // using the "correct" iterator's current key (now a char[]) on a Set<std::wstring>
  // would not hit any element of the CAS and therefor never call remove() on
  // the iterator
  try {
    set->removeAll(
        make_shared<CharArraySet>(Arrays::asList(TEST_STOP_WORDS), true));
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }

  try {
    set->retainAll(make_shared<CharArraySet>(Arrays::asList(NOT_IN_SET), true));
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertEquals(L"Size of unmodifiable set has changed", size, set->size());
  }

  try {
    set->addAll(Arrays::asList(NOT_IN_SET));
    fail(L"Modified unmodifiable set");
  } catch (const UnsupportedOperationException &e) {
    // expected
    assertFalse(L"Test std::wstring has been added to unmodifiable set",
                set->contains(NOT_IN_SET));
  }

  for (int i = 0; i < TEST_STOP_WORDS.size(); i++) {
    assertTrue(set->contains(TEST_STOP_WORDS[i]));
  }
}

void TestCharArraySet::testUnmodifiableSet()
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(10, true);
  set->addAll(Arrays::asList(TEST_STOP_WORDS));
  set->add(static_cast<Integer>(1));
  constexpr int size = set->size();
  set = CharArraySet::unmodifiableSet(set);
  assertEquals(L"Set size changed due to unmodifiableSet call", size,
               set->size());
  for (auto stopword : TEST_STOP_WORDS) {
    assertTrue(set->contains(stopword));
  }
  assertTrue(set->contains(static_cast<Integer>(1)));
  assertTrue(set->contains(L"1"));
  assertTrue(set->contains(std::deque<wchar_t>{L'1'}));

  expectThrows(NullPointerException::typeid,
               [&]() { CharArraySet::unmodifiableSet(nullptr); });
}

void TestCharArraySet::testSupplementaryChars()
{
  wstring missing = L"Term %s is missing in the set";
  wstring falsePos = L"Term %s is in the set but shouldn't";
  // for reference see
  // http://unicode.org/cldr/utility/deque-unicodeset.jsp?a=[[%3ACase_Sensitive%3DTrue%3A]%26[^[\u0000-\uFFFF]]]&esc=on
  std::deque<wstring> upperArr = {
      L"Abc\ud801\udc1c", L"\ud801\udc1c\ud801\udc1cCDE", L"A\ud801\udc1cB"};
  std::deque<wstring> lowerArr = {
      L"abc\ud801\udc44", L"\ud801\udc44\ud801\udc44cde", L"a\ud801\udc44b"};
  shared_ptr<CharArraySet> set =
      make_shared<CharArraySet>(Arrays::asList(TEST_STOP_WORDS), true);
  for (auto upper : upperArr) {
    set->add(upper);
  }
  for (int i = 0; i < upperArr.size(); i++) {
    assertTrue(wstring::format(Locale::ROOT, missing, upperArr[i]),
               set->contains(upperArr[i]));
    assertTrue(wstring::format(Locale::ROOT, missing, lowerArr[i]),
               set->contains(lowerArr[i]));
  }
  set = make_shared<CharArraySet>(Arrays::asList(TEST_STOP_WORDS), false);
  for (auto upper : upperArr) {
    set->add(upper);
  }
  for (int i = 0; i < upperArr.size(); i++) {
    assertTrue(wstring::format(Locale::ROOT, missing, upperArr[i]),
               set->contains(upperArr[i]));
    assertFalse(wstring::format(Locale::ROOT, falsePos, lowerArr[i]),
                set->contains(lowerArr[i]));
  }
}

void TestCharArraySet::testSingleHighSurrogate()
{
  wstring missing = L"Term %s is missing in the set";
  wstring falsePos = L"Term %s is in the set but shouldn't";
  std::deque<wstring> upperArr = {L"ABC\uD800", L"ABC\uD800EfG", L"\uD800EfG",
                                   L"\uD800\ud801\udc1cB"};

  std::deque<wstring> lowerArr = {L"abc\uD800", L"abc\uD800efg", L"\uD800efg",
                                   L"\uD800\ud801\udc44b"};
  shared_ptr<CharArraySet> set =
      make_shared<CharArraySet>(Arrays::asList(TEST_STOP_WORDS), true);
  for (auto upper : upperArr) {
    set->add(upper);
  }
  for (int i = 0; i < upperArr.size(); i++) {
    assertTrue(wstring::format(Locale::ROOT, missing, upperArr[i]),
               set->contains(upperArr[i]));
    assertTrue(wstring::format(Locale::ROOT, missing, lowerArr[i]),
               set->contains(lowerArr[i]));
  }
  set = make_shared<CharArraySet>(Arrays::asList(TEST_STOP_WORDS), false);
  for (auto upper : upperArr) {
    set->add(upper);
  }
  for (int i = 0; i < upperArr.size(); i++) {
    assertTrue(wstring::format(Locale::ROOT, missing, upperArr[i]),
               set->contains(upperArr[i]));
    assertFalse(wstring::format(Locale::ROOT, falsePos, upperArr[i]),
                set->contains(lowerArr[i]));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("deprecated") public void
// testCopyCharArraySetBWCompat()
void TestCharArraySet::testCopyCharArraySetBWCompat()
{
  shared_ptr<CharArraySet> setIngoreCase = make_shared<CharArraySet>(10, true);
  shared_ptr<CharArraySet> setCaseSensitive =
      make_shared<CharArraySet>(10, false);

  deque<wstring> stopwords = deque<wstring>{TEST_STOP_WORDS};
  deque<wstring> stopwordsUpper = deque<wstring>();
  for (auto string : stopwords) {
    stopwordsUpper.push_back(string.toUpperCase(Locale::ROOT));
  }
  setIngoreCase->addAll(Arrays::asList(TEST_STOP_WORDS));
  setIngoreCase->add(static_cast<Integer>(1));
  setCaseSensitive->addAll(Arrays::asList(TEST_STOP_WORDS));
  setCaseSensitive->add(static_cast<Integer>(1));

  shared_ptr<CharArraySet> copy = CharArraySet::copy(setIngoreCase);
  shared_ptr<CharArraySet> copyCaseSens = CharArraySet::copy(setCaseSensitive);

  assertEquals(setIngoreCase->size(), copy->size());
  assertEquals(setCaseSensitive->size(), copy->size());

  assertTrue(copy->containsAll(stopwords));
  assertTrue(copy->containsAll(stopwordsUpper));
  assertTrue(copyCaseSens->containsAll(stopwords));
  for (auto string : stopwordsUpper) {
    assertFalse(copyCaseSens->contains(string));
  }
  // test adding terms to the copy
  deque<wstring> newWords = deque<wstring>();
  for (auto string : stopwords) {
    newWords.push_back(string + L"_1");
  }
  copy->addAll(newWords);

  assertTrue(copy->containsAll(stopwords));
  assertTrue(copy->containsAll(stopwordsUpper));
  assertTrue(copy->containsAll(newWords));
  // new added terms are not in the source set
  for (auto string : newWords) {
    assertFalse(setIngoreCase->contains(string));
    assertFalse(setCaseSensitive->contains(string));
  }
}

void TestCharArraySet::testCopyCharArraySet()
{
  shared_ptr<CharArraySet> setIngoreCase = make_shared<CharArraySet>(10, true);
  shared_ptr<CharArraySet> setCaseSensitive =
      make_shared<CharArraySet>(10, false);

  deque<wstring> stopwords = deque<wstring>{TEST_STOP_WORDS};
  deque<wstring> stopwordsUpper = deque<wstring>();
  for (auto string : stopwords) {
    stopwordsUpper.push_back(string.toUpperCase(Locale::ROOT));
  }
  setIngoreCase->addAll(Arrays::asList(TEST_STOP_WORDS));
  setIngoreCase->add(static_cast<Integer>(1));
  setCaseSensitive->addAll(Arrays::asList(TEST_STOP_WORDS));
  setCaseSensitive->add(static_cast<Integer>(1));

  shared_ptr<CharArraySet> copy = CharArraySet::copy(setIngoreCase);
  shared_ptr<CharArraySet> copyCaseSens = CharArraySet::copy(setCaseSensitive);

  assertEquals(setIngoreCase->size(), copy->size());
  assertEquals(setCaseSensitive->size(), copy->size());

  assertTrue(copy->containsAll(stopwords));
  assertTrue(copy->containsAll(stopwordsUpper));
  assertTrue(copyCaseSens->containsAll(stopwords));
  for (auto string : stopwordsUpper) {
    assertFalse(copyCaseSens->contains(string));
  }
  // test adding terms to the copy
  deque<wstring> newWords = deque<wstring>();
  for (auto string : stopwords) {
    newWords.push_back(string + L"_1");
  }
  copy->addAll(newWords);

  assertTrue(copy->containsAll(stopwords));
  assertTrue(copy->containsAll(stopwordsUpper));
  assertTrue(copy->containsAll(newWords));
  // new added terms are not in the source set
  for (auto string : newWords) {
    assertFalse(setIngoreCase->contains(string));
    assertFalse(setCaseSensitive->contains(string));
  }
}

void TestCharArraySet::testCopyJDKSet()
{
  shared_ptr<Set<wstring>> set = unordered_set<wstring>();

  deque<wstring> stopwords = deque<wstring>{TEST_STOP_WORDS};
  deque<wstring> stopwordsUpper = deque<wstring>();
  for (auto string : stopwords) {
    stopwordsUpper.push_back(string.toUpperCase(Locale::ROOT));
  }
  set->addAll(Arrays::asList(TEST_STOP_WORDS));

  shared_ptr<CharArraySet> copy = CharArraySet::copy(set);

  assertEquals(set->size(), copy->size());
  assertEquals(set->size(), copy->size());

  assertTrue(copy->containsAll(stopwords));
  for (auto string : stopwordsUpper) {
    assertFalse(copy->contains(string));
  }

  deque<wstring> newWords = deque<wstring>();
  for (auto string : stopwords) {
    newWords.push_back(string + L"_1");
  }
  copy->addAll(newWords);

  assertTrue(copy->containsAll(stopwords));
  assertTrue(copy->containsAll(newWords));
  // new added terms are not in the source set
  for (auto string : newWords) {
    assertFalse(set->contains(string));
  }
}

void TestCharArraySet::testCopyEmptySet()
{
  assertSame(CharArraySet::EMPTY_SET,
             CharArraySet::copy(CharArraySet::EMPTY_SET));
}

void TestCharArraySet::testEmptySet()
{
  assertEquals(0, CharArraySet::EMPTY_SET->size());

  assertTrue(CharArraySet::EMPTY_SET->isEmpty());
  for (auto stopword : TEST_STOP_WORDS) {
    assertFalse(CharArraySet::EMPTY_SET->contains(stopword));
  }
  assertFalse(CharArraySet::EMPTY_SET->contains(L"foo"));
  assertFalse(CharArraySet::EMPTY_SET->contains(static_cast<any>(L"foo")));
  assertFalse(
      CharArraySet::EMPTY_SET->contains((wstring(L"foo")).toCharArray()));
  assertFalse(
      CharArraySet::EMPTY_SET->contains((wstring(L"foo")).toCharArray(), 0, 3));
}

void TestCharArraySet::testContainsWithNull()
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);

  expectThrows(NullPointerException::typeid, [&]() {
    set->contains(static_cast<std::deque<wchar_t>>(nullptr), 0, 10);
  });

  expectThrows(NullPointerException::typeid, [&]() {
    set->contains(std::static_pointer_cast<std::wstring>(nullptr));
  });

  expectThrows(NullPointerException::typeid,
               [&]() { set->contains(static_cast<any>(nullptr)); });
}

void TestCharArraySet::testToString()
{
  shared_ptr<CharArraySet> set =
      CharArraySet::copy(Collections::singleton(L"test"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[test]", set->toString());
  set->add(L"test2");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(set->toString()->find(L", ") != wstring::npos);
}
} // namespace org::apache::lucene::analysis