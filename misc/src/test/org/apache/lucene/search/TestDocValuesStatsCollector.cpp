using namespace std;

#include "TestDocValuesStatsCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using DoubleDocValuesStats =
    org::apache::lucene::search::DocValuesStats::DoubleDocValuesStats;
using LongDocValuesStats =
    org::apache::lucene::search::DocValuesStats::LongDocValuesStats;
using SortedDocValuesStats =
    org::apache::lucene::search::DocValuesStats::SortedDocValuesStats;
using SortedDoubleDocValuesStats =
    org::apache::lucene::search::DocValuesStats::SortedDoubleDocValuesStats;
using SortedLongDocValuesStats =
    org::apache::lucene::search::DocValuesStats::SortedLongDocValuesStats;
using SortedSetDocValuesStats =
    org::apache::lucene::search::DocValuesStats::SortedSetDocValuesStats;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocValuesStatsCollector::testNoDocsWithField() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    for (int i = 0; i < numDocs; i++) {
      indexWriter->addDocument(make_shared<Document>());
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<LongDocValuesStats> stats =
          make_shared<LongDocValuesStats>(L"foo");
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      TestUtil::assertEquals(0, stats->count());
      TestUtil::assertEquals(numDocs, stats->missing());
    }
  }
}

void TestDocValuesStatsCollector::testOneDoc() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"numeric";
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(field, 1));
    doc->push_back(make_shared<StringField>(L"id", L"doc1", Store::NO));
    indexWriter->addDocument(doc);

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<LongDocValuesStats> stats =
          make_shared<LongDocValuesStats>(field);
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      TestUtil::assertEquals(1, stats->count());
      TestUtil::assertEquals(0, stats->missing());
      TestUtil::assertEquals(1, stats->max().longValue());
      TestUtil::assertEquals(1, stats->min().longValue());
      TestUtil::assertEquals(1, stats->sum().value());
      assertEquals(1, stats->mean(), 0.0001);
      assertEquals(0, stats->variance(), 0.0001);
      assertEquals(0, stats->stdev(), 0.0001);
    }
  }
}

void TestDocValuesStatsCollector::testDocsWithLongValues() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"numeric";
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    std::deque<int64_t> docValues(numDocs);
    int nextVal = 1;
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      if (random()->nextBoolean()) { // not all documents have a value
        doc->push_back(make_shared<NumericDocValuesField>(field, nextVal));
        doc->push_back(
            make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
        docValues[i] = nextVal;
        ++nextVal;
      }
      indexWriter->addDocument(doc);
    }

    // 20% of cases delete some docs
    if (random()->nextDouble() < 0.2) {
      for (int i = 0; i < numDocs; i++) {
        if (random()->nextBoolean()) {
          indexWriter->deleteDocuments(
              {make_shared<Term>(L"id", L"doc" + to_wstring(i))});
          docValues[i] = 0;
        }
      }
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<LongDocValuesStats> stats =
          make_shared<LongDocValuesStats>(field);
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      int expCount = static_cast<int>(Arrays::stream(docValues)
                                          .filter([&](any v) { return v > 0; })
                                          .count());
      TestUtil::assertEquals(expCount, stats->count());
      int numDocsWithoutField =
          static_cast<int>(getZeroValues(docValues)->count());
      TestUtil::assertEquals(
          computeExpMissing(numDocsWithoutField, numDocs, reader),
          stats->missing());
      if (stats->count() > 0) {
        shared_ptr<LongSummaryStatistics> sumStats =
            getPositiveValues(docValues)->summaryStatistics();
        TestUtil::assertEquals(sumStats->getMax(), stats->max().longValue());
        TestUtil::assertEquals(sumStats->getMin(), stats->min().longValue());
        assertEquals(sumStats->getAverage(), stats->mean(), 0.00001);
        TestUtil::assertEquals(sumStats->getSum(), stats->sum().value());
        double variance =
            computeVariance(docValues, stats->mean_, stats->count());
        assertEquals(variance, stats->variance(), 0.00001);
        assertEquals(sqrt(variance), stats->stdev(), 0.00001);
      }
    }
  }
}

void TestDocValuesStatsCollector::testDocsWithDoubleValues() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"numeric";
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    std::deque<double> docValues(numDocs);
    double nextVal = 1.0;
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      if (random()->nextBoolean()) { // not all documents have a value
        doc->push_back(make_shared<DoubleDocValuesField>(field, nextVal));
        doc->push_back(
            make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
        docValues[i] = nextVal;
        ++nextVal;
      }
      indexWriter->addDocument(doc);
    }

    // 20% of cases delete some docs
    if (random()->nextDouble() < 0.2) {
      for (int i = 0; i < numDocs; i++) {
        if (random()->nextBoolean()) {
          indexWriter->deleteDocuments(
              {make_shared<Term>(L"id", L"doc" + to_wstring(i))});
          docValues[i] = 0;
        }
      }
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<DoubleDocValuesStats> stats =
          make_shared<DoubleDocValuesStats>(field);
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      int expCount = static_cast<int>(Arrays::stream(docValues)
                                          .filter([&](any v) { return v > 0; })
                                          .count());
      TestUtil::assertEquals(expCount, stats->count());
      int numDocsWithoutField =
          static_cast<int>(getZeroValues(docValues)->count());
      TestUtil::assertEquals(
          computeExpMissing(numDocsWithoutField, numDocs, reader),
          stats->missing());
      if (stats->count() > 0) {
        shared_ptr<DoubleSummaryStatistics> sumStats =
            getPositiveValues(docValues)->summaryStatistics();
        assertEquals(sumStats->getMax(), stats->max().doubleValue(), 0.00001);
        assertEquals(sumStats->getMin(), stats->min().doubleValue(), 0.00001);
        assertEquals(sumStats->getAverage(), stats->mean(), 0.00001);
        assertEquals(sumStats->getSum(), stats->sum(), 0.00001);
        double variance =
            computeVariance(docValues, stats->mean_, stats->count());
        assertEquals(variance, stats->variance(), 0.00001);
        assertEquals(sqrt(variance), stats->stdev(), 0.00001);
      }
    }
  }
}

void TestDocValuesStatsCollector::testDocsWithMultipleLongValues() throw(
    IOException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"numeric";
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    std::deque<std::deque<int64_t>> docValues(numDocs);
    int64_t nextVal = 1;
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      if (random()->nextBoolean()) { // not all documents have a value
        int numValues = TestUtil::nextInt(random(), 1, 5);
        docValues[i] = std::deque<int64_t>(numValues);
        for (int j = 0; j < numValues; j++) {
          doc->push_back(
              make_shared<SortedNumericDocValuesField>(field, nextVal));
          docValues[i][j] = nextVal;
          ++nextVal;
        }
        doc->push_back(
            make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
      }
      indexWriter->addDocument(doc);
    }

    // 20% of cases delete some docs
    if (random()->nextDouble() < 0.2) {
      for (int i = 0; i < numDocs; i++) {
        if (random()->nextBoolean()) {
          indexWriter->deleteDocuments(
              {make_shared<Term>(L"id", L"doc" + to_wstring(i))});
          docValues[i].clear();
        }
      }
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<SortedLongDocValuesStats> stats =
          make_shared<SortedLongDocValuesStats>(field);
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      TestUtil::assertEquals(nonNull(docValues)->count(), stats->count());
      int numDocsWithoutField = static_cast<int>(isNull(docValues)->count());
      TestUtil::assertEquals(
          computeExpMissing(numDocsWithoutField, numDocs, reader),
          stats->missing());
      if (stats->count() > 0) {
        shared_ptr<LongSummaryStatistics> sumStats =
            filterAndFlatValues(docValues, [&](v) { return v != nullptr; })
                ->summaryStatistics();
        TestUtil::assertEquals(sumStats->getMax(), stats->max().longValue());
        TestUtil::assertEquals(sumStats->getMin(), stats->min().longValue());
        assertEquals(sumStats->getAverage(), stats->mean(), 0.00001);
        TestUtil::assertEquals(sumStats->getSum(), stats->sum().value());
        TestUtil::assertEquals(sumStats->getCount(), stats->valuesCount());
        double variance = computeVariance(
            filterAndFlatValues(docValues, [&](v) { return v != nullptr; }),
            stats->mean_, stats->count());
        assertEquals(variance, stats->variance(), 0.00001);
        assertEquals(sqrt(variance), stats->stdev(), 0.00001);
      }
    }
  }
}

void TestDocValuesStatsCollector::testDocsWithMultipleDoubleValues() throw(
    IOException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"numeric";
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    std::deque<std::deque<double>> docValues(numDocs);
    double nextVal = 1;
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      if (random()->nextBoolean()) { // not all documents have a value
        int numValues = TestUtil::nextInt(random(), 1, 5);
        docValues[i] = std::deque<double>(numValues);
        for (int j = 0; j < numValues; j++) {
          doc->push_back(make_shared<SortedNumericDocValuesField>(
              field, Double::doubleToRawLongBits(nextVal)));
          docValues[i][j] = nextVal;
          ++nextVal;
        }
        doc->push_back(
            make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
      }
      indexWriter->addDocument(doc);
    }

    // 20% of cases delete some docs
    if (random()->nextDouble() < 0.2) {
      for (int i = 0; i < numDocs; i++) {
        if (random()->nextBoolean()) {
          indexWriter->deleteDocuments(
              {make_shared<Term>(L"id", L"doc" + to_wstring(i))});
          docValues[i].clear();
        }
      }
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<SortedDoubleDocValuesStats> stats =
          make_shared<SortedDoubleDocValuesStats>(field);
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      TestUtil::assertEquals(nonNull(docValues)->count(), stats->count());
      int numDocsWithoutField = static_cast<int>(isNull(docValues)->count());
      TestUtil::assertEquals(
          computeExpMissing(numDocsWithoutField, numDocs, reader),
          stats->missing());
      if (stats->count() > 0) {
        shared_ptr<DoubleSummaryStatistics> sumStats =
            filterAndFlatValues(docValues, [&](v) { return v != nullptr; })
                ->summaryStatistics();
        assertEquals(sumStats->getMax(), stats->max().longValue(), 0.00001);
        assertEquals(sumStats->getMin(), stats->min().longValue(), 0.00001);
        assertEquals(sumStats->getAverage(), stats->mean(), 0.00001);
        assertEquals(sumStats->getSum(), stats->sum().value(), 0.00001);
        TestUtil::assertEquals(sumStats->getCount(), stats->valuesCount());
        double variance = computeVariance(
            filterAndFlatValues(docValues, [&](v) { return v != nullptr; }),
            stats->mean_, stats->count());
        assertEquals(variance, stats->variance(), 0.00001);
        assertEquals(sqrt(variance), stats->stdev(), 0.00001);
      }
    }
  }
}

void TestDocValuesStatsCollector::testDocsWithSortedValues() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"sorted";
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    std::deque<std::shared_ptr<BytesRef>> docValues(numDocs);
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      if (random()->nextBoolean()) { // not all documents have a value
        shared_ptr<BytesRef> val = TestUtil::randomBinaryTerm(random());
        doc->push_back(make_shared<SortedDocValuesField>(field, val));
        doc->push_back(
            make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
        docValues[i] = val;
      }
      indexWriter->addDocument(doc);
    }

    // 20% of cases delete some docs
    if (random()->nextDouble() < 0.2) {
      for (int i = 0; i < numDocs; i++) {
        if (random()->nextBoolean()) {
          indexWriter->deleteDocuments(
              {make_shared<Term>(L"id", L"doc" + to_wstring(i))});
          docValues[i].reset();
        }
      }
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<SortedDocValuesStats> stats =
          make_shared<SortedDocValuesStats>(field);
      searcher->search(make_shared<MatchAllDocsQuery>(),
                       make_shared<DocValuesStatsCollector>(stats));

      int expCount = static_cast<int>(nonNull(docValues)->count());
      TestUtil::assertEquals(expCount, stats->count());
      int numDocsWithoutField = static_cast<int>(isNull(docValues)->count());
      TestUtil::assertEquals(
          computeExpMissing(numDocsWithoutField, numDocs, reader),
          stats->missing());
      if (stats->count() > 0) {
        TestUtil::assertEquals(
            nonNull(docValues)->min(BytesRef::compareTo)->get(), stats->min());
        TestUtil::assertEquals(
            nonNull(docValues)->max(BytesRef::compareTo)->get(), stats->max());
      }
    }
  }
}

void TestDocValuesStatsCollector::testDocsWithSortedSetValues() throw(
    IOException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); org.apache.lucene.index.IndexWriter indexWriter = new
  // org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
    wstring field = L"sorted";
    int numDocs = TestUtil::nextInt(random(), 1, 100);
    std::deque<std::deque<std::shared_ptr<BytesRef>>> docValues(numDocs);
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      if (random()->nextBoolean()) { // not all documents have a value
        int numValues = TestUtil::nextInt(random(), 1, 5);
        docValues[i] = std::deque<std::shared_ptr<BytesRef>>(numValues);
        for (int j = 0; j < numValues; j++) {
          shared_ptr<BytesRef> val = TestUtil::randomBinaryTerm(random());
          doc->push_back(make_shared<SortedSetDocValuesField>(field, val));
          docValues[i][j] = val;
        }
        doc->push_back(
            make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
      }
      indexWriter->addDocument(doc);
    }

    // 20% of cases delete some docs
    if (random()->nextDouble() < 0.2) {
      for (int i = 0; i < numDocs; i++) {
        if (random()->nextBoolean()) {
          indexWriter->deleteDocuments(
              {make_shared<Term>(L"id", L"doc" + to_wstring(i))});
          docValues[i].clear();
        }
      }
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.DirectoryReader
    // reader = org.apache.lucene.index.DirectoryReader.open(indexWriter))
    {
      org::apache::lucene::index::DirectoryReader reader =
          org::apache::lucene::index::DirectoryReader::open(indexWriter);
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<SortedSetDocValuesStats> stats =
          make_shared<SortedSetDocValuesStats>(field);
      shared_ptr<TotalHitCountCollector> totalHitCount =
          make_shared<TotalHitCountCollector>();
      searcher->search(
          make_shared<MatchAllDocsQuery>(),
          MultiCollector::wrap(
              {totalHitCount, make_shared<DocValuesStatsCollector>(stats)}));

      int expCount = static_cast<int>(nonNull(docValues)->count());
      TestUtil::assertEquals(expCount, stats->count());
      int numDocsWithoutField = static_cast<int>(isNull(docValues)->count());
      TestUtil::assertEquals(
          computeExpMissing(numDocsWithoutField, numDocs, reader),
          stats->missing());
      if (stats->count() > 0) {
        TestUtil::assertEquals(nonNull(docValues)
                                   ->flatMap(Arrays::stream)
                                   .min(BytesRef::compareTo)
                                   ->get(),
                               stats->min());
        TestUtil::assertEquals(nonNull(docValues)
                                   ->flatMap(Arrays::stream)
                                   .max(BytesRef::compareTo)
                                   ->get(),
                               stats->max());
      }
    }
  }
}

shared_ptr<LongStream>
TestDocValuesStatsCollector::getPositiveValues(std::deque<int64_t> &values)
{
  return Arrays::stream(values).filter([&](any v) { return v > 0; });
}

shared_ptr<DoubleStream>
TestDocValuesStatsCollector::getPositiveValues(std::deque<double> &values)
{
  return Arrays::stream(values).filter([&](any v) { return v > 0; });
}

shared_ptr<LongStream>
TestDocValuesStatsCollector::getZeroValues(std::deque<int64_t> &values)
{
  return Arrays::stream(values).filter([&](any v) { return v == 0; });
}

shared_ptr<DoubleStream>
TestDocValuesStatsCollector::getZeroValues(std::deque<double> &values)
{
  return Arrays::stream(values).filter([&](any v) { return v == 0; });
}

double
TestDocValuesStatsCollector::computeVariance(std::deque<int64_t> &values,
                                             double mean, int count)
{
  return getPositiveValues(values)
             ->mapToDouble([&](any v) { return (v - mean) * (v - mean); })
             .sum() /
         count;
}

double TestDocValuesStatsCollector::computeVariance(std::deque<double> &values,
                                                    double mean, int count)
{
  return getPositiveValues(values)
             ->map_obj([&](any v) { return (v - mean) * (v - mean); })
             .sum() /
         count;
}

template <>
shared_ptr<LongStream> TestDocValuesStatsCollector::filterAndFlatValues(
    std::deque<std::deque<int64_t>> &values, function<T1> p)
{
  return nonNull(values)->flatMapToLong(Arrays::stream);
}

template <>
shared_ptr<DoubleStream> TestDocValuesStatsCollector::filterAndFlatValues(
    std::deque<std::deque<double>> &values, function<T1> p)
{
  return nonNull(values)->flatMapToDouble(Arrays::stream);
}

double
TestDocValuesStatsCollector::computeVariance(shared_ptr<LongStream> values,
                                             double mean, int count)
{
  return values->mapToDouble([&](any v) { return (v - mean) * (v - mean); })
             .sum() /
         count;
}

double
TestDocValuesStatsCollector::computeVariance(shared_ptr<DoubleStream> values,
                                             double mean, int count)
{
  return values->map_obj([&](any v) { return (v - mean) * (v - mean); }).sum() /
         count;
}

template <typename T>
shared_ptr<Stream<T>>
TestDocValuesStatsCollector::nonNull(std::deque<T> &values)
{
  return filterValues(values, Objects::nonNull);
}

template <typename T>
shared_ptr<Stream<T>>
TestDocValuesStatsCollector::isNull(std::deque<T> &values)
{
  return filterValues(values, Objects::isNull);
}

template <typename T, typename T1>
shared_ptr<Stream<T>>
TestDocValuesStatsCollector::filterValues(std::deque<T> &values,
                                          function<T1> p)
{
  return Arrays::stream(values).filter(p);
}

int TestDocValuesStatsCollector::computeExpMissing(
    int numDocsWithoutField, int numIndexedDocs, shared_ptr<IndexReader> reader)
{
  // The number of missing documents equals the number of docs without the field
  // (not indexed with it, or were deleted). However, in case we deleted all
  // documents in a segment before the reader was opened, there will be a
  // mismatch between numDocs (how many we indexed) to reader.maxDoc(), so
  // compensate for that.
  return numDocsWithoutField - reader->numDeletedDocs() -
         (numIndexedDocs - reader->maxDoc());
}
} // namespace org::apache::lucene::search