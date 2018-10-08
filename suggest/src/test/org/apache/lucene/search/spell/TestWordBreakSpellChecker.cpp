using namespace std;

#include "TestWordBreakSpellChecker.h"

namespace org::apache::lucene::search::spell
{
using junit::framework::Assert;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BreakSuggestionSortMethod = org::apache::lucene::search::spell::
    WordBreakSpellChecker::BreakSuggestionSortMethod;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestWordBreakSpellChecker::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  for (int i = 900; i < 1112; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring num = English::intToEnglish(i)
                      .replaceAll(L"[-]", L" ")
                      .replaceAll(L"[,]", L"");
    doc->push_back(newTextField(L"numbers", num, Field::Store::NO));
    writer->addDocument(doc);
  }

  {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"numbers", L"thou hast sand betwixt thy toes",
                                Field::Store::NO));
    writer->addDocument(doc);
  }
  {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"numbers", L"hundredeight eightyeight yeight",
                                Field::Store::NO));
    writer->addDocument(doc);
  }
  {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"numbers", L"tres y cinco", Field::Store::NO));
    writer->addDocument(doc);
  }

  writer->commit();
  delete writer;
}

void TestWordBreakSpellChecker::tearDown() 
{
  IOUtils::close({dir, analyzer});
  LuceneTestCase::tearDown();
}

void TestWordBreakSpellChecker::testCombiningWords() 
{
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<WordBreakSpellChecker> wbsp = make_shared<WordBreakSpellChecker>();

  {
    std::deque<std::shared_ptr<Term>> terms = {
        make_shared<Term>(L"numbers", L"one"),
        make_shared<Term>(L"numbers", L"hun"),
        make_shared<Term>(L"numbers", L"dred"),
        make_shared<Term>(L"numbers", L"eight"),
        make_shared<Term>(L"numbers", L"y"),
        make_shared<Term>(L"numbers", L"eight")};
    wbsp->setMaxChanges(3);
    wbsp->setMaxCombineWordLength(20);
    wbsp->setMinSuggestionFrequency(1);
    std::deque<std::shared_ptr<CombineSuggestion>> cs =
        wbsp->suggestWordCombinations(terms, 10, ir,
                                      SuggestMode::SUGGEST_ALWAYS);
    Assert::assertTrue(cs.size() == 5);

    Assert::assertTrue(cs[0]->originalTermIndexes->size() == 2);
    Assert::assertTrue(cs[0]->originalTermIndexes[0] == 1);
    Assert::assertTrue(cs[0]->originalTermIndexes[1] == 2);
    Assert::assertTrue(cs[0]->suggestion->string == L"hundred");
    Assert::assertTrue(cs[0]->suggestion->score == 1);

    Assert::assertTrue(cs[1]->originalTermIndexes->size() == 2);
    Assert::assertTrue(cs[1]->originalTermIndexes[0] == 3);
    Assert::assertTrue(cs[1]->originalTermIndexes[1] == 4);
    Assert::assertTrue(cs[1]->suggestion->string == L"eighty");
    Assert::assertTrue(cs[1]->suggestion->score == 1);

    Assert::assertTrue(cs[2]->originalTermIndexes->size() == 2);
    Assert::assertTrue(cs[2]->originalTermIndexes[0] == 4);
    Assert::assertTrue(cs[2]->originalTermIndexes[1] == 5);
    Assert::assertTrue(cs[2]->suggestion->string == L"yeight");
    Assert::assertTrue(cs[2]->suggestion->score == 1);

    for (int i = 3; i < 5; i++) {
      Assert::assertTrue(cs[i]->originalTermIndexes->size() == 3);
      Assert::assertTrue(cs[i]->suggestion->score == 2);
      Assert::assertTrue((cs[i]->originalTermIndexes[0] == 1 &&
                          cs[i]->originalTermIndexes[1] == 2 &&
                          cs[i]->originalTermIndexes[2] == 3 &&
                          cs[i]->suggestion->string == L"hundredeight") ||
                         (cs[i]->originalTermIndexes[0] == 3 &&
                          cs[i]->originalTermIndexes[1] == 4 &&
                          cs[i]->originalTermIndexes[2] == 5 &&
                          cs[i]->suggestion->string == L"eightyeight"));
    }

    cs = wbsp->suggestWordCombinations(terms, 5, ir,
                                       SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
    Assert::assertTrue(cs.size() == 2);
    Assert::assertTrue(cs[0]->originalTermIndexes->size() == 2);
    Assert::assertTrue(cs[0]->suggestion->score == 1);
    Assert::assertTrue(cs[0]->originalTermIndexes[0] == 1);
    Assert::assertTrue(cs[0]->originalTermIndexes[1] == 2);
    Assert::assertTrue(cs[0]->suggestion->string == L"hundred");
    Assert::assertTrue(cs[0]->suggestion->score == 1);

    Assert::assertTrue(cs[1]->originalTermIndexes->size() == 3);
    Assert::assertTrue(cs[1]->suggestion->score == 2);
    Assert::assertTrue(cs[1]->originalTermIndexes[0] == 1);
    Assert::assertTrue(cs[1]->originalTermIndexes[1] == 2);
    Assert::assertTrue(cs[1]->originalTermIndexes[2] == 3);
    Assert::assertTrue(cs[1]->suggestion->string == L"hundredeight");
  }
  delete ir;
}

void TestWordBreakSpellChecker::testBreakingWords() 
{
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<WordBreakSpellChecker> wbsp = make_shared<WordBreakSpellChecker>();

  {
    shared_ptr<Term> term = make_shared<Term>(L"numbers", L"ninetynine");
    wbsp->setMaxChanges(1);
    wbsp->setMinBreakWordLength(1);
    wbsp->setMinSuggestionFrequency(1);
    std::deque<std::deque<std::shared_ptr<SuggestWord>>> sw =
        wbsp->suggestWordBreaks(
            term, 5, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
            BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 1);
    Assert::assertTrue(sw[0].size() == 2);
    Assert::assertTrue(sw[0][0]->string == L"ninety");
    Assert::assertTrue(sw[0][1]->string == L"nine");
    Assert::assertTrue(sw[0][0]->score == 1);
    Assert::assertTrue(sw[0][1]->score == 1);
  }
  {
    shared_ptr<Term> term = make_shared<Term>(L"numbers", L"onethousand");
    wbsp->setMaxChanges(1);
    wbsp->setMinBreakWordLength(1);
    wbsp->setMinSuggestionFrequency(1);
    std::deque<std::deque<std::shared_ptr<SuggestWord>>> sw =
        wbsp->suggestWordBreaks(
            term, 2, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
            BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 1);
    Assert::assertTrue(sw[0].size() == 2);
    Assert::assertTrue(sw[0][0]->string == L"one");
    Assert::assertTrue(sw[0][1]->string == L"thousand");
    Assert::assertTrue(sw[0][0]->score == 1);
    Assert::assertTrue(sw[0][1]->score == 1);

    wbsp->setMaxChanges(2);
    wbsp->setMinSuggestionFrequency(1);
    sw = wbsp->suggestWordBreaks(
        term, 1, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
        BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 1);
    Assert::assertTrue(sw[0].size() == 2);

    wbsp->setMaxChanges(2);
    wbsp->setMinSuggestionFrequency(2);
    sw = wbsp->suggestWordBreaks(
        term, 2, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
        BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 1);
    Assert::assertTrue(sw[0].size() == 2);

    wbsp->setMaxChanges(2);
    wbsp->setMinSuggestionFrequency(1);
    sw = wbsp->suggestWordBreaks(
        term, 2, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
        BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 2);
    Assert::assertTrue(sw[0].size() == 2);
    Assert::assertTrue(sw[0][0]->string == L"one");
    Assert::assertTrue(sw[0][1]->string == L"thousand");
    Assert::assertTrue(sw[0][0]->score == 1);
    Assert::assertTrue(sw[0][1]->score == 1);
    Assert::assertTrue(sw[0][1]->freq > 1);
    Assert::assertTrue(sw[0][0]->freq > sw[0][1]->freq);
    Assert::assertTrue(sw[1].size() == 3);
    Assert::assertTrue(sw[1][0]->string == L"one");
    Assert::assertTrue(sw[1][1]->string == L"thou");
    Assert::assertTrue(sw[1][2]->string == L"sand");
    Assert::assertTrue(sw[1][0]->score == 2);
    Assert::assertTrue(sw[1][1]->score == 2);
    Assert::assertTrue(sw[1][2]->score == 2);
    Assert::assertTrue(sw[1][0]->freq > 1);
    Assert::assertTrue(sw[1][1]->freq == 1);
    Assert::assertTrue(sw[1][2]->freq == 1);
  }
  {
    shared_ptr<Term> term =
        make_shared<Term>(L"numbers", L"onethousandonehundredeleven");
    wbsp->setMaxChanges(3);
    wbsp->setMinBreakWordLength(1);
    wbsp->setMinSuggestionFrequency(1);
    std::deque<std::deque<std::shared_ptr<SuggestWord>>> sw =
        wbsp->suggestWordBreaks(
            term, 5, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
            BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.empty());

    wbsp->setMaxChanges(4);
    sw = wbsp->suggestWordBreaks(
        term, 5, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
        BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 1);
    Assert::assertTrue(sw[0].size() == 5);

    wbsp->setMaxChanges(5);
    sw = wbsp->suggestWordBreaks(
        term, 5, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
        BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.size() == 2);
    Assert::assertTrue(sw[0].size() == 5);
    Assert::assertTrue(sw[0][1]->string == L"thousand");
    Assert::assertTrue(sw[1].size() == 6);
    Assert::assertTrue(sw[1][1]->string == L"thou");
    Assert::assertTrue(sw[1][2]->string == L"sand");
  }
  {
    // make sure we can handle 2-char codepoints
    shared_ptr<Term> term = make_shared<Term>(L"numbers", L"\uD864\uDC79");
    wbsp->setMaxChanges(1);
    wbsp->setMinBreakWordLength(1);
    wbsp->setMinSuggestionFrequency(1);
    std::deque<std::deque<std::shared_ptr<SuggestWord>>> sw =
        wbsp->suggestWordBreaks(
            term, 5, ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX,
            BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
    Assert::assertTrue(sw.empty());
  }

  delete ir;
}

void TestWordBreakSpellChecker::testRandom() 
{
  int numDocs = TestUtil::nextInt(random(), (10 * RANDOM_MULTIPLIER),
                                  (100 * RANDOM_MULTIPLIER));
  shared_ptr<IndexReader> ir = nullptr;

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  int maxLength = TestUtil::nextInt(random(), 5, 50);
  deque<wstring> originals = deque<wstring>(numDocs);
  deque<std::deque<wstring>> breaks = deque<std::deque<wstring>>(numDocs);
  for (int i = 0; i < numDocs; i++) {
    wstring orig = L"";
    if (random()->nextBoolean()) {
      while (!goodTestString(orig)) {
        orig = TestUtil::randomSimpleString(random(), maxLength);
      }
    } else {
      while (!goodTestString(orig)) {
        orig = TestUtil::randomUnicodeString(random(), maxLength);
      }
    }
    originals.push_back(orig);
    int totalLength = orig.codePointCount(0, orig.length());
    int breakAt = orig.offsetByCodePoints(
        0, TestUtil::nextInt(random(), 1, totalLength - 1));
    std::deque<wstring> broken(2);
    broken[0] = orig.substr(0, breakAt);
    broken[1] = orig.substr(breakAt);
    breaks.push_back(broken);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"random_break", broken[0] + L" " + broken[1],
                                Field::Store::NO));
    doc->push_back(newTextField(L"random_combine", orig, Field::Store::NO));
    writer->addDocument(doc);
  }
  writer->commit();
  delete writer;

  ir = DirectoryReader::open(dir);
  shared_ptr<WordBreakSpellChecker> wbsp = make_shared<WordBreakSpellChecker>();
  wbsp->setMaxChanges(1);
  wbsp->setMinBreakWordLength(1);
  wbsp->setMinSuggestionFrequency(1);
  wbsp->setMaxCombineWordLength(maxLength);
  for (int i = 0; i < originals.size(); i++) {
    wstring orig = originals[i];
    wstring left = breaks[i][0];
    wstring right = breaks[i][1];
    {
      shared_ptr<Term> term = make_shared<Term>(L"random_break", orig);

      std::deque<std::deque<std::shared_ptr<SuggestWord>>> sw =
          wbsp->suggestWordBreaks(
              term, originals.size(), ir, SuggestMode::SUGGEST_ALWAYS,
              BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY);
      bool failed = true;
      for (auto sw1 : sw) {
        Assert::assertTrue(sw1.size() == 2);
        if (sw1[0]->string == left && sw1[1]->string == right) {
          failed = false;
        }
      }
      Assert::assertFalse(L"Failed getting break suggestions\n >Original: " +
                              orig + L"\n >Left: " + left + L"\n >Right: " +
                              right,
                          failed);
    }
    {
      std::deque<std::shared_ptr<Term>> terms = {
          make_shared<Term>(L"random_combine", left),
          make_shared<Term>(L"random_combine", right)};
      std::deque<std::shared_ptr<CombineSuggestion>> cs =
          wbsp->suggestWordCombinations(terms, originals.size(), ir,
                                        SuggestMode::SUGGEST_ALWAYS);
      bool failed = true;
      for (auto cs1 : cs) {
        Assert::assertTrue(cs1->originalTermIndexes.size() == 2);
        if (cs1->suggestion->string == left + right) {
          failed = false;
        }
      }
      Assert::assertFalse(L"Failed getting combine suggestions\n >Original: " +
                              orig + L"\n >Left: " + left + L"\n >Right: " +
                              right,
                          failed);
    }
  }
  IOUtils::close({ir, dir, analyzer});
}

const shared_ptr<java::util::regex::Pattern>
    TestWordBreakSpellChecker::mockTokenizerWhitespacePattern =
        java::util::regex::Pattern::compile(L"[ \\t\\r\\n]");

bool TestWordBreakSpellChecker::goodTestString(const wstring &s)
{
  if (s.codePointCount(0, s.length()) < 2 ||
      mockTokenizerWhitespacePattern->matcher(s).find()) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::search::spell