using namespace std;

#include "TestMatchesIterator.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestMatchesIterator::FIELD_WITH_OFFSETS = L"field_offsets";
const wstring TestMatchesIterator::FIELD_NO_OFFSETS = L"field_no_offsets";
const wstring TestMatchesIterator::FIELD_DOCS_ONLY = L"field_docs_only";
const wstring TestMatchesIterator::FIELD_FREQS = L"field_freqs";
const shared_ptr<org::apache::lucene::document::FieldType>
    TestMatchesIterator::OFFSETS =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);

TestMatchesIterator::StaticConstructor::StaticConstructor()
{
  OFFSETS->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  DOCS->setIndexOptions(IndexOptions::DOCS);
  DOCS_AND_FREQS->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
}

TestMatchesIterator::StaticConstructor TestMatchesIterator::staticConstructor;
const shared_ptr<org::apache::lucene::document::FieldType>
    TestMatchesIterator::DOCS =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    TestMatchesIterator::DOCS_AND_FREQS =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);

void TestMatchesIterator::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestMatchesIterator::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  for (int i = 0; i < docFields.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(FIELD_WITH_OFFSETS, docFields[i], OFFSETS));
    doc->push_back(
        newField(FIELD_NO_OFFSETS, docFields[i], TextField::TYPE_STORED));
    doc->push_back(newField(FIELD_DOCS_ONLY, docFields[i], DOCS));
    doc->push_back(newField(FIELD_FREQS, docFields[i], DOCS_AND_FREQS));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        newField(L"id", Integer::toString(i), TextField::TYPE_STORED));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(getOnlyLeafReader(reader));
}

void TestMatchesIterator::checkMatches(
    shared_ptr<Query> q, const wstring &field,
    std::deque<std::deque<int>> &expected) 
{
  shared_ptr<Weight> w = searcher->createWeight(searcher->rewrite(q), false, 1);
  for (int i = 0; i < expected.size(); i++) {
    shared_ptr<LeafReaderContext> ctx =
        searcher->leafContexts[ReaderUtil::subIndex(expected[i][0],
                                                    searcher->leafContexts)];
    int doc = expected[i][0] - ctx->docBase;
    shared_ptr<Matches> matches = w->matches(ctx, doc);
    if (matches->empty()) {
      assertEquals(expected[i].size(), 1);
      continue;
    }
    shared_ptr<MatchesIterator> it = matches->getMatches(field);
    if (expected[i].size() == 1) {
      assertNull(it);
      return;
    }
    checkFieldMatches(it, expected[i]);
    checkFieldMatches(matches->getMatches(field),
                      expected[i]); // test multiple calls
  }
}

void TestMatchesIterator::checkFieldMatches(
    shared_ptr<MatchesIterator> it,
    std::deque<int> &expected) 
{
  int pos = 1;
  while (it->next()) {
    // System.out.println(expected[i][pos] + "->" + expected[i][pos + 1] + "[" +
    // expected[i][pos + 2] + "->" + expected[i][pos + 3] + "]");
    assertEquals(L"Wrong start position", expected[pos], it->startPosition());
    assertEquals(L"Wrong end position", expected[pos + 1], it->endPosition());
    assertEquals(L"Wrong start offset", expected[pos + 2], it->startOffset());
    assertEquals(L"Wrong end offset", expected[pos + 3], it->endOffset());
    pos += 4;
  }
  assertEquals(expected.size(), pos);
}

void TestMatchesIterator::checkNoPositionsMatches(
    shared_ptr<Query> q, const wstring &field,
    std::deque<bool> &expected) 
{
  shared_ptr<Weight> w = searcher->createWeight(searcher->rewrite(q), false, 1);
  for (int i = 0; i < expected.size(); i++) {
    shared_ptr<LeafReaderContext> ctx =
        searcher->leafContexts[ReaderUtil::subIndex(i, searcher->leafContexts)];
    int doc = i - ctx->docBase;
    shared_ptr<Matches> matches = w->matches(ctx, doc);
    if (expected[i]) {
      shared_ptr<MatchesIterator> mi = matches->getMatches(field);
      assertNull(mi);
    } else {
      assertNull(matches);
    }
  }
}

void TestMatchesIterator::testTermQuery() 
{
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w1"));
  checkMatches(
      q, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{
          std::deque<int>{0, 0, 0, 0, 2}, std::deque<int>{1, 0, 0, 0, 2},
          std::deque<int>{2, 0, 0, 0, 2},
          std::deque<int>{3, 0, 0, 0, 2, 2, 2, 6, 8}, std::deque<int>{4}});
}

void TestMatchesIterator::testTermQueryNoStoredOffsets() 
{
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(FIELD_NO_OFFSETS, L"w1"));
  checkMatches(q, FIELD_NO_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0, 0, 0, -1, -1},
                   std::deque<int>{1, 0, 0, -1, -1},
                   std::deque<int>{2, 0, 0, -1, -1},
                   std::deque<int>{3, 0, 0, -1, -1, 2, 2, -1, -1},
                   std::deque<int>{4}});
}

void TestMatchesIterator::testTermQueryNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_DOCS_ONLY, FIELD_FREQS}) {
    shared_ptr<Query> q =
        make_shared<TermQuery>(make_shared<Term>(field, L"w1"));
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, true, true, true, false});
  }
}

void TestMatchesIterator::testDisjunction() 
{
  shared_ptr<Query> w1 =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w1"));
  shared_ptr<Query> w3 =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w3"));
  shared_ptr<Query> q = (make_shared<BooleanQuery::Builder>())
                            ->add(w1, BooleanClause::Occur::SHOULD)
                            ->add(w3, BooleanClause::Occur::SHOULD)
                            ->build();
  checkMatches(q, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0, 0, 0, 0, 2, 2, 2, 6, 8},
                   std::deque<int>{1, 0, 0, 0, 2, 1, 1, 3, 5, 3, 3, 9, 11},
                   std::deque<int>{2, 0, 0, 0, 2},
                   std::deque<int>{3, 0, 0, 0, 2, 2, 2, 6, 8, 5, 5, 15, 17},
                   std::deque<int>{4}});
}

void TestMatchesIterator::testDisjunctionNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_DOCS_ONLY, FIELD_FREQS}) {
    shared_ptr<Query> q =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w1")),
                  BooleanClause::Occur::SHOULD)
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
                  BooleanClause::Occur::SHOULD)
            ->build();
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, true, true, true, false});
  }
}

void TestMatchesIterator::testReqOpt() 
{
  shared_ptr<Query> q =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"w1")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"w3")),
                BooleanClause::Occur::MUST)
          ->build();
  checkMatches(q, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0, 0, 0, 0, 2, 2, 2, 6, 8},
                   std::deque<int>{1, 0, 0, 0, 2, 1, 1, 3, 5, 3, 3, 9, 11},
                   std::deque<int>{2},
                   std::deque<int>{3, 0, 0, 0, 2, 2, 2, 6, 8, 5, 5, 15, 17},
                   std::deque<int>{4}});
}

void TestMatchesIterator::testReqOptNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_DOCS_ONLY, FIELD_FREQS}) {
    shared_ptr<Query> q =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w1")),
                  BooleanClause::Occur::SHOULD)
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
                  BooleanClause::Occur::MUST)
            ->build();
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, true, false, true, false});
  }
}

void TestMatchesIterator::testMinShouldMatch() 
{
  shared_ptr<Query> w1 =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w1"));
  shared_ptr<Query> w3 =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w3"));
  shared_ptr<Query> w4 =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w4"));
  shared_ptr<Query> xx =
      make_shared<TermQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"xx"));
  shared_ptr<Query> q = (make_shared<BooleanQuery::Builder>())
                            ->add(w3, BooleanClause::Occur::SHOULD)
                            ->add(make_shared<BooleanQuery::Builder>()
                                      .add(w1, BooleanClause::Occur::SHOULD)
                                      ->add(w4, BooleanClause::Occur::SHOULD)
                                      ->add(xx, BooleanClause::Occur::SHOULD)
                                      .setMinimumNumberShouldMatch(2)
                                      .build(),
                                  BooleanClause::Occur::SHOULD)
                            .build();
  checkMatches(q, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0, 0, 0, 0, 2, 2, 2, 6, 8, 3, 3, 9, 11},
                   std::deque<int>{1, 1, 1, 3, 5, 3, 3, 9, 11},
                   std::deque<int>{2, 0, 0, 0, 2, 1, 1, 3, 5, 4, 4, 12, 14},
                   std::deque<int>{3, 0, 0, 0, 2, 2, 2, 6, 8, 3, 3, 9, 11, 5,
                                    5, 15, 17},
                   std::deque<int>{4}});
}

void TestMatchesIterator::testMinShouldMatchNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_FREQS, FIELD_DOCS_ONLY}) {
    shared_ptr<Query> q =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
                  BooleanClause::Occur::SHOULD)
            ->add(make_shared<BooleanQuery::Builder>()
                      .add(make_shared<TermQuery>(
                               make_shared<Term>(field, L"w1")),
                           BooleanClause::Occur::SHOULD)
                      ->add(make_shared<TermQuery>(
                                make_shared<Term>(field, L"w4")),
                            BooleanClause::Occur::SHOULD)
                      ->add(make_shared<TermQuery>(
                                make_shared<Term>(field, L"xx")),
                            BooleanClause::Occur::SHOULD)
                      .setMinimumNumberShouldMatch(2)
                      .build(),
                  BooleanClause::Occur::SHOULD)
            .build();
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, true, true, true, false});
  }
}

void TestMatchesIterator::testExclusion() 
{
  shared_ptr<Query> q =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"w3")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"zz")),
                BooleanClause::Occur::MUST_NOT)
          ->build();
  checkMatches(q, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0, 2, 2, 6, 8}, std::deque<int>{1},
                   std::deque<int>{2}, std::deque<int>{3, 5, 5, 15, 17},
                   std::deque<int>{4}});
}

void TestMatchesIterator::testExclusionNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_FREQS, FIELD_DOCS_ONLY}) {
    shared_ptr<Query> q =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
                  BooleanClause::Occur::SHOULD)
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"zz")),
                  BooleanClause::Occur::MUST_NOT)
            ->build();
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, false, false, true, false});
  }
}

void TestMatchesIterator::testConjunction() 
{
  shared_ptr<Query> q =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"w3")),
                BooleanClause::Occur::MUST)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"w4")),
                BooleanClause::Occur::MUST)
          ->build();
  checkMatches(q, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0, 2, 2, 6, 8, 3, 3, 9, 11},
                   std::deque<int>{1}, std::deque<int>{2},
                   std::deque<int>{3, 3, 3, 9, 11, 5, 5, 15, 17},
                   std::deque<int>{4}});
}

void TestMatchesIterator::testConjunctionNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_FREQS, FIELD_DOCS_ONLY}) {
    shared_ptr<Query> q =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
                  BooleanClause::Occur::MUST)
            ->add(make_shared<TermQuery>(make_shared<Term>(field, L"w4")),
                  BooleanClause::Occur::MUST)
            ->build();
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, false, false, true, false});
  }
}

void TestMatchesIterator::testWildcards() 
{
  shared_ptr<Query> q =
      make_shared<PrefixQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"x"));
  checkMatches(
      q, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{std::deque<int>{0}, std::deque<int>{1},
                                    std::deque<int>{2, 1, 1, 3, 5},
                                    std::deque<int>{3}, std::deque<int>{4}});

  shared_ptr<Query> rq = make_shared<RegexpQuery>(
      make_shared<Term>(FIELD_WITH_OFFSETS, L"w[1-2]"));
  checkMatches(
      rq, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{
          std::deque<int>{0, 0, 0, 0, 2, 1, 1, 3, 5},
          std::deque<int>{1, 0, 0, 0, 2, 2, 2, 6, 8},
          std::deque<int>{2, 0, 0, 0, 2, 2, 2, 6, 8},
          std::deque<int>{3, 0, 0, 0, 2, 1, 1, 3, 5, 2, 2, 6, 8, 4, 4, 12, 14},
          std::deque<int>{4}});
}

void TestMatchesIterator::testNoMatchWildcards() 
{
  shared_ptr<Query> nomatch = make_shared<PrefixQuery>(
      make_shared<Term>(FIELD_WITH_OFFSETS, L"wibble"));
  shared_ptr<Matches> matches =
      searcher->createWeight(searcher->rewrite(nomatch), false, 1)
          ->matches(searcher->leafContexts[0], 0);
  assertNull(matches);
}

void TestMatchesIterator::testWildcardsNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_FREQS, FIELD_DOCS_ONLY}) {
    shared_ptr<Query> q =
        make_shared<PrefixQuery>(make_shared<Term>(field, L"x"));
    checkNoPositionsMatches(
        q, field, std::deque<bool>{false, false, true, false, false});
  }
}

void TestMatchesIterator::testSynonymQuery() 
{
  shared_ptr<Query> q =
      make_shared<SynonymQuery>(make_shared<Term>(FIELD_WITH_OFFSETS, L"w1"),
                                make_shared<Term>(FIELD_WITH_OFFSETS, L"w2"));
  checkMatches(
      q, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{
          std::deque<int>{0, 0, 0, 0, 2, 1, 1, 3, 5},
          std::deque<int>{1, 0, 0, 0, 2, 2, 2, 6, 8},
          std::deque<int>{2, 0, 0, 0, 2, 2, 2, 6, 8},
          std::deque<int>{3, 0, 0, 0, 2, 1, 1, 3, 5, 2, 2, 6, 8, 4, 4, 12, 14},
          std::deque<int>{4}});
}

void TestMatchesIterator::testSynonymQueryNoPositions() 
{
  for (auto field : std::deque<wstring>{FIELD_FREQS, FIELD_DOCS_ONLY}) {
    shared_ptr<Query> q = make_shared<SynonymQuery>(
        make_shared<Term>(field, L"w1"), make_shared<Term>(field, L"w2"));
    checkNoPositionsMatches(q, field,
                            std::deque<bool>{true, true, true, true, false});
  }
}

void TestMatchesIterator::testMultipleFields() 
{
  shared_ptr<Query> q =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"id", L"1")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(
                    make_shared<Term>(FIELD_WITH_OFFSETS, L"w3")),
                BooleanClause::Occur::MUST)
          ->build();
  shared_ptr<Weight> w = searcher->createWeight(searcher->rewrite(q), false, 1);

  shared_ptr<LeafReaderContext> ctx =
      searcher->leafContexts[ReaderUtil::subIndex(1, searcher->leafContexts)];
  shared_ptr<Matches> m = w->matches(ctx, 1 - ctx->docBase);
  assertNotNull(m);
  checkFieldMatches(m->getMatches(L"id"), std::deque<int>{-1, 0, 0, -1, -1});
  checkFieldMatches(m->getMatches(FIELD_WITH_OFFSETS),
                    std::deque<int>{-1, 1, 1, 3, 5, 3, 3, 9, 11});
  assertNull(m->getMatches(L"bogus"));

  shared_ptr<Set<wstring>> fields = unordered_set<wstring>();
  for (auto field : m) {
    fields->add(field);
  }
  assertEquals(2, fields->size());
  assertTrue(fields->contains(FIELD_WITH_OFFSETS));
  assertTrue(fields->contains(L"id"));
}

void TestMatchesIterator::testSloppyPhraseQuery() 
{
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(4, FIELD_WITH_OFFSETS, L"a", L"sentence");
  checkMatches(
      pq, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{
          std::deque<int>{0}, std::deque<int>{1}, std::deque<int>{2},
          std::deque<int>{3},
          std::deque<int>{4, 0, 2, 0, 17, 6, 9, 35, 59, 9, 11, 58, 75}});
}

void TestMatchesIterator::testExactPhraseQuery() 
{
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(FIELD_WITH_OFFSETS, L"phrase", L"sentence");
  checkMatches(
      pq, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{
          std::deque<int>{0}, std::deque<int>{1}, std::deque<int>{2},
          std::deque<int>{3},
          std::deque<int>{4, 1, 2, 2, 17, 5, 6, 28, 43, 10, 11, 60, 75}});

  shared_ptr<PhraseQuery> pq2 =
      (make_shared<PhraseQuery::Builder>())
          ->add(make_shared<Term>(FIELD_WITH_OFFSETS, L"a"))
          ->add(make_shared<Term>(FIELD_WITH_OFFSETS, L"sentence"), 2)
          ->build();
  checkMatches(pq2, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0}, std::deque<int>{1},
                   std::deque<int>{2}, std::deque<int>{3},
                   std::deque<int>{4, 0, 2, 0, 17, 9, 11, 58, 75}});
}

void TestMatchesIterator::testSloppyMultiPhraseQuery() 
{
  shared_ptr<MultiPhraseQuery> mpq =
      (make_shared<MultiPhraseQuery::Builder>())
          ->add(make_shared<Term>(FIELD_WITH_OFFSETS, L"phrase"))
          ->add(std::deque<std::shared_ptr<Term>>{
              make_shared<Term>(FIELD_WITH_OFFSETS, L"sentence"),
              make_shared<Term>(FIELD_WITH_OFFSETS, L"iterations")})
          ->setSlop(4)
          ->build();
  checkMatches(mpq, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0}, std::deque<int>{1},
                   std::deque<int>{2}, std::deque<int>{3},
                   std::deque<int>{4, 1, 2, 2, 17, 5, 7, 28, 54, 5, 7, 28, 54,
                                    10, 11, 60, 75}});
}

void TestMatchesIterator::testExactMultiPhraseQuery() 
{
  shared_ptr<MultiPhraseQuery> mpq =
      (make_shared<MultiPhraseQuery::Builder>())
          ->add(make_shared<Term>(FIELD_WITH_OFFSETS, L"sentence"))
          ->add(std::deque<std::shared_ptr<Term>>{
              make_shared<Term>(FIELD_WITH_OFFSETS, L"with"),
              make_shared<Term>(FIELD_WITH_OFFSETS, L"iterations")})
          ->build();
  checkMatches(mpq, FIELD_WITH_OFFSETS,
               std::deque<std::deque<int>>{
                   std::deque<int>{0}, std::deque<int>{1},
                   std::deque<int>{2}, std::deque<int>{3},
                   std::deque<int>{4, 2, 3, 9, 22, 6, 7, 35, 54}});

  shared_ptr<MultiPhraseQuery> mpq2 =
      (make_shared<MultiPhraseQuery::Builder>())
          ->add(std::deque<std::shared_ptr<Term>>{
              make_shared<Term>(FIELD_WITH_OFFSETS, L"a"),
              make_shared<Term>(FIELD_WITH_OFFSETS, L"many")})
          ->add(make_shared<Term>(FIELD_WITH_OFFSETS, L"phrase"))
          ->build();
  checkMatches(
      mpq2, FIELD_WITH_OFFSETS,
      std::deque<std::deque<int>>{
          std::deque<int>{0}, std::deque<int>{1}, std::deque<int>{2},
          std::deque<int>{3},
          std::deque<int>{4, 0, 1, 0, 8, 4, 5, 23, 34, 9, 10, 58, 66}});
}
} // namespace org::apache::lucene::search