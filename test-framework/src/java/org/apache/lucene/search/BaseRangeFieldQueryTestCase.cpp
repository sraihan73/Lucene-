using namespace std;

#include "BaseRangeFieldQueryTestCase.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using MultiFields = org::apache::lucene::index::MultiFields;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

int BaseRangeFieldQueryTestCase::dimension()
{
  return random()->nextInt(4) + 1;
}

void BaseRangeFieldQueryTestCase::testRandomTiny() 
{
  // Make sure single-leaf-node case is OK:
  for (int i = 0; i < 10; ++i) {
    doTestRandom(10, false);
  }
}

void BaseRangeFieldQueryTestCase::testRandomMedium() 
{
  doTestRandom(10000, false);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomBig() throws Exception
void BaseRangeFieldQueryTestCase::testRandomBig() 
{
  doTestRandom(200000, false);
}

void BaseRangeFieldQueryTestCase::testMultiValued() 
{
  doTestRandom(10000, true);
}

void BaseRangeFieldQueryTestCase::doTestRandom(
    int count, bool multiValued) 
{
  int numDocs = atLeast(count);
  int dimensions = dimension();

  if (VERBOSE) {
    wcout << L"TEST: numDocs=" << numDocs << endl;
  }

  std::deque<std::deque<std::shared_ptr<Range>>> ranges(numDocs);

  bool haveRealDoc = true;

  for (int id = 0; id < numDocs; ++id) {
    int x = random()->nextInt(20);
    if (ranges[id].empty()) {
      ranges[id] = std::deque<std::shared_ptr<Range>>{nextRange(dimensions)};
    }
    if (x == 17) {
      // some docs don't have a box:
      ranges[id][0]->isMissing = true;
      if (VERBOSE) {
        wcout << L"  id=" << id << L" is missing" << endl;
      }
      continue;
    }

    if (multiValued == true && random()->nextBoolean()) {
      // randomly add multi valued documents (up to 2 fields)
      int n = random()->nextInt(2) + 1;
      ranges[id] = std::deque<std::shared_ptr<Range>>(n);
      for (int i = 0; i < n; ++i) {
        ranges[id][i] = nextRange(dimensions);
      }
    }

    if (id > 0 && x < 9 && haveRealDoc) {
      int oldID;
      int i = 0;
      // don't step on missing ranges:
      while (true) {
        oldID = random()->nextInt(id);
        if (ranges[oldID][0]->isMissing == false) {
          break;
        } else if (++i > id) {
          goto nextdocContinue;
        }
      }

      if (x == dimensions * 2) {
        // Fully identical box (use first box in case current is multivalued but
        // old is not)
        for (int d = 0; d < dimensions; ++d) {
          ranges[id][0]->setMin(d, ranges[oldID][0]->getMin(d));
          ranges[id][0]->setMax(d, ranges[oldID][0]->getMax(d));
        }
        if (VERBOSE) {
          wcout << L"  id=" << id << L" box=" << ranges[id]
                << L" (same box as doc=" << oldID << L")" << endl;
        }
      } else {
        for (int m = 0, even = dimensions % 2; m < dimensions * 2; ++m) {
          if (x == m) {
            int d = static_cast<int>(floor(m / 2));
            // current could be multivalue but old may not be, so use first box
            if (even == 0) { // even is min
              ranges[id][0]->setMin(d, ranges[oldID][0]->getMin(d));
              if (VERBOSE) {
                wcout << L"  id=" << id << L" box=" << ranges[id]
                      << L" (same min[" << d << L"] as doc=" << oldID << L")"
                      << endl;
              }
            } else { // odd is max
              ranges[id][0]->setMax(d, ranges[oldID][0]->getMax(d));
              if (VERBOSE) {
                wcout << L"  id=" << id << L" box=" << ranges[id]
                      << L" (same max[" << d << L"] as doc=" << oldID << L")"
                      << endl;
              }
            }
          }
        }
      }
    }
  nextdocContinue:;
  }
nextdocBreak:
  verify(ranges);
}

void BaseRangeFieldQueryTestCase::verify(
    std::deque<std::deque<std::shared_ptr<Range>>>
        &ranges) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  // Else we can get O(N^2) merging
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < ranges.size() / 100) {
    iwc->setMaxBufferedDocs(ranges.size() / 100);
  }
  shared_ptr<Directory> dir;
  if (ranges.size() > 50000) {
    dir = newFSDirectory(createTempDir(getClass().getSimpleName()));
  } else {
    dir = newDirectory();
  }

  shared_ptr<Set<int>> deleted = unordered_set<int>();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int id = 0; id < ranges.size(); ++id) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    if (ranges[id][0]->isMissing == false) {
      for (int n = 0; n < ranges[id].size(); ++n) {
        addRange(doc, ranges[id][n]);
      }
    }
    w->addDocument(doc);
    if (id > 0 && random()->nextInt(100) == 1) {
      int idToDelete = random()->nextInt(id);
      w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(idToDelete))});
      deleted->add(idToDelete);
      if (VERBOSE) {
        wcout << L"  delete id=" << idToDelete << endl;
      }
    }
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);

  int dimensions = ranges[0][0]->numDimensions();
  int iters = atLeast(25);
  shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(s->getIndexReader());
  int maxDoc = s->getIndexReader()->maxDoc();

  for (int iter = 0; iter < iters; ++iter) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" s=" << s << endl;
    }

    // occasionally test open ended bounding ranges
    shared_ptr<Range> queryRange = nextRange(dimensions);
    int rv = random()->nextInt(4);
    shared_ptr<Query> query;
    Range::QueryType queryType;
    if (rv == 0) {
      queryType = Range::QueryType::INTERSECTS;
      query = newIntersectsQuery(queryRange);
    } else if (rv == 1) {
      queryType = Range::QueryType::CONTAINS;
      query = newContainsQuery(queryRange);
    } else if (rv == 2) {
      queryType = Range::QueryType::WITHIN;
      query = newWithinQuery(queryRange);
    } else {
      queryType = Range::QueryType::CROSSES;
      query = newCrossesQuery(queryRange);
    }

    if (VERBOSE) {
      wcout << L"  query=" << query << endl;
    }

    shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(maxDoc);
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(
                         shared_from_this(), hits));

    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");
    for (int docID = 0; docID < maxDoc; ++docID) {
      assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      bool expected;
      if (liveDocs != nullptr && liveDocs->get(docID) == false) {
        // document is deleted
        expected = false;
      } else if (ranges[id][0]->isMissing) {
        expected = false;
      } else {
        expected = expectedResult(queryRange, ranges[id], queryType);
      }

      if (hits->get(docID) != expected) {
        shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
        b->append(L"FAIL (iter " + to_wstring(iter) + L"): ");
        if (expected == true) {
          b->append(L"id=" + to_wstring(id) +
                    (ranges[id].size() > 1 ? L" (MultiValue) " : L" ") +
                    L"should match but did not\n");
        } else {
          b->append(L"id=" + to_wstring(id) + L" should not match but did\n");
        }
        b->append(L" queryRange=" + queryRange + L"\n");
        b->append(L" box" + ((ranges[id].size() > 1) ? L"es=" : L"=") +
                  ranges[id][0]);
        for (int n = 1; n < ranges[id].size(); ++n) {
          b->append(L", ");
          b->append(ranges[id][n]);
        }
        b->append(L"\n queryType=" + queryType + L"\n");
        b->append(L" deleted?=" +
                  StringHelper::toString(liveDocs != nullptr &&
                                         liveDocs->get(docID) == false));
        fail(L"wrong hit (first of possibly more):\n\n" + b);
      }
    }
  }
  IOUtils::close({r, dir});
}

BaseRangeFieldQueryTestCase::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<BaseRangeFieldQueryTestCase> outerInstance,
        shared_ptr<FixedBitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

void BaseRangeFieldQueryTestCase::SimpleCollectorAnonymousInnerClass::collect(
    int doc)
{
  hits->set(docBase + doc);
}

void BaseRangeFieldQueryTestCase::SimpleCollectorAnonymousInnerClass::
    doSetNextReader(shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

bool BaseRangeFieldQueryTestCase::SimpleCollectorAnonymousInnerClass::
    needsScores()
{
  return false;
}

void BaseRangeFieldQueryTestCase::addRange(shared_ptr<Document> doc,
                                           shared_ptr<Range> box)
{
  doc->push_back(newRangeField(box));
}

bool BaseRangeFieldQueryTestCase::expectedResult(
    shared_ptr<Range> queryRange, std::deque<std::shared_ptr<Range>> &range,
    Range::QueryType queryType)
{
  for (int i = 0; i < range.size(); ++i) {
    if (expectedBBoxQueryResult(queryRange, range[i], queryType) == true) {
      return true;
    }
  }
  return false;
}

bool BaseRangeFieldQueryTestCase::expectedBBoxQueryResult(
    shared_ptr<Range> queryRange, shared_ptr<Range> range,
    Range::QueryType queryType)
{
  if (queryRange->isEqual(range) && queryType != Range::QueryType::CROSSES) {
    return true;
  }
  Range::QueryType relation = range->relate(queryRange);
  if (queryType == Range::QueryType::INTERSECTS) {
    return relation != nullptr;
  } else if (queryType == Range::QueryType::CROSSES) {
    // by definition, RangeFields that CONTAIN the query are also considered to
    // cross
    return relation == queryType || relation == Range::QueryType::CONTAINS;
  }
  return relation == queryType;
}

BaseRangeFieldQueryTestCase::Range::QueryType
BaseRangeFieldQueryTestCase::Range::relate(shared_ptr<Range> other)
{
  if (isDisjoint(other)) {
    // if disjoint; return null:
    return nullptr;
  } else if (isWithin(other)) {
    return QueryType::WITHIN;
  } else if (contains(other)) {
    return QueryType::CONTAINS;
  }
  return QueryType::CROSSES;
}
} // namespace org::apache::lucene::search