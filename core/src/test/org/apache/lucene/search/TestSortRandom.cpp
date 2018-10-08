using namespace std;

#include "TestSortRandom.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSortRandom::testRandomStringSort() 
{
  testRandomStringSort(SortField::Type::STRING);
}

void TestSortRandom::testRandomStringValSort() 
{
  testRandomStringSort(SortField::Type::STRING_VAL);
}

void TestSortRandom::testRandomStringSort(SortField::Type type) throw(
    runtime_error)
{
  shared_ptr<Random> random =
      make_shared<Random>(TestSortRandom::random()->nextLong());

  constexpr int NUM_DOCS = atLeast(100);
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const writer =
      make_shared<RandomIndexWriter>(random, dir);
  constexpr bool allowDups = random->nextBoolean();
  shared_ptr<Set<wstring>> *const seen = unordered_set<wstring>();
  constexpr int maxLength = TestUtil::nextInt(random, 5, 100);
  if (VERBOSE) {
    wcout << L"TEST: NUM_DOCS=" << NUM_DOCS << L" maxLength=" << maxLength
          << L" allowDups=" << allowDups << endl;
  }

  int numDocs = 0;
  const deque<std::shared_ptr<BytesRef>> docValues =
      deque<std::shared_ptr<BytesRef>>();
  // TODO: deletions
  while (numDocs < NUM_DOCS) {
    shared_ptr<Document> *const doc = make_shared<Document>();

    // 10% of the time, the document is missing the value:
    shared_ptr<BytesRef> *const br;
    if (TestSortRandom::random()->nextInt(10) != 7) {
      const wstring s;
      if (random->nextBoolean()) {
        s = TestUtil::randomSimpleString(random, maxLength);
      } else {
        s = TestUtil::randomUnicodeString(random, maxLength);
      }

      if (!allowDups) {
        if (seen->contains(s)) {
          continue;
        }
        seen->add(s);
      }

      if (VERBOSE) {
        wcout << L"  " << numDocs << L": s=" << s << endl;
      }

      br = make_shared<BytesRef>(s);
      doc->push_back(make_shared<SortedDocValuesField>(L"stringdv", br));
      docValues.push_back(br);

    } else {
      br.reset();
      if (VERBOSE) {
        wcout << L"  " << numDocs << L": <missing>" << endl;
      }
      docValues.push_back(nullptr);
    }

    doc->push_back(make_shared<NumericDocValuesField>(L"id", numDocs));
    doc->push_back(make_shared<StoredField>(L"id", numDocs));
    writer->addDocument(doc);
    numDocs++;

    if (random->nextInt(40) == 17) {
      // force flush
      writer->getReader()->close();
    }
  }

  shared_ptr<IndexReader> *const r = writer->getReader();
  delete writer;
  if (VERBOSE) {
    wcout << L"  reader=" << r << endl;
  }

  shared_ptr<IndexSearcher> *const s = newSearcher(r, false);
  constexpr int ITERS = atLeast(100);
  for (int iter = 0; iter < ITERS; iter++) {
    constexpr bool reverse = random->nextBoolean();

    shared_ptr<TopFieldDocs> *const hits;
    shared_ptr<SortField> *const sf;
    constexpr bool sortMissingLast;
    sf = make_shared<SortField>(L"stringdv", type, reverse);
    sortMissingLast = TestSortRandom::random()->nextBoolean();

    if (sortMissingLast) {
      sf->setMissingValue(SortField::STRING_LAST);
    }

    shared_ptr<Sort> *const sort;
    if (random->nextBoolean()) {
      sort = make_shared<Sort>(sf);
    } else {
      sort = make_shared<Sort>(sf, SortField::FIELD_DOC);
    }
    constexpr int hitCount = TestUtil::nextInt(random, 1, r->maxDoc() + 20);
    shared_ptr<RandomQuery> *const f = make_shared<RandomQuery>(
        random->nextLong(), random->nextFloat(), docValues);
    hits = s->search(f, hitCount, sort, random->nextBoolean(),
                     random->nextBoolean());

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" " << hits->totalHits
            << L" hits; topN=" << hitCount << L"; reverse=" << reverse
            << L"; sortMissingLast=" << sortMissingLast << L" sort=" << sort
            << endl;
    }

    // Compute expected results:
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(f.matchValues, new
    // java.util.Comparator<org.apache.lucene.util.BytesRef>()
    sort(f->matchValues.begin(), f->matchValues.end(),
         make_shared<ComparatorAnonymousInnerClass>(shared_from_this(),
                                                    sortMissingLast));

    if (reverse) {
      reverse(f->matchValues.begin(), f->matchValues.end());
    }
    const deque<std::shared_ptr<BytesRef>> &expected = f->matchValues;
    if (VERBOSE) {
      wcout << L"  expected:" << endl;
      for (int idx = 0; idx < expected.size(); idx++) {
        shared_ptr<BytesRef> br = expected[idx];
        wcout << L"    " << idx << L": "
              << (br == nullptr ? L"<missing>" : br->utf8ToString()) << endl;
        if (idx == hitCount - 1) {
          break;
        }
      }
    }

    if (VERBOSE) {
      wcout << L"  actual:" << endl;
      for (int hitIDX = 0; hitIDX < hits->scoreDocs.size(); hitIDX++) {
        shared_ptr<FieldDoc> *const fd =
            std::static_pointer_cast<FieldDoc>(hits->scoreDocs[hitIDX]);
        shared_ptr<BytesRef> br =
            any_cast<std::shared_ptr<BytesRef>>(fd->fields[0]);

        wcout << L"    " << hitIDX << L": "
              << (br == nullptr ? L"<missing>" : br->utf8ToString()) << L" id="
              << s->doc(fd->doc)[L"id"] << endl;
      }
    }
    for (int hitIDX = 0; hitIDX < hits->scoreDocs.size(); hitIDX++) {
      shared_ptr<FieldDoc> *const fd =
          std::static_pointer_cast<FieldDoc>(hits->scoreDocs[hitIDX]);
      shared_ptr<BytesRef> br = expected[hitIDX];

      shared_ptr<BytesRef> br2 =
          any_cast<std::shared_ptr<BytesRef>>(fd->fields[0]);

      TestUtil::assertEquals(br, br2);
    }
  }

  delete r;
  delete dir;
}

TestSortRandom::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestSortRandom> outerInstance, bool sortMissingLast)
{
  this->outerInstance = outerInstance;
  this->sortMissingLast = sortMissingLast;
}

int TestSortRandom::ComparatorAnonymousInnerClass::compare(
    shared_ptr<BytesRef> a, shared_ptr<BytesRef> b)
{
  if (a == nullptr) {
    if (b == nullptr) {
      return 0;
    }
    if (sortMissingLast) {
      return 1;
    } else {
      return -1;
    }
  } else if (b == nullptr) {
    if (sortMissingLast) {
      return -1;
    } else {
      return 1;
    }
  } else {
    return a->compareTo(b);
  }
}

TestSortRandom::RandomQuery::RandomQuery(
    int64_t seed, float density, deque<std::shared_ptr<BytesRef>> &docValues)
    : seed(seed), docValues(docValues)
{
  this->density = density;
}

shared_ptr<Weight>
TestSortRandom::RandomQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                          bool needsScores,
                                          float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

TestSortRandom::RandomQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<RandomQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
TestSortRandom::RandomQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Random> random =
      make_shared<Random>(context->docBase ^ outerInstance->seed);
  constexpr int maxDoc = context->reader()->maxDoc();
  shared_ptr<NumericDocValues> *const idSource =
      DocValues::getNumeric(context->reader(), L"id");
  assertNotNull(idSource);
  shared_ptr<FixedBitSet> *const bits = make_shared<FixedBitSet>(maxDoc);
  for (int docID = 0; docID < maxDoc; docID++) {
    TestUtil::assertEquals(docID, idSource->nextDoc());
    if (random->nextFloat() <= outerInstance->density) {
      bits->set(docID);
      // System.out.println("  acc id=" + idSource.getInt(docID) + " docID=" +
      // docID);
      outerInstance->matchValues.push_back(
          outerInstance->docValues[static_cast<int>(idSource->longValue())]);
    }
  }

  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<BitSetIterator>(bits, bits->approximateCardinality()));
}

bool TestSortRandom::RandomQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

wstring TestSortRandom::RandomQuery::toString(const wstring &field)
{
  return L"RandomFilter(density=" + to_wstring(density) + L")";
}

bool TestSortRandom::RandomQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TestSortRandom::RandomQuery::equalsTo(shared_ptr<RandomQuery> other)
{
  return seed == other->seed && docValues == other->docValues;
}

int TestSortRandom::RandomQuery::hashCode()
{
  int h = Objects::hash(seed, density);
  h = 31 * h + System::identityHashCode(docValues);
  h = 31 * h + classHash();
  return h;
}
} // namespace org::apache::lucene::search