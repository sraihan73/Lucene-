using namespace std;

#include "TestFreeTextSuggester.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Document = org::apache::lucene::document::Document;
using Input = org::apache::lucene::search::suggest::Input;
using InputArrayIterator =
    org::apache::lucene::search::suggest::InputArrayIterator;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using BytesRef = org::apache::lucene::util::BytesRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Ignore;

void TestFreeTextSuggester::testBasic() 
{
  deque<std::shared_ptr<Input>> keys = AnalyzingSuggesterTest::shuffle(
      make_shared<Input>(L"foo bar baz blah", 50),
      make_shared<Input>(L"boo foo bar foo bee", 20));

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, 2, static_cast<char>(0x20));
  sug->build(make_shared<InputArrayIterator>(keys));
  TestUtil::assertEquals(2, sug->getCount());

  for (int i = 0; i < 2; i++) {

    // Uses bigram model and unigram backoff:
    TestUtil::assertEquals(
        L"foo bar/0.67 foo bee/0.33 baz/0.04 blah/0.04 boo/0.04",
        toString(sug->lookup(L"foo b", 10)));

    // Uses only bigram model:
    TestUtil::assertEquals(L"foo bar/0.67 foo bee/0.33",
                           toString(sug->lookup(L"foo ", 10)));

    // Uses only unigram model:
    TestUtil::assertEquals(L"foo/0.33", toString(sug->lookup(L"foo", 10)));

    // Uses only unigram model:
    TestUtil::assertEquals(L"bar/0.22 baz/0.11 bee/0.11 blah/0.11 boo/0.11",
                           toString(sug->lookup(L"b", 10)));

    // Try again after save/load:
    shared_ptr<Path> tmpDir = createTempDir(L"FreeTextSuggesterTest");

    shared_ptr<Path> path = tmpDir->resolve(L"suggester");

    shared_ptr<OutputStream> os = Files::newOutputStream(path);
    sug->store(os);
    os->close();

    shared_ptr<InputStream> is = Files::newInputStream(path);
    sug = make_shared<FreeTextSuggester>(a, a, 2, static_cast<char>(0x20));
    sug->load(is);
    is->close();
    TestUtil::assertEquals(2, sug->getCount());
  }
  delete a;
}

void TestFreeTextSuggester::testIllegalByteDuringBuild() 
{
  // Default separator is INFORMATION SEPARATOR TWO
  // (0x1e), so no input token is allowed to contain it
  deque<std::shared_ptr<Input>> keys = AnalyzingSuggesterTest::shuffle(
      make_shared<Input>(L"foo\u001ebar baz", 50));
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug = make_shared<FreeTextSuggester>(analyzer);
  expectThrows(invalid_argument::typeid,
               [&]() { sug->build(make_shared<InputArrayIterator>(keys)); });

  delete analyzer;
}

void TestFreeTextSuggester::testIllegalByteDuringQuery() 
{
  // Default separator is INFORMATION SEPARATOR TWO
  // (0x1e), so no input token is allowed to contain it
  deque<std::shared_ptr<Input>> keys =
      AnalyzingSuggesterTest::shuffle(make_shared<Input>(L"foo bar baz", 50));
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug = make_shared<FreeTextSuggester>(analyzer);
  sug->build(make_shared<InputArrayIterator>(keys));

  expectThrows(invalid_argument::typeid,
               [&]() { sug->lookup(L"foo\u001eb", 10); });

  delete analyzer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testWiki() throws Exception
void TestFreeTextSuggester::testWiki() 
{
  shared_ptr<LineFileDocs> *const lfd = make_shared<LineFileDocs>(
      nullptr, L"/lucenedata/enwiki/enwiki-20120502-lines-1k.txt");
  // Skip header:
  lfd->nextDoc();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug = make_shared<FreeTextSuggester>(analyzer);
  sug->build(
      make_shared<InputIteratorAnonymousInnerClass>(shared_from_this(), lfd));
  if (VERBOSE) {
    wcout << sug->ramBytesUsed() << L" bytes" << endl;

    deque<std::shared_ptr<LookupResult>> results =
        sug->lookup(L"general r", 10);
    wcout << L"results:" << endl;
    for (auto result : results) {
      wcout << L"  " << result << endl;
    }
  }
  delete analyzer;
}

TestFreeTextSuggester::InputIteratorAnonymousInnerClass::
    InputIteratorAnonymousInnerClass(
        shared_ptr<TestFreeTextSuggester> outerInstance,
        shared_ptr<LineFileDocs> lfd)
{
  this->outerInstance = outerInstance;
  this->lfd = lfd;
}

int64_t TestFreeTextSuggester::InputIteratorAnonymousInnerClass::weight()
{
  return 1;
}

shared_ptr<BytesRef>
TestFreeTextSuggester::InputIteratorAnonymousInnerClass::next()
{
  shared_ptr<Document> doc;
  try {
    doc = lfd->nextDoc();
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
  if (doc->empty()) {
    return nullptr;
  }
  if (count++ == 10000) {
    return nullptr;
  }
  return make_shared<BytesRef>(doc[L"body"]);
}

shared_ptr<BytesRef>
TestFreeTextSuggester::InputIteratorAnonymousInnerClass::payload()
{
  return nullptr;
}

bool TestFreeTextSuggester::InputIteratorAnonymousInnerClass::hasPayloads()
{
  return false;
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
TestFreeTextSuggester::InputIteratorAnonymousInnerClass::contexts()
{
  return nullptr;
}

bool TestFreeTextSuggester::InputIteratorAnonymousInnerClass::hasContexts()
{
  return false;
}

void TestFreeTextSuggester::testUnigrams() 
{
  deque<std::shared_ptr<Input>> keys = AnalyzingSuggesterTest::shuffle(
      make_shared<Input>(L"foo bar baz blah boo foo bar foo bee", 50));

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, 1, static_cast<char>(0x20));
  sug->build(make_shared<InputArrayIterator>(keys));
  // Sorts first by count, descending, second by term, ascending
  TestUtil::assertEquals(L"bar/0.22 baz/0.11 bee/0.11 blah/0.11 boo/0.11",
                         toString(sug->lookup(L"b", 10)));
  delete a;
}

void TestFreeTextSuggester::testNoDupsAcrossGrams() 
{
  deque<std::shared_ptr<Input>> keys = AnalyzingSuggesterTest::shuffle(
      make_shared<Input>(L"foo bar bar bar bar", 50));
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, 2, static_cast<char>(0x20));
  sug->build(make_shared<InputArrayIterator>(keys));
  TestUtil::assertEquals(L"foo bar/1.00", toString(sug->lookup(L"foo b", 10)));
  delete a;
}

void TestFreeTextSuggester::testEmptyString() 
{
  deque<std::shared_ptr<Input>> keys = AnalyzingSuggesterTest::shuffle(
      make_shared<Input>(L"foo bar bar bar bar", 50));
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, 2, static_cast<char>(0x20));
  sug->build(make_shared<InputArrayIterator>(keys));
  expectThrows(invalid_argument::typeid, [&]() { sug->lookup(L"", 10); });

  delete a;
}

void TestFreeTextSuggester::testEndingHole() 
{
  // Just deletes "of"
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  deque<std::shared_ptr<Input>> keys =
      AnalyzingSuggesterTest::shuffle(make_shared<Input>(L"wizard of oz", 50));
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, 3, static_cast<char>(0x20));
  sug->build(make_shared<InputArrayIterator>(keys));
  TestUtil::assertEquals(L"wizard _ oz/1.00",
                         toString(sug->lookup(L"wizard of", 10)));

  // Falls back to unigram model, with backoff 0.4 times
  // prop 0.5:
  TestUtil::assertEquals(L"oz/0.20", toString(sug->lookup(L"wizard o", 10)));
  delete a;
}

TestFreeTextSuggester::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestFreeTextSuggester> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFreeTextSuggester::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet({L"of"});
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<StopFilter>(tokenizer, stopSet));
}

void TestFreeTextSuggester::testTwoEndingHoles() 
{
  // Just deletes "of"
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  deque<std::shared_ptr<Input>> keys = AnalyzingSuggesterTest::shuffle(
      make_shared<Input>(L"wizard of of oz", 50));
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, 3, static_cast<char>(0x20));
  sug->build(make_shared<InputArrayIterator>(keys));
  TestUtil::assertEquals(L"", toString(sug->lookup(L"wizard of of", 10)));
  delete a;
}

TestFreeTextSuggester::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestFreeTextSuggester> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFreeTextSuggester::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet({L"of"});
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<StopFilter>(tokenizer, stopSet));
}

shared_ptr<java::util::Comparator<std::shared_ptr<
    org::apache::lucene::search::suggest::Lookup::LookupResult>>>
    TestFreeTextSuggester::byScoreThenKey =
        make_shared<ComparatorAnonymousInnerClass>();

TestFreeTextSuggester::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int TestFreeTextSuggester::ComparatorAnonymousInnerClass::compare(
    shared_ptr<LookupResult> a, shared_ptr<LookupResult> b)
{
  if (a->value > b->value) {
    return -1;
  } else if (a->value < b->value) {
    return 1;
  } else {
    // Tie break by UTF16 sort order:
    return (static_cast<wstring>(a->key))
        ->compare(static_cast<wstring>(b->key));
  }
}

void TestFreeTextSuggester::testRandom() 
{
  std::deque<wstring> terms(TestUtil::nextInt(random(), 2, 10));
  shared_ptr<Set<wstring>> seen = unordered_set<wstring>();
  while (seen->size() < terms.size()) {
    wstring token = TestUtil::randomSimpleString(random(), 1, 5);
    if (!seen->contains(token)) {
      terms[seen->size()] = token;
      seen->add(token);
    }
  }

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());

  int numDocs = atLeast(10);
  int64_t totTokens = 0;
  const std::deque<std::deque<wstring>> docs =
      std::deque<std::deque<wstring>>(numDocs);
  for (int i = 0; i < numDocs; i++) {
    docs[i] = std::deque<wstring>(atLeast(100));
    if (VERBOSE) {
      wcout << L"  doc " << i << L":";
    }
    for (int j = 0; j < docs[i].size(); j++) {
      docs[i][j] = getZipfToken(terms);
      if (VERBOSE) {
        wcout << L" " << docs[i][j];
      }
    }
    if (VERBOSE) {
      wcout << endl;
    }
    totTokens += docs[i].size();
  }

  int grams = TestUtil::nextInt(random(), 1, 4);

  if (VERBOSE) {
    wcout << L"TEST: " << terms.size() << L" terms; " << numDocs << L" docs; "
          << grams << L" grams" << endl;
  }

  // Build suggester model:
  shared_ptr<FreeTextSuggester> sug =
      make_shared<FreeTextSuggester>(a, a, grams, static_cast<char>(0x20));
  sug->build(
      make_shared<InputIteratorAnonymousInnerClass2>(shared_from_this(), docs));

  // Build inefficient but hopefully correct model:
  deque<unordered_map<wstring, int>> gramCounts =
      deque<unordered_map<wstring, int>>(grams);
  for (int gram = 0; gram < grams; gram++) {
    if (VERBOSE) {
      wcout << L"TEST: build model for gram=" << gram << endl;
    }
    unordered_map<wstring, int> model = unordered_map<wstring, int>();
    gramCounts.push_back(model);
    for (auto doc : docs) {
      for (int i = 0; i < doc.size() - gram; i++) {
        shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
        for (int j = i; j <= i + gram; j++) {
          if (j > i) {
            b->append(L' ');
          }
          b->append(doc[j]);
        }
        wstring token = b->toString();
        optional<int> curCount = model[token];
        if (!curCount) {
          model.emplace(token, 1);
        } else {
          model.emplace(token, 1 + curCount);
        }
        if (VERBOSE) {
          wcout << L"  add '" << token << L"' -> count=" << model[token]
                << endl;
        }
      }
    }
  }

  int lookups = atLeast(100);
  for (int iter = 0; iter < lookups; iter++) {
    std::deque<wstring> tokens(TestUtil::nextInt(random(), 1, 5));
    for (int i = 0; i < tokens.size(); i++) {
      tokens[i] = getZipfToken(terms);
    }

    // Maybe trim last token; be sure not to create the
    // empty string:
    int trimStart;
    if (tokens.size() == 1) {
      trimStart = 1;
    } else {
      trimStart = 0;
    }
    int trimAt = TestUtil::nextInt(random(), trimStart,
                                   tokens[tokens.size() - 1].length());
    tokens[tokens.size() - 1] = tokens[tokens.size() - 1].substr(0, trimAt);

    int num = TestUtil::nextInt(random(), 1, 100);
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (auto token : tokens) {
      b->append(L' ');
      b->append(token);
    }
    wstring query = b->toString();
    query = query.substr(1);

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" query='" << query << L"' num="
            << num << endl;
    }

    // Expected:
    deque<std::shared_ptr<LookupResult>> expected =
        deque<std::shared_ptr<LookupResult>>();
    double backoff = 1.0;
    seen = unordered_set<>();

    if (VERBOSE) {
      wcout << L"  compute expected" << endl;
    }
    for (int i = grams - 1; i >= 0; i--) {
      if (VERBOSE) {
        wcout << L"    grams=" << i << endl;
      }

      if (tokens.size() < i + 1) {
        // Don't have enough tokens to use this model
        if (VERBOSE) {
          wcout << L"      skip" << endl;
        }
        continue;
      }

      if (i == 0 && tokens[tokens.size() - 1].length() == 0) {
        // Never suggest unigrams from empty string:
        if (VERBOSE) {
          wcout << L"      skip unigram priors only" << endl;
        }
        continue;
      }

      // Build up "context" ngram:
      b = make_shared<StringBuilder>();
      for (int j = tokens.size() - i - 1; j < tokens.size() - 1; j++) {
        b->append(L' ');
        b->append(tokens[j]);
      }
      wstring context = b->toString();
      if (context.length() > 0) {
        context = context.substr(1);
      }
      if (VERBOSE) {
        wcout << L"      context='" << context << L"'" << endl;
      }
      int64_t contextCount;
      if (context.length() == 0) {
        contextCount = totTokens;
      } else {
        optional<int> count = gramCounts[i - 1][context];
        if (!count) {
          // We never saw this context:
          backoff *= FreeTextSuggester::ALPHA;
          if (VERBOSE) {
            wcout << L"      skip: never saw context" << endl;
          }
          continue;
        }
        contextCount = count;
      }
      if (VERBOSE) {
        wcout << L"      contextCount=" << contextCount << endl;
      }
      unordered_map<wstring, int> model = gramCounts[i];

      // First pass, gather all predictions for this model:
      if (VERBOSE) {
        wcout << L"      find terms w/ prefix=" << tokens[tokens.size() - 1]
              << endl;
      }
      deque<std::shared_ptr<LookupResult>> tmp =
          deque<std::shared_ptr<LookupResult>>();
      for (auto term : terms) {
        if (term.startsWith(tokens[tokens.size() - 1])) {
          if (VERBOSE) {
            wcout << L"        term=" << term << endl;
          }
          if (seen->contains(term)) {
            if (VERBOSE) {
              wcout << L"          skip seen" << endl;
            }
            continue;
          }
          wstring ngram = (context + L" " + term).trim();
          optional<int> count = model[ngram];
          if (count) {
            shared_ptr<LookupResult> lr = make_shared<LookupResult>(
                ngram,
                static_cast<int64_t>(
                    numeric_limits<int64_t>::max() *
                    (backoff * static_cast<double>(count) / contextCount)));
            tmp.push_back(lr);
            if (VERBOSE) {
              wcout << L"      add tmp key='" << lr->key << L"' score="
                    << lr->value << endl;
            }
          }
        }
      }

      // Second pass, trim to only top N, and fold those
      // into overall suggestions:
      // C++ TODO: The 'Compare' parameter of std::sort produces a bool
      // value, while the Java Comparator parameter produces a tri-state result:
      // ORIGINAL LINE: java.util.Collections.sort(tmp, byScoreThenKey);
      sort(tmp.begin(), tmp.end(), byScoreThenKey);
      if (tmp.size() > num) {
        tmp.subList(num, tmp.size())->clear();
      }
      for (auto result : tmp) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wstring key = result->key->toString();
        int idx = (int)key.rfind(L' ');
        wstring lastToken;
        if (idx != -1) {
          lastToken = key.substr(idx + 1);
        } else {
          lastToken = key;
        }
        if (!seen->contains(lastToken)) {
          seen->add(lastToken);
          expected.push_back(result);
          if (VERBOSE) {
            wcout << L"      keep key='" << result->key << L"' score="
                  << result->value << endl;
          }
        }
      }

      backoff *= FreeTextSuggester::ALPHA;
    }

    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expected, byScoreThenKey);
    sort(expected.begin(), expected.end(), byScoreThenKey);

    if (expected.size() > num) {
      expected.subList(num, expected.size())->clear();
    }

    // Actual:
    deque<std::shared_ptr<LookupResult>> actual = sug->lookup(query, num);

    if (VERBOSE) {
      wcout << L"  expected: " << expected << endl;
      wcout << L"    actual: " << actual << endl;
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(expected.toString(), actual.toString());
  }
  delete a;
}

TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::
    InputIteratorAnonymousInnerClass2(
        shared_ptr<TestFreeTextSuggester> outerInstance,
        deque<deque<wstring>> &docs)
{
  this->outerInstance = outerInstance;
  this->docs = docs;
}

shared_ptr<BytesRef>
TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::next()
{
  if (upto == docs.size()) {
    return nullptr;
  } else {
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (auto token : docs[upto]) {
      b->append(L' ');
      b->append(token);
    }
    upto++;
    return make_shared<BytesRef>(b->toString());
  }
}

int64_t TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::weight()
{
  return LuceneTestCase::random()->nextLong();
}

shared_ptr<BytesRef>
TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::payload()
{
  return nullptr;
}

bool TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::hasPayloads()
{
  return false;
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::contexts()
{
  return nullptr;
}

bool TestFreeTextSuggester::InputIteratorAnonymousInnerClass2::hasContexts()
{
  return false;
}

wstring TestFreeTextSuggester::getZipfToken(std::deque<wstring> &tokens)
{
  // Zipf-like distribution:
  for (int k = 0; k < tokens.size(); k++) {
    if (random()->nextBoolean() || k == tokens.size() - 1) {
      return tokens[k];
    }
  }
  assert(false);
  return L"";
}

wstring
TestFreeTextSuggester::toString(deque<std::shared_ptr<LookupResult>> &results)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (auto result : results) {
    b->append(L' ');
    b->append(result->key);
    b->append(L'/');
    b->append(wstring::format(Locale::ROOT, L"%.2f",
                              (static_cast<double>(result->value)) /
                                  numeric_limits<int64_t>::max()));
  }
  return b->toString()->trim();
}
} // namespace org::apache::lucene::search::suggest::analyzing