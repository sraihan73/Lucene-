using namespace std;

#include "ReadTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Collector.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopFieldCollector.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopScoreDocCollector.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Bits.h"
#include "../PerfRunData.h"
#include "../feeds/QueryMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Collector = org::apache::lucene::search::Collector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using TopScoreDocCollector = org::apache::lucene::search::TopScoreDocCollector;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;

ReadTask::ReadTask(shared_ptr<PerfRunData> runData) : PerfTask(runData)
{
  if (withSearch()) {
    queryMaker = getQueryMaker();
  } else {
    queryMaker.reset();
  }
}

int ReadTask::doLogic() 
{
  int res = 0;

  // open reader or use existing one
  shared_ptr<IndexSearcher> searcher =
      getRunData()->getIndexSearcher(); // (will incRef the reader)

  shared_ptr<IndexReader> reader;

  constexpr bool closeSearcher;
  if (searcher == nullptr) {
    // open our own reader
    shared_ptr<Directory> dir = getRunData()->getDirectory();
    reader = DirectoryReader::open(dir);
    searcher = make_shared<IndexSearcher>(reader);
    closeSearcher = true;
  } else {
    // use existing one; this passes +1 ref to us
    reader = searcher->getIndexReader();
    closeSearcher = false;
  }

  // optionally warm and add num docs traversed to count
  if (withWarm()) {
    shared_ptr<Document> doc;
    shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(reader);
    for (int m = 0; m < reader->maxDoc(); m++) {
      if (nullptr == liveDocs || liveDocs->get(m)) {
        doc = reader->document(m);
        res += (doc->empty() ? 0 : 1);
      }
    }
  }

  if (withSearch()) {
    res++;
    shared_ptr<Query> q = queryMaker->makeQuery();
    shared_ptr<Sort> sort = getSort();
    shared_ptr<TopDocs> hits = nullptr;
    constexpr int numHits = this->numHits();
    if (numHits > 0) {
      if (withCollector() == false) {
        if (sort != nullptr) {
          // TODO: instead of always passing false we
          // should detect based on the query; if we make
          // the IndexSearcher search methods that take
          // Weight public again, we can go back to
          // pulling the Weight ourselves:
          shared_ptr<TopFieldCollector> collector =
              TopFieldCollector::create(sort, numHits, true, withScore(),
                                        withMaxScore(), withTotalHits());
          searcher->search(q, collector);
          hits = collector->topDocs();
        } else {
          hits = searcher->search(q, numHits);
        }
      } else {
        shared_ptr<Collector> collector = createCollector();
        searcher->search(q, collector);
        // hits = collector.topDocs();
      }

      if (hits != nullptr) {
        const wstring printHitsField =
            getRunData()->getConfig()->get(L"print.hits.field", L"");
        if (printHitsField != L"" && printHitsField.length() > 0) {
          wcout << L"totalHits = " << hits->totalHits << endl;
          wcout << L"maxDoc()  = " << reader->maxDoc() << endl;
          wcout << L"numDocs() = " << reader->numDocs() << endl;
          for (int i = 0; i < hits->scoreDocs.size(); i++) {
            constexpr int docID = hits->scoreDocs[i]->doc;
            shared_ptr<Document> *const doc = reader->document(docID);
            wcout << L"  " << i << L": doc=" << docID << L" score="
                  << hits->scoreDocs[i]->score << L" " << printHitsField
                  << L" =" << doc[printHitsField] << endl;
          }
        }

        res += withTopDocs(searcher, q, hits);
      }
    }
  }

  if (closeSearcher) {
    delete reader;
  } else {
    // Release our +1 ref from above
    reader->decRef();
  }
  return res;
}

int ReadTask::withTopDocs(shared_ptr<IndexSearcher> searcher,
                          shared_ptr<Query> q,
                          shared_ptr<TopDocs> hits) 
{
  shared_ptr<IndexReader> reader = searcher->getIndexReader();
  int res = 0;
  if (withTraverse()) {
    std::deque<std::shared_ptr<ScoreDoc>> scoreDocs = hits->scoreDocs;
    int traversalSize = min(scoreDocs.size(), this->traversalSize());

    if (traversalSize > 0) {
      bool retrieve = withRetrieve();
      for (int m = 0; m < traversalSize; m++) {
        int id = scoreDocs[m]->doc;
        res++;
        if (retrieve) {
          shared_ptr<Document> document = retrieveDoc(reader, id);
          res += document->size() > 0 ? 1 : 0;
        }
      }
    }
  }
  return res;
}

shared_ptr<Collector> ReadTask::createCollector() 
{
  return TopScoreDocCollector::create(numHits());
}

shared_ptr<Document> ReadTask::retrieveDoc(shared_ptr<IndexReader> ir,
                                           int id) 
{
  return ir->document(id);
}

bool ReadTask::withCollector() { return false; }

bool ReadTask::withScore() { return true; }

bool ReadTask::withMaxScore() { return true; }

bool ReadTask::withTotalHits() { return false; }

int ReadTask::traversalSize() { return numeric_limits<int>::max(); }

void ReadTask::setup() 
{
  PerfTask::setup();
  numHits_ = getRunData()->getConfig()->get(L"search.num.hits",
                                            DEFAULT_SEARCH_NUM_HITS);
}

int ReadTask::numHits() { return numHits_; }

shared_ptr<Sort> ReadTask::getSort() { return nullptr; }
} // namespace org::apache::lucene::benchmark::byTask::tasks