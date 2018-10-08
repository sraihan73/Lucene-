using namespace std;

#include "TestIntervals.h"

namespace org::apache::lucene::search::intervals
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
std::deque<wstring> TestIntervals::field1_docs = {
    L"Nothing of interest to anyone here",
    L"Pease porridge hot, pease porridge cold, pease porridge in the pot nine "
    L"days old.  Some like it hot, some like it cold, some like it in the pot "
    L"nine days old",
    L"Pease porridge cold, pease porridge hot, pease porridge in the pot "
    L"twelve days old.  Some like it cold, some like it hot, some like it in "
    L"the fraggle",
    L"Nor here, nowt hot going on in pease this one",
    L"Pease porridge hot, pease porridge cold, pease porridge in the pot nine "
    L"years old.  Some like it hot, some like it twelve",
    L"Porridge is great"};
std::deque<wstring> TestIntervals::field2_docs = {
    L"In Xanadu did Kubla Khan a stately pleasure dome decree",
    L"Where Alph the sacred river ran through caverns measureless to man",
    L"Down to a sunless sea",
    L"So thrice five miles of fertile ground",
    L"Pease hot porridge porridge",
    L"Pease porridge porridge hot"};
shared_ptr<org::apache::lucene::store::Directory> TestIntervals::directory;
shared_ptr<org::apache::lucene::search::IndexSearcher> TestIntervals::searcher;
shared_ptr<org::apache::lucene::analysis::Analyzer> TestIntervals::analyzer =
    make_shared<org::apache::lucene::analysis::standard::StandardAnalyzer>(
        org::apache::lucene::analysis::CharArraySet::EMPTY_SET);

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setupIndex() throws
// java.io.IOException
void TestIntervals::setupIndex() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory, newIndexWriterConfig(analyzer));
  for (int i = 0; i < field1_docs.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<TextField>(L"field1", field1_docs[i], Field::Store::NO));
    doc->push_back(
        make_shared<TextField>(L"field2", field2_docs[i], Field::Store::NO));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    writer->addDocument(doc);
  }
  delete writer;
  searcher = make_shared<IndexSearcher>(DirectoryReader::open(directory));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void teardownIndex() throws
// java.io.IOException
void TestIntervals::teardownIndex() 
{
  IOUtils::close({searcher->getIndexReader(), directory});
}

void TestIntervals::checkIntervals(
    shared_ptr<IntervalsSource> source, const wstring &field,
    int expectedMatchCount,
    std::deque<std::deque<int>> &expected) 
{
  int matchedDocs = 0;
  for (auto ctx : searcher->getIndexReader()->leaves()) {
    assertNull(source->intervals(field + L"fake", ctx));
    shared_ptr<NumericDocValues> ids =
        DocValues::getNumeric(ctx->reader(), L"id");
    shared_ptr<IntervalIterator> intervals = source->intervals(field, ctx);
    if (intervals == nullptr) {
      continue;
    }
    for (int doc = 0; doc < ctx->reader()->maxDoc(); doc++) {
      ids->advance(doc);
      int id = static_cast<int>(ids->longValue());
      if (intervals->docID() == doc ||
          (intervals->docID() < doc && intervals->advance(doc) == doc)) {
        int i = 0, pos;
        assertEquals(-1, intervals->start());
        assertEquals(-1, intervals->end());
        while ((pos = intervals->nextInterval()) !=
               IntervalIterator::NO_MORE_INTERVALS) {
          // System.out.println(doc + ": " + intervals);
          assertEquals(L"Wrong start value", expected[id][i], pos);
          assertEquals(L"start() != pos returned from nextInterval()",
                       expected[id][i], intervals->start());
          assertEquals(L"Wrong end value", expected[id][i + 1],
                       intervals->end());
          i += 2;
        }
        assertEquals(L"Wrong number of endpoints", expected[id].size(), i);
        if (i > 0) {
          matchedDocs++;
        }
      } else {
        assertEquals(0, expected[id].size());
      }
    }
  }
  assertEquals(expectedMatchCount, matchedDocs);
}

void TestIntervals::testIntervalsOnFieldWithNoPositions() 
{
  invalid_argument e = expectThrows(invalid_argument::typeid, [&]() {
    Intervals::term(L"wibble")->intervals(
        L"id", searcher->getIndexReader()->leaves()[0]);
  });
  assertEquals(L"Cannot create an IntervalIterator over field id because it "
               L"has no indexed positions",
               e.what());
}

void TestIntervals::testTermQueryIntervals() 
{
  checkIntervals(Intervals::term(L"porridge"), L"field1", 4,
                 std::deque<std::deque<int>>{
                     std::deque<int>(), std::deque<int>{1, 1, 4, 4, 7, 7},
                     std::deque<int>{1, 1, 4, 4, 7, 7}, std::deque<int>(),
                     std::deque<int>{1, 1, 4, 4, 7, 7},
                     std::deque<int>{0, 0}});
}

void TestIntervals::testOrderedNearIntervals() 
{
  checkIntervals(
      Intervals::ordered({Intervals::term(L"pease"), Intervals::term(L"hot")}),
      L"field1", 3,
      std::deque<std::deque<int>>{
          std::deque<int>(), std::deque<int>{0, 2, 6, 17},
          std::deque<int>{3, 5, 6, 21}, std::deque<int>(),
          std::deque<int>{0, 2, 6, 17}, std::deque<int>()});
}

void TestIntervals::testPhraseIntervals() 
{
  checkIntervals(Intervals::phrase({L"pease", L"porridge"}), L"field1", 3,
                 std::deque<std::deque<int>>{
                     std::deque<int>(), std::deque<int>{0, 1, 3, 4, 6, 7},
                     std::deque<int>{0, 1, 3, 4, 6, 7}, std::deque<int>(),
                     std::deque<int>{0, 1, 3, 4, 6, 7}, std::deque<int>()});
}

void TestIntervals::testUnorderedNearIntervals() 
{
  checkIntervals(Intervals::unordered(
                     {Intervals::term(L"pease"), Intervals::term(L"hot")}),
                 L"field1", 4,
                 std::deque<std::deque<int>>{
                     std::deque<int>(), std::deque<int>{0, 2, 2, 3, 6, 17},
                     std::deque<int>{3, 5, 5, 6, 6, 21},
                     std::deque<int>{3, 7},
                     std::deque<int>{0, 2, 2, 3, 6, 17}, std::deque<int>()});
}

void TestIntervals::testIntervalDisjunction() 
{
  checkIntervals(
      Intervals:: or ({Intervals::term(L"pease"), Intervals::term(L"hot"),
                       Intervals::term(L"notMatching")}),
      L"field1", 4,
      std::deque<std::deque<int>>{
          std::deque<int>(), std::deque<int>{0, 0, 2, 2, 3, 3, 6, 6, 17, 17},
          std::deque<int>{0, 0, 3, 3, 5, 5, 6, 6, 21, 21},
          std::deque<int>{3, 3, 7, 7},
          std::deque<int>{0, 0, 2, 2, 3, 3, 6, 6, 17, 17},
          std::deque<int>()});
}

void TestIntervals::testNesting() 
{
  checkIntervals(
      Intervals::unordered(
          {Intervals::term(L"pease"), Intervals::term(L"porridge"),
           Intervals:: or
               ({Intervals::term(L"hot"), Intervals::term(L"cold")})}),
      L"field1", 3,
      std::deque<std::deque<int>>{
          std::deque<int>(),
          std::deque<int>{0, 2, 1, 3, 2, 4, 3, 5, 4, 6, 5, 7, 6, 17},
          std::deque<int>{0, 2, 1, 3, 2, 4, 3, 5, 4, 6, 5, 7, 6, 17},
          std::deque<int>(),
          std::deque<int>{0, 2, 1, 3, 2, 4, 3, 5, 4, 6, 5, 7, 6, 17},
          std::deque<int>()});
}

void TestIntervals::testNesting2() 
{
  checkIntervals(
      Intervals::unordered(
          Intervals::ordered({Intervals::term(L"like"), Intervals::term(L"it"),
                              Intervals::term(L"cold")}),
          Intervals::term(L"pease")),
      L"field1", 2,
      std::deque<std::deque<int>>{std::deque<int>(), std::deque<int>{6, 21},
                                    std::deque<int>{6, 17}, std::deque<int>(),
                                    std::deque<int>(), std::deque<int>()});
}

void TestIntervals::testUnorderedDistinct() 
{
  checkIntervals(Intervals::unordered(false, Intervals::term(L"pease"),
                                      Intervals::term(L"pease")),
                 L"field1", 3,
                 std::deque<std::deque<int>>{
                     std::deque<int>(), std::deque<int>{0, 3, 3, 6},
                     std::deque<int>{0, 3, 3, 6}, std::deque<int>(),
                     std::deque<int>{0, 3, 3, 6}, std::deque<int>()});
  checkIntervals(
      Intervals::unordered(false,
                           Intervals::unordered({Intervals::term(L"pease"),
                                                 Intervals::term(L"porridge"),
                                                 Intervals::term(L"hot")}),
                           Intervals::term(L"porridge")),
      L"field1", 3,
      std::deque<std::deque<int>>{
          std::deque<int>(), std::deque<int>{1, 4, 4, 17},
          std::deque<int>{1, 5, 4, 7}, std::deque<int>(),
          std::deque<int>{1, 4, 4, 17}, std::deque<int>()});
  checkIntervals(
      Intervals::unordered(false,
                           Intervals::unordered({Intervals::term(L"pease"),
                                                 Intervals::term(L"porridge"),
                                                 Intervals::term(L"hot")}),
                           Intervals::term(L"porridge")),
      L"field2", 1,
      std::deque<std::deque<int>>{
          std::deque<int>(), std::deque<int>(), std::deque<int>(),
          std::deque<int>(), std::deque<int>{0, 3}, std::deque<int>()});
}
} // namespace org::apache::lucene::search::intervals