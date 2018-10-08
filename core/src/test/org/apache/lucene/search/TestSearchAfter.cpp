using namespace std;

#include "TestSearchAfter.h"

namespace org::apache::lucene::search
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSearchAfter::setUp() 
{
  LuceneTestCase::setUp();

  allSortFields =
      deque<>(Arrays::asList(std::deque<std::shared_ptr<SortField>>{
          make_shared<SortField>(L"int", SortField::Type::INT, false),
          make_shared<SortField>(L"long", SortField::Type::LONG, false),
          make_shared<SortField>(L"float", SortField::Type::FLOAT, false),
          make_shared<SortField>(L"double", SortField::Type::DOUBLE, false),
          make_shared<SortField>(L"bytes", SortField::Type::STRING, false),
          make_shared<SortField>(L"bytesval", SortField::Type::STRING_VAL,
                                 false),
          make_shared<SortField>(L"int", SortField::Type::INT, true),
          make_shared<SortField>(L"long", SortField::Type::LONG, true),
          make_shared<SortField>(L"float", SortField::Type::FLOAT, true),
          make_shared<SortField>(L"double", SortField::Type::DOUBLE, true),
          make_shared<SortField>(L"bytes", SortField::Type::STRING, true),
          make_shared<SortField>(L"bytesval", SortField::Type::STRING_VAL,
                                 true),
          SortField::FIELD_SCORE, SortField::FIELD_DOC}));

  // Also test missing first / last for the "string" sorts:
  for (auto field : std::deque<wstring>{L"bytes", L"sortedbytesdocvalues"}) {
    for (int rev = 0; rev < 2; rev++) {
      bool reversed = rev == 0;
      shared_ptr<SortField> sf =
          make_shared<SortField>(field, SortField::Type::STRING, reversed);
      sf->setMissingValue(SortField::STRING_FIRST);
      allSortFields.push_back(sf);

      sf = make_shared<SortField>(field, SortField::Type::STRING, reversed);
      sf->setMissingValue(SortField::STRING_LAST);
      allSortFields.push_back(sf);
    }
  }

  // Also test missing first / last for the "string_val" sorts:
  for (auto field : std::deque<wstring>{L"sortedbytesdocvaluesval",
                                         L"straightbytesdocvalues"}) {
    for (int rev = 0; rev < 2; rev++) {
      bool reversed = rev == 0;
      shared_ptr<SortField> sf =
          make_shared<SortField>(field, SortField::Type::STRING_VAL, reversed);
      sf->setMissingValue(SortField::STRING_FIRST);
      allSortFields.push_back(sf);

      sf = make_shared<SortField>(field, SortField::Type::STRING_VAL, reversed);
      sf->setMissingValue(SortField::STRING_LAST);
      allSortFields.push_back(sf);
    }
  }

  int limit = allSortFields.size();
  for (int i = 0; i < limit; i++) {
    shared_ptr<SortField> sf = allSortFields[i];
    if (sf->getType() == SortField::Type::INT) {
      shared_ptr<SortField> sf2 = make_shared<SortField>(
          sf->getField(), SortField::Type::INT, sf->getReverse());
      sf2->setMissingValue(random()->nextInt());
      allSortFields.push_back(sf2);
    } else if (sf->getType() == SortField::Type::LONG) {
      shared_ptr<SortField> sf2 = make_shared<SortField>(
          sf->getField(), SortField::Type::LONG, sf->getReverse());
      sf2->setMissingValue(random()->nextLong());
      allSortFields.push_back(sf2);
    } else if (sf->getType() == SortField::Type::FLOAT) {
      shared_ptr<SortField> sf2 = make_shared<SortField>(
          sf->getField(), SortField::Type::FLOAT, sf->getReverse());
      sf2->setMissingValue(random()->nextFloat());
      allSortFields.push_back(sf2);
    } else if (sf->getType() == SortField::Type::DOUBLE) {
      shared_ptr<SortField> sf2 = make_shared<SortField>(
          sf->getField(), SortField::Type::DOUBLE, sf->getReverse());
      sf2->setMissingValue(random()->nextDouble());
      allSortFields.push_back(sf2);
    }
  }

  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  int numDocs = atLeast(200);
  shared_ptr<Random> r = random();
  for (int i = 0; i < numDocs; i++) {
    deque<std::shared_ptr<Field>> fields = deque<std::shared_ptr<Field>>();
    fields.push_back(
        newTextField(L"english", English::intToEnglish(i), Field::Store::NO));
    fields.push_back(newTextField(L"oddeven", (i % 2 == 0) ? L"even" : L"odd",
                                  Field::Store::NO));
    fields.push_back(make_shared<NumericDocValuesField>(
        L"byte", static_cast<char>(r->nextInt())));
    fields.push_back(make_shared<NumericDocValuesField>(
        L"short", static_cast<short>(r->nextInt())));
    fields.push_back(make_shared<NumericDocValuesField>(L"int", r->nextInt()));
    fields.push_back(
        make_shared<NumericDocValuesField>(L"long", r->nextLong()));
    fields.push_back(
        make_shared<FloatDocValuesField>(L"float", r->nextFloat()));
    fields.push_back(
        make_shared<DoubleDocValuesField>(L"double", r->nextDouble()));
    fields.push_back(make_shared<SortedDocValuesField>(
        L"bytes", make_shared<BytesRef>(
                      TestUtil::randomRealisticUnicodeString(random()))));
    fields.push_back(make_shared<BinaryDocValuesField>(
        L"bytesval", make_shared<BytesRef>(
                         TestUtil::randomRealisticUnicodeString(random()))));

    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<StoredField>(L"id", L"" + to_wstring(i)));
    if (VERBOSE) {
      wcout << L"  add doc id=" << i << endl;
    }
    for (auto field : fields) {
      // So we are sometimes missing that field:
      if (random()->nextInt(5) != 4) {
        document->push_back(field);
        if (VERBOSE) {
          wcout << L"    " << field << endl;
        }
      }
    }

    iw->addDocument(document);

    if (random()->nextInt(50) == 17) {
      iw->commit();
    }
  }
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);
  if (VERBOSE) {
    wcout << L"  searcher=" << searcher << endl;
  }
}

void TestSearchAfter::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestSearchAfter::testQueries() 
{
  // because the first page has a null 'after', we get a normal collector.
  // so we need to run the test a few times to ensure we will collect multiple
  // pages.
  int n = atLeast(20);
  for (int i = 0; i < n; i++) {
    assertQuery(make_shared<MatchAllDocsQuery>(), nullptr);
    assertQuery(make_shared<TermQuery>(make_shared<Term>(L"english", L"one")),
                nullptr);
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"english", L"one")),
            BooleanClause::Occur::SHOULD);
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"oddeven", L"even")),
            BooleanClause::Occur::SHOULD);
    assertQuery(bq->build(), nullptr);
  }
}

void TestSearchAfter::assertQuery(shared_ptr<Query> query) 
{
  assertQuery(query, nullptr);
  assertQuery(query, Sort::RELEVANCE);
  assertQuery(query, Sort::INDEXORDER);
  for (auto sortField : allSortFields) {
    assertQuery(query, make_shared<Sort>(
                           std::deque<std::shared_ptr<SortField>>{sortField}));
  }
  for (int i = 0; i < 20; i++) {
    assertQuery(query, getRandomSort());
  }
}

shared_ptr<Sort> TestSearchAfter::getRandomSort()
{
  std::deque<std::shared_ptr<SortField>> sortFields(
      TestUtil::nextInt(random(), 2, 7));
  for (int i = 0; i < sortFields.size(); i++) {
    sortFields[i] = allSortFields[random()->nextInt(allSortFields.size())];
  }
  return make_shared<Sort>(sortFields);
}

void TestSearchAfter::assertQuery(shared_ptr<Query> query,
                                  shared_ptr<Sort> sort) 
{
  int maxDoc = searcher->getIndexReader()->maxDoc();
  shared_ptr<TopDocs> all;
  int pageSize = TestUtil::nextInt(random(), 1, maxDoc * 2);
  if (VERBOSE) {
    wcout << L"\nassertQuery " << (iter++) << L": query=" << query << L" sort="
          << sort << L" pageSize=" << pageSize << endl;
  }
  constexpr bool doMaxScore = random()->nextBoolean();
  constexpr bool doScores = random()->nextBoolean();
  if (sort == nullptr) {
    all = searcher->search(query, maxDoc);
  } else if (sort == Sort::RELEVANCE) {
    all = searcher->search(query, maxDoc, sort, true, doMaxScore);
  } else {
    all = searcher->search(query, maxDoc, sort, doScores, doMaxScore);
  }
  if (VERBOSE) {
    wcout << L"  all.totalHits=" << all->totalHits << endl;
    int upto = 0;
    for (auto scoreDoc : all->scoreDocs) {
      wcout << L"    hit " << (upto++) << L": id="
            << searcher->doc(scoreDoc->doc)[L"id"] << L" " << scoreDoc << endl;
    }
  }
  int pageStart = 0;
  shared_ptr<ScoreDoc> lastBottom = nullptr;
  while (pageStart < all->totalHits) {
    shared_ptr<TopDocs> paged;
    if (sort == nullptr) {
      if (VERBOSE) {
        wcout << L"  iter lastBottom=" << lastBottom << endl;
      }
      paged = searcher->searchAfter(lastBottom, query, pageSize);
    } else {
      if (VERBOSE) {
        wcout << L"  iter lastBottom=" << lastBottom << endl;
      }
      if (sort == Sort::RELEVANCE) {
        paged = searcher->searchAfter(lastBottom, query, pageSize, sort, true,
                                      doMaxScore);
      } else {
        paged = searcher->searchAfter(lastBottom, query, pageSize, sort,
                                      doScores, doMaxScore);
      }
    }
    if (VERBOSE) {
      wcout << L"    " << paged->scoreDocs.size() << L" hits on page" << endl;
    }

    if (paged->scoreDocs.empty()) {
      break;
    }
    assertPage(pageStart, all, paged);
    pageStart += paged->scoreDocs.size();
    lastBottom = paged->scoreDocs[paged->scoreDocs.size() - 1];
  }
  TestUtil::assertEquals(all->scoreDocs.size(), pageStart);
}

void TestSearchAfter::assertPage(int pageStart, shared_ptr<TopDocs> all,
                                 shared_ptr<TopDocs> paged) 
{
  TestUtil::assertEquals(all->totalHits, paged->totalHits);
  for (int i = 0; i < paged->scoreDocs.size(); i++) {
    shared_ptr<ScoreDoc> sd1 = all->scoreDocs[pageStart + i];
    shared_ptr<ScoreDoc> sd2 = paged->scoreDocs[i];
    if (VERBOSE) {
      wcout << L"    hit " << (pageStart << i) << endl;
      wcout << L"      expected id=" << searcher->doc(sd1->doc)[L"id"] << L" "
            << sd1 << endl;
      wcout << L"        actual id=" << searcher->doc(sd2->doc)[L"id"] << L" "
            << sd2 << endl;
    }
    TestUtil::assertEquals(sd1->doc, sd2->doc);
    assertEquals(sd1->score, sd2->score, 0.0f);
    if (std::dynamic_pointer_cast<FieldDoc>(sd1) != nullptr) {
      assertTrue(std::dynamic_pointer_cast<FieldDoc>(sd2) != nullptr);
      TestUtil::assertEquals((std::static_pointer_cast<FieldDoc>(sd1))->fields,
                             (std::static_pointer_cast<FieldDoc>(sd2))->fields);
    }
  }
}
} // namespace org::apache::lucene::search