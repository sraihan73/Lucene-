using namespace std;

#include "PerfRunData.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include "../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/TaxonomyWriter.h"
#include "feeds/ContentSource.h"
#include "feeds/DocMaker.h"
#include "feeds/FacetSource.h"
#include "feeds/QueryMaker.h"
#include "stats/Points.h"
#include "tasks/NewAnalyzerTask.h"
#include "tasks/ReadTask.h"
#include "tasks/SearchTask.h"
#include "utils/AnalyzerFactory.h"
#include "utils/Config.h"

namespace org::apache::lucene::benchmark::byTask
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using ContentSource =
    org::apache::lucene::benchmark::byTask::feeds::ContentSource;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using FacetSource = org::apache::lucene::benchmark::byTask::feeds::FacetSource;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;
using Points = org::apache::lucene::benchmark::byTask::stats::Points;
using NewAnalyzerTask =
    org::apache::lucene::benchmark::byTask::tasks::NewAnalyzerTask;
using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;
using ReadTask = org::apache::lucene::benchmark::byTask::tasks::ReadTask;
using SearchTask = org::apache::lucene::benchmark::byTask::tasks::SearchTask;
using AnalyzerFactory =
    org::apache::lucene::benchmark::byTask::utils::AnalyzerFactory;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;

PerfRunData::PerfRunData(shared_ptr<Config> config) 
{
  this->config = config;
  // analyzer (default is standard analyzer)
  analyzer = NewAnalyzerTask::createAnalyzer(config->get(
      L"analyzer", L"org.apache.lucene.analysis.standard.StandardAnalyzer"));

  // content source
  wstring sourceClass =
      config->get(L"content.source",
                  L"org.apache.lucene.benchmark.byTask.feeds.SingleDocSource");
  contentSource = type_info::forName(sourceClass)
                      .asSubclass(ContentSource::typeid)
                      .newInstance();
  contentSource->setConfig(config);

  // doc maker
  docMaker =
      type_info::forName(
          config->get(L"doc.maker",
                      L"org.apache.lucene.benchmark.byTask.feeds.DocMaker"))
          .asSubclass(DocMaker::typeid)
          .newInstance();
  docMaker->setConfig(config, contentSource);
  // facet source
  facetSource =
      type_info::forName(
          config->get(
              L"facet.source",
              L"org.apache.lucene.benchmark.byTask.feeds.RandomFacetSource"))
          .asSubclass(FacetSource::typeid)
          .newInstance();
  facetSource->setConfig(config);
  // query makers
  readTaskQueryMaker = unordered_map<type_info, std::shared_ptr<QueryMaker>>();
  qmkrClass =
      type_info::forName(
          config->get(
              L"query.maker",
              L"org.apache.lucene.benchmark.byTask.feeds.SimpleQueryMaker"))
          .asSubclass(QueryMaker::typeid);

  // index stuff
  reinit(false);

  // statistic points
  points = make_shared<Points>(config);

  if (static_cast<Boolean>(config->get(L"log.queries", L"false"))
          .booleanValue()) {
    wcout << L"------------> queries:" << endl;
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    wcout << getQueryMaker(make_shared<SearchTask>(shared_from_this()))
                 ->printQueries()
          << endl;
  }
}

PerfRunData::~PerfRunData()
{
  if (indexWriter != nullptr) {
    delete indexWriter;
  }
  IOUtils::close({indexReader, directory, taxonomyWriter, taxonomyReader,
                  taxonomyDir, docMaker, facetSource, contentSource});

  // close all perf objects that are closeable.
  deque<std::shared_ptr<Closeable>> perfObjectsToClose =
      deque<std::shared_ptr<Closeable>>();
  for (auto obj : perfObjects) {
    if (std::dynamic_pointer_cast<Closeable>(obj.second) != nullptr) {
      perfObjectsToClose.push_back(
          std::static_pointer_cast<Closeable>(obj.second));
    }
  }
  IOUtils::close(perfObjectsToClose);
}

void PerfRunData::reinit(bool eraseIndex) 
{

  // cleanup index
  if (indexWriter != nullptr) {
    delete indexWriter;
  }
  IOUtils::close({indexReader, directory});
  indexWriter.reset();
  indexReader.reset();

  IOUtils::close({taxonomyWriter, taxonomyReader, taxonomyDir});
  taxonomyWriter.reset();
  taxonomyReader.reset();

  // directory (default is ram-dir).
  directory = createDirectory(eraseIndex, L"index", L"directory");
  taxonomyDir = createDirectory(eraseIndex, L"taxo", L"taxonomy.directory");

  // inputs
  resetInputs();

  // release unused stuff
  System::runFinalization();
  System::gc();

  // Re-init clock
  setStartTimeMillis();
}

shared_ptr<Directory>
PerfRunData::createDirectory(bool eraseIndex, const wstring &dirName,
                             const wstring &dirParam) 
{
  if (L"FSDirectory" == config->get(dirParam, L"RAMDirectory")) {
    shared_ptr<Path> workDir = Paths->get(config->get(L"work.dir", L"work"));
    shared_ptr<Path> indexDir = workDir->resolve(dirName);
    if (eraseIndex && Files::exists(indexDir)) {
      IOUtils::rm({indexDir});
    }
    Files::createDirectories(indexDir);
    return FSDirectory::open(indexDir);
  }

  return make_shared<RAMDirectory>();
}

// C++ WARNING: The following method was originally marked 'synchronized':
any PerfRunData::getPerfObject(const wstring &key) { return perfObjects[key]; }

// C++ WARNING: The following method was originally marked 'synchronized':
void PerfRunData::setPerfObject(const wstring &key, any obj)
{
  perfObjects.emplace(key, obj);
}

int64_t PerfRunData::setStartTimeMillis()
{
  startTimeMillis = System::currentTimeMillis();
  return startTimeMillis;
}

int64_t PerfRunData::getStartTimeMillis() { return startTimeMillis; }

shared_ptr<Points> PerfRunData::getPoints() { return points; }

shared_ptr<Directory> PerfRunData::getDirectory() { return directory; }

void PerfRunData::setDirectory(shared_ptr<Directory> directory)
{
  this->directory = directory;
}

shared_ptr<Directory> PerfRunData::getTaxonomyDir() { return taxonomyDir; }

// C++ WARNING: The following method was originally marked 'synchronized':
void PerfRunData::setTaxonomyReader(
    shared_ptr<TaxonomyReader> taxoReader) 
{
  if (taxoReader == this->taxonomyReader) {
    return;
  }
  if (taxonomyReader != nullptr) {
    taxonomyReader->decRef();
  }

  if (taxoReader != nullptr) {
    taxoReader->incRef();
  }
  this->taxonomyReader = taxoReader;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<TaxonomyReader> PerfRunData::getTaxonomyReader()
{
  if (taxonomyReader != nullptr) {
    taxonomyReader->incRef();
  }
  return taxonomyReader;
}

void PerfRunData::setTaxonomyWriter(shared_ptr<TaxonomyWriter> taxoWriter)
{
  this->taxonomyWriter = taxoWriter;
}

shared_ptr<TaxonomyWriter> PerfRunData::getTaxonomyWriter()
{
  return taxonomyWriter;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DirectoryReader> PerfRunData::getIndexReader()
{
  if (indexReader != nullptr) {
    indexReader->incRef();
  }
  return indexReader;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexSearcher> PerfRunData::getIndexSearcher()
{
  if (indexReader != nullptr) {
    indexReader->incRef();
  }
  return indexSearcher;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PerfRunData::setIndexReader(shared_ptr<DirectoryReader> indexReader) throw(
    IOException)
{
  if (indexReader == this->indexReader) {
    return;
  }

  if (this->indexReader != nullptr) {
    // Release current IR
    this->indexReader->decRef();
  }

  this->indexReader = indexReader;
  if (indexReader != nullptr) {
    // Hold reference to new IR
    indexReader->incRef();
    indexSearcher = make_shared<IndexSearcher>(indexReader);
    // TODO Some day we should make the query cache in this module configurable
    // and control clearing the cache
    indexSearcher->setQueryCache(nullptr);
  } else {
    indexSearcher.reset();
  }
}

shared_ptr<IndexWriter> PerfRunData::getIndexWriter() { return indexWriter; }

void PerfRunData::setIndexWriter(shared_ptr<IndexWriter> indexWriter)
{
  this->indexWriter = indexWriter;
}

shared_ptr<Analyzer> PerfRunData::getAnalyzer() { return analyzer; }

void PerfRunData::setAnalyzer(shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
}

shared_ptr<ContentSource> PerfRunData::getContentSource()
{
  return contentSource;
}

shared_ptr<DocMaker> PerfRunData::getDocMaker() { return docMaker; }

shared_ptr<FacetSource> PerfRunData::getFacetSource() { return facetSource; }

shared_ptr<Locale> PerfRunData::getLocale() { return locale; }

void PerfRunData::setLocale(shared_ptr<Locale> locale)
{
  this->locale = locale;
}

shared_ptr<Config> PerfRunData::getConfig() { return config; }

void PerfRunData::resetInputs() 
{
  contentSource->resetInputs();
  docMaker->resetInputs();
  facetSource->resetInputs();
  for (auto queryMaker : readTaskQueryMaker) {
    try {
      queryMaker->second.resetInputs();
    } catch (const IOException &e) {
      throw e;
    } catch (const runtime_error &e) {
      throw runtime_error(e);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<QueryMaker> PerfRunData::getQueryMaker(shared_ptr<ReadTask> readTask)
{
  // mapping the query maker by task class allows extending/adding new
  // search/read tasks without needing to modify this class.
  type_info readTaskClass = readTask->getClass();
  shared_ptr<QueryMaker> qm = readTaskQueryMaker[readTaskClass];
  if (qm == nullptr) {
    try {
      qm = qmkrClass.newInstance();
      qm->setConfig(config);
    } catch (const runtime_error &e) {
      throw runtime_error(e);
    }
    readTaskQueryMaker.emplace(readTaskClass, qm);
  }
  return qm;
}

unordered_map<wstring, std::shared_ptr<AnalyzerFactory>>
PerfRunData::getAnalyzerFactories()
{
  return analyzerFactories;
}
} // namespace org::apache::lucene::benchmark::byTask