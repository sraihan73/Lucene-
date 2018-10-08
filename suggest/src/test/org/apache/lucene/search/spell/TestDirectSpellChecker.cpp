using namespace std;

#include "TestDirectSpellChecker.h"

namespace org::apache::lucene::search::spell
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDirectSpellChecker::testInternalLevenshteinDistance() throw(
    runtime_error)
{
  shared_ptr<DirectSpellChecker> spellchecker =
      make_shared<DirectSpellChecker>();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  std::deque<wstring> termsToAdd = {L"metanoia", L"metanoian", L"metanoiai",
                                     L"metanoias", L"metanoi𐑍"};
  for (int i = 0; i < termsToAdd.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"repentance", termsToAdd[i], Field::Store::NO));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = writer->getReader();
  wstring misspelled = L"metanoix";
  std::deque<std::shared_ptr<SuggestWord>> similar =
      spellchecker->suggestSimilar(make_shared<Term>(L"repentance", misspelled),
                                   4, ir,
                                   SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() == 4);

  shared_ptr<StringDistance> sd = spellchecker->getDistance();
  assertTrue(std::dynamic_pointer_cast<LuceneLevenshteinDistance>(sd) !=
             nullptr);
  for (auto word : similar) {
    assertTrue(word->score == sd->getDistance(word->string, misspelled));
    assertTrue(word->score == sd->getDistance(misspelled, word->string));
  }

  IOUtils::close({ir, writer, dir, analyzer});
}

void TestDirectSpellChecker::testSimpleExamples() 
{
  shared_ptr<DirectSpellChecker> spellChecker =
      make_shared<DirectSpellChecker>();
  spellChecker->setMinQueryLength(0);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"numbers", English::intToEnglish(i), Field::Store::NO));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = writer->getReader();

  std::deque<std::shared_ptr<SuggestWord>> similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"fvie"), 2,
                                   ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() > 0);
  assertEquals(L"five", similar[0]->string);

  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"five"), 2,
                                   ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  if (similar.size() > 0) {
    assertFalse(similar[0]->string ==
                L"five"); // don't suggest a word for itself
  }

  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"fvie"), 2,
                                   ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() > 0);
  assertEquals(L"five", similar[0]->string);

  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"fiv"), 2, ir,
                                   SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() > 0);
  assertEquals(L"five", similar[0]->string);

  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"fives"), 2,
                                   ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() > 0);
  assertEquals(L"five", similar[0]->string);

  assertTrue(similar.size() > 0);
  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"fie"), 2, ir,
                                   SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(L"five", similar[0]->string);

  // add some more documents
  for (int i = 1000; i < 1100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"numbers", English::intToEnglish(i), Field::Store::NO));
    writer->addDocument(doc);
  }

  delete ir;
  ir = writer->getReader();

  // look ma, no spellcheck index rebuild
  similar = spellChecker->suggestSimilar(
      make_shared<Term>(L"numbers", L"tousand"), 10, ir,
      SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() > 0);
  assertEquals(L"thousand", similar[0]->string);

  IOUtils::close({ir, writer, dir, analyzer});
}

void TestDirectSpellChecker::testOptions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"text", L"foobar", Field::Store::NO));
  writer->addDocument(doc);
  doc->push_back(newTextField(L"text", L"foobar", Field::Store::NO));
  writer->addDocument(doc);
  doc->push_back(newTextField(L"text", L"foobaz", Field::Store::NO));
  writer->addDocument(doc);
  doc->push_back(newTextField(L"text", L"fobar", Field::Store::NO));
  writer->addDocument(doc);

  shared_ptr<IndexReader> ir = writer->getReader();

  shared_ptr<DirectSpellChecker> spellChecker =
      make_shared<DirectSpellChecker>();
  spellChecker->setMaxQueryFrequency(0.0F);
  std::deque<std::shared_ptr<SuggestWord>> similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"text", L"fobar"), 1, ir,
                                   SuggestMode::SUGGEST_MORE_POPULAR);
  assertEquals(0, similar.size());

  spellChecker = make_shared<DirectSpellChecker>(); // reset defaults
  spellChecker->setMinQueryLength(5);
  similar = spellChecker->suggestSimilar(make_shared<Term>(L"text", L"foba"), 1,
                                         ir, SuggestMode::SUGGEST_MORE_POPULAR);
  assertEquals(0, similar.size());

  spellChecker = make_shared<DirectSpellChecker>(); // reset defaults
  spellChecker->setMaxEdits(1);
  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"text", L"foobazzz"), 1,
                                   ir, SuggestMode::SUGGEST_MORE_POPULAR);
  assertEquals(0, similar.size());

  spellChecker = make_shared<DirectSpellChecker>(); // reset defaults
  spellChecker->setAccuracy(0.9F);
  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"text", L"foobazzz"), 1,
                                   ir, SuggestMode::SUGGEST_MORE_POPULAR);
  assertEquals(0, similar.size());

  spellChecker = make_shared<DirectSpellChecker>(); // reset defaults
  spellChecker->setMinPrefix(0);
  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"text", L"roobaz"), 1, ir,
                                   SuggestMode::SUGGEST_MORE_POPULAR);
  assertEquals(1, similar.size());
  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"text", L"roobaz"), 1, ir,
                                   SuggestMode::SUGGEST_MORE_POPULAR);

  spellChecker = make_shared<DirectSpellChecker>(); // reset defaults
  spellChecker->setMinPrefix(1);
  similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"text", L"roobaz"), 1, ir,
                                   SuggestMode::SUGGEST_MORE_POPULAR);
  assertEquals(0, similar.size());

  spellChecker = make_shared<DirectSpellChecker>(); // reset defaults
  spellChecker->setMaxEdits(2);
  similar = spellChecker->suggestSimilar(make_shared<Term>(L"text", L"fobar"),
                                         2, ir, SuggestMode::SUGGEST_ALWAYS);
  assertEquals(2, similar.size());

  IOUtils::close({ir, writer, dir, analyzer});
}

void TestDirectSpellChecker::testBogusField() 
{
  shared_ptr<DirectSpellChecker> spellChecker =
      make_shared<DirectSpellChecker>();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"numbers", English::intToEnglish(i), Field::Store::NO));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = writer->getReader();

  std::deque<std::shared_ptr<SuggestWord>> similar =
      spellChecker->suggestSimilar(
          make_shared<Term>(L"bogusFieldBogusField", L"fvie"), 2, ir,
          SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(0, similar.size());

  IOUtils::close({ir, writer, dir, analyzer});
}

void TestDirectSpellChecker::testTransposition() 
{
  shared_ptr<DirectSpellChecker> spellChecker =
      make_shared<DirectSpellChecker>();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"numbers", English::intToEnglish(i), Field::Store::NO));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = writer->getReader();

  std::deque<std::shared_ptr<SuggestWord>> similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"fvie"), 1,
                                   ir, SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(1, similar.size());
  assertEquals(L"five", similar[0]->string);

  IOUtils::close({ir, writer, dir, analyzer});
}

void TestDirectSpellChecker::testTransposition2() 
{
  shared_ptr<DirectSpellChecker> spellChecker =
      make_shared<DirectSpellChecker>();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);

  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"numbers", English::intToEnglish(i), Field::Store::NO));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = writer->getReader();

  std::deque<std::shared_ptr<SuggestWord>> similar =
      spellChecker->suggestSimilar(make_shared<Term>(L"numbers", L"seevntene"),
                                   2, ir,
                                   SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(1, similar.size());
  assertEquals(L"seventeen", similar[0]->string);

  IOUtils::close({ir, writer, dir, analyzer});
}
} // namespace org::apache::lucene::search::spell