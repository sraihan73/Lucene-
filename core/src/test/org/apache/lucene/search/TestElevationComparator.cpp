using namespace std;

#include "TestElevationComparator.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using Entry = org::apache::lucene::search::FieldValueHitQueue::Entry;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestElevationComparator::testSorting() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setMaxBufferedDocs(2)
                     ->setMergePolicy(newLogMergePolicy(1000))
                     ->setSimilarity(make_shared<ClassicSimilarity>()));
  writer->addDocument(adoc(
      std::deque<wstring>{L"id", L"a", L"title", L"ipod", L"str_s", L"a"}));
  writer->addDocument(adoc(std::deque<wstring>{L"id", L"b", L"title",
                                                L"ipod ipod", L"str_s", L"b"}));
  writer->addDocument(adoc(std::deque<wstring>{
      L"id", L"c", L"title", L"ipod ipod ipod", L"str_s", L"c"}));
  writer->addDocument(adoc(
      std::deque<wstring>{L"id", L"x", L"title", L"boosted", L"str_s", L"x"}));
  writer->addDocument(adoc(std::deque<wstring>{
      L"id", L"y", L"title", L"boosted boosted", L"str_s", L"y"}));
  writer->addDocument(adoc(std::deque<wstring>{
      L"id", L"z", L"title", L"boosted boosted boosted", L"str_s", L"z"}));

  shared_ptr<IndexReader> r = DirectoryReader::open(writer);
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  searcher->setSimilarity(make_shared<BM25Similarity>());

  runTest(searcher, true);
  runTest(searcher, false);

  delete r;
  delete directory;
}

void TestElevationComparator::runTest(shared_ptr<IndexSearcher> searcher,
                                      bool reversed) 
{

  shared_ptr<BooleanQuery::Builder> newq = make_shared<BooleanQuery::Builder>();
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(L"title", L"ipod"));

  newq->add(query, BooleanClause::Occur::SHOULD);
  newq->add(getElevatedQuery(std::deque<wstring>{L"id", L"a", L"id", L"x"}),
            BooleanClause::Occur::SHOULD);

  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(
          L"id", make_shared<ElevationComparatorSource>(priority), false),
      make_shared<SortField>(nullptr, SortField::Type::SCORE, reversed));

  shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> topCollector =
      TopFieldCollector::create(sort, 50, false, true, true, true);
  searcher->search(newq->build(), topCollector);

  shared_ptr<TopDocs> topDocs = topCollector->topDocs(0, 10);
  int nDocsReturned = topDocs->scoreDocs.size();

  assertEquals(4, nDocsReturned);

  // 0 & 3 were elevated
  assertEquals(0, topDocs->scoreDocs[0]->doc);
  assertEquals(3, topDocs->scoreDocs[1]->doc);

  if (reversed) {
    assertEquals(1, topDocs->scoreDocs[2]->doc);
    assertEquals(2, topDocs->scoreDocs[3]->doc);
  } else {
    assertEquals(2, topDocs->scoreDocs[2]->doc);
    assertEquals(1, topDocs->scoreDocs[3]->doc);
  }

  /*
  for (int i = 0; i < nDocsReturned; i++) {
   ScoreDoc scoreDoc = topDocs.scoreDocs[i];
   ids[i] = scoreDoc.doc;
   scores[i] = scoreDoc.score;
   documents[i] = searcher.doc(ids[i]);
   System.out.println("ids[i] = " + ids[i]);
   System.out.println("documents[i] = " + documents[i]);
   System.out.println("scores[i] = " + scores[i]);
 }
  */
}

shared_ptr<Query>
TestElevationComparator::getElevatedQuery(std::deque<wstring> &vals)
{
  shared_ptr<BooleanQuery::Builder> b = make_shared<BooleanQuery::Builder>();
  int max = (vals.size() / 2) + 5;
  for (int i = 0; i < vals.size() - 1; i += 2) {
    b->add(make_shared<TermQuery>(make_shared<Term>(vals[i], vals[i + 1])),
           BooleanClause::Occur::SHOULD);
    priority.emplace(make_shared<BytesRef>(vals[i + 1]),
                     static_cast<Integer>(max--));
    // System.out.println(" pri doc=" + vals[i+1] + " pri=" + (1+max));
  }
  shared_ptr<BooleanQuery> q = b->build();
  return make_shared<BoostQuery>(q, 0.0f);
}

shared_ptr<Document> TestElevationComparator::adoc(std::deque<wstring> &vals)
{
  shared_ptr<Document> doc = make_shared<Document>();
  for (int i = 0; i < vals.size() - 2; i += 2) {
    doc->push_back(newTextField(vals[i], vals[i + 1], Field::Store::YES));
    if (vals[i] == L"id") {
      doc->push_back(make_shared<SortedDocValuesField>(
          vals[i], make_shared<BytesRef>(vals[i + 1])));
    }
  }
  return doc;
}

ElevationComparatorSource::ElevationComparatorSource(
    unordered_map<std::shared_ptr<BytesRef>, int> &boosts)
    : priority(boosts)
{
}

shared_ptr<FieldComparator<int>> ElevationComparatorSource::newComparator(
    const wstring &fieldname, int const numHits, int sortPos, bool reversed)
{
  return make_shared<FieldComparatorAnonymousInnerClass>(shared_from_this(),
                                                         fieldname, numHits);
}

ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    FieldComparatorAnonymousInnerClass(
        shared_ptr<ElevationComparatorSource> outerInstance,
        const wstring &fieldname, int numHits)
{
  this->outerInstance = outerInstance;
  this->fieldname = fieldname;
  this->numHits = numHits;
  values = std::deque<int>(numHits);
}

shared_ptr<LeafFieldComparator>
ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    getLeafComparator(shared_ptr<LeafReaderContext> context) 
{
  return make_shared<LeafFieldComparatorAnonymousInnerClass>(shared_from_this(),
                                                             context);
}

ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::
        LeafFieldComparatorAnonymousInnerClass(
            shared_ptr<FieldComparatorAnonymousInnerClass> outerInstance,
            shared_ptr<LeafReaderContext> context)
{
  this->outerInstance = outerInstance;
  this->context = context;
}

void ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::setBottom(int slot)
{
  bottomVal = values[slot];
}

int ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::compareTop(int doc)
{
  throw make_shared<UnsupportedOperationException>();
}

int ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::docVal(int doc) 
{
  shared_ptr<SortedDocValues> idIndex =
      DocValues::getSorted(context->reader(), outerInstance->fieldname);
  if (idIndex->advance(doc) == doc) {
    shared_ptr<BytesRef> *const term = idIndex->binaryValue();
    optional<int> prio = outerInstance->outerInstance.priority[term];
    return !prio ? 0 : prio.value();
  } else {
    return 0;
  }
}

int ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::compareBottom(int doc) throw(
        IOException)
{
  return docVal(doc) - bottomVal;
}

void ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::copy(int slot,
                                                 int doc) 
{
  values[slot] = docVal(doc);
}

void ElevationComparatorSource::FieldComparatorAnonymousInnerClass::
    LeafFieldComparatorAnonymousInnerClass::setScorer(shared_ptr<Scorer> scorer)
{
}

int ElevationComparatorSource::FieldComparatorAnonymousInnerClass::compare(
    int slot1, int slot2)
{
  return values[slot2] - values[slot1]; // values will be small enough that
                                        // there is no overflow concern
}

void ElevationComparatorSource::FieldComparatorAnonymousInnerClass::setTopValue(
    optional<int> &value)
{
  throw make_shared<UnsupportedOperationException>();
}

optional<int>
ElevationComparatorSource::FieldComparatorAnonymousInnerClass::value(int slot)
{
  return static_cast<Integer>(values[slot]);
}
} // namespace org::apache::lucene::search