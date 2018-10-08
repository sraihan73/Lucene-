#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::stats
{
class Points;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::benchmark::byTask::utils
{
class AnalyzerFactory;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocMaker;
}
namespace org::apache::lucene::benchmark::byTask::feeds
{
class ContentSource;
}
namespace org::apache::lucene::benchmark::byTask::feeds
{
class FacetSource;
}
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyWriter;
}
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyReader;
}
namespace org::apache::lucene::benchmark::byTask::feeds
{
class QueryMaker;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
}
namespace org::apache::lucene::benchmark::byTask::tasks
{
class ReadTask;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::benchmark::byTask
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using ContentSource =
    org::apache::lucene::benchmark::byTask::feeds::ContentSource;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using FacetSource = org::apache::lucene::benchmark::byTask::feeds::FacetSource;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;
using Points = org::apache::lucene::benchmark::byTask::stats::Points;
using ReadTask = org::apache::lucene::benchmark::byTask::tasks::ReadTask;
using AnalyzerFactory =
    org::apache::lucene::benchmark::byTask::utils::AnalyzerFactory;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;

/**
 * Data maintained by a performance test run.
 * <p>
 * Data includes:
 * <ul>
 *  <li>Configuration.
 *  <li>Directory, Writer, Reader.
 *  <li>Taxonomy Directory, Writer, Reader.
 *  <li>DocMaker, FacetSource and a few instances of QueryMaker.
 *  <li>Named AnalysisFactories.
 *  <li>Analyzer.
 *  <li>Statistics data which updated during the run.
 * </ul>
 * Config properties:
 * <ul>
 *  <li><b>work.dir</b>=&lt;path to root of docs and index dirs| Default:
 * work&gt; <li><b>analyzer</b>=&lt;class name for analyzer| Default:
 * StandardAnalyzer&gt; <li><b>doc.maker</b>=&lt;class name for doc-maker|
 * Default: DocMaker&gt; <li><b>facet.source</b>=&lt;class name for
 * facet-source| Default: RandomFacetSource&gt; <li><b>query.maker</b>=&lt;class
 * name for query-maker| Default: SimpleQueryMaker&gt;
 *  <li><b>log.queries</b>=&lt;whether queries should be printed| Default:
 * false&gt; <li><b>directory</b>=&lt;type of directory to use for the index|
 * Default: RAMDirectory&gt; <li><b>taxonomy.directory</b>=&lt;type of directory
 * for taxonomy index| Default: RAMDirectory&gt;
 * </ul>
 */
class PerfRunData : public std::enable_shared_from_this<PerfRunData>
{
  GET_CLASS_NAME(PerfRunData)

private:
  std::shared_ptr<Points> points;

  // objects used during performance test run
  // directory, analyzer, docMaker - created at startup.
  // reader, writer, searcher - maintained by basic tasks.
  std::shared_ptr<Directory> directory;
  std::unordered_map<std::wstring, std::shared_ptr<AnalyzerFactory>>
      analyzerFactories =
          std::unordered_map<std::wstring, std::shared_ptr<AnalyzerFactory>>();
  std::shared_ptr<Analyzer> analyzer;
  std::shared_ptr<DocMaker> docMaker;
  std::shared_ptr<ContentSource> contentSource;
  std::shared_ptr<FacetSource> facetSource;
  std::shared_ptr<Locale> locale;

  std::shared_ptr<Directory> taxonomyDir;
  std::shared_ptr<TaxonomyWriter> taxonomyWriter;
  std::shared_ptr<TaxonomyReader> taxonomyReader;

  // we use separate (identical) instances for each "read" task type, so each
  // can iterate the quries separately.
  std::unordered_map<std::type_info, std::shared_ptr<QueryMaker>>
      readTaskQueryMaker;
  std::type_info qmkrClass;

  std::shared_ptr<DirectoryReader> indexReader;
  std::shared_ptr<IndexSearcher> indexSearcher;
  std::shared_ptr<IndexWriter> indexWriter;
  std::shared_ptr<Config> config;
  int64_t startTimeMillis = 0;

  const std::unordered_map<std::wstring, std::any> perfObjects =
      std::unordered_map<std::wstring, std::any>();

  // constructor
public:
  PerfRunData(std::shared_ptr<Config> config) ;

  virtual ~PerfRunData();

  // clean old stuff, reopen
  virtual void reinit(bool eraseIndex) ;

private:
  std::shared_ptr<Directory>
  createDirectory(bool eraseIndex, const std::wstring &dirName,
                  const std::wstring &dirParam) ;

  /** Returns an object that was previously set by {@link #setPerfObject(std::wstring,
   * Object)}. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::any getPerfObject(const std::wstring &key);

  /**
   * Sets an object that is required by {@link PerfTask}s, keyed by the given
   * {@code key}. If the object implements {@link Closeable}, it will be closed
   * by {@link #close()}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setPerfObject(const std::wstring &key, std::any obj);

  virtual int64_t setStartTimeMillis();

  /**
   * @return Start time in milliseconds
   */
  virtual int64_t getStartTimeMillis();

  /**
   * @return Returns the points.
   */
  virtual std::shared_ptr<Points> getPoints();

  /**
   * @return Returns the directory.
   */
  virtual std::shared_ptr<Directory> getDirectory();

  /**
   * @param directory The directory to set.
   */
  virtual void setDirectory(std::shared_ptr<Directory> directory);

  /**
   * @return Returns the taxonomy directory
   */
  virtual std::shared_ptr<Directory> getTaxonomyDir();

  /**
   * Set the taxonomy reader. Takes ownership of that taxonomy reader, that is,
   * internally performs taxoReader.incRef() (If caller no longer needs that
   * reader it should decRef()/close() it after calling this method, otherwise,
   * the reader will remain open).
   * @param taxoReader The taxonomy reader to set.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setTaxonomyReader(
      std::shared_ptr<TaxonomyReader> taxoReader) ;

  /**
   * @return Returns the taxonomyReader.  NOTE: this returns a
   * reference.  You must call TaxonomyReader.decRef() when
   * you're done.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<TaxonomyReader> getTaxonomyReader();

  /**
   * @param taxoWriter The taxonomy writer to set.
   */
  virtual void setTaxonomyWriter(std::shared_ptr<TaxonomyWriter> taxoWriter);

  virtual std::shared_ptr<TaxonomyWriter> getTaxonomyWriter();

  /**
   * @return Returns the indexReader.  NOTE: this returns a
   * reference.  You must call IndexReader.decRef() when
   * you're done.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<DirectoryReader> getIndexReader();

  /**
   * @return Returns the indexSearcher.  NOTE: this returns
   * a reference to the underlying IndexReader.  You must
   * call IndexReader.decRef() when you're done.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<IndexSearcher> getIndexSearcher();

  /**
   * Set the index reader. Takes ownership of that index reader, that is,
   * internally performs indexReader.incRef() (If caller no longer needs that
   * reader it should decRef()/close() it after calling this method, otherwise,
   * the reader will remain open).
   * @param indexReader The indexReader to set.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setIndexReader(
      std::shared_ptr<DirectoryReader> indexReader) ;

  /**
   * @return Returns the indexWriter.
   */
  virtual std::shared_ptr<IndexWriter> getIndexWriter();

  /**
   * @param indexWriter The indexWriter to set.
   */
  virtual void setIndexWriter(std::shared_ptr<IndexWriter> indexWriter);

  /**
   * @return Returns the analyzer.
   */
  virtual std::shared_ptr<Analyzer> getAnalyzer();

  virtual void setAnalyzer(std::shared_ptr<Analyzer> analyzer);

  /** Returns the ContentSource. */
  virtual std::shared_ptr<ContentSource> getContentSource();

  /** Returns the DocMaker. */
  virtual std::shared_ptr<DocMaker> getDocMaker();

  /** Returns the facet source. */
  virtual std::shared_ptr<FacetSource> getFacetSource();

  /**
   * @return the locale
   */
  virtual std::shared_ptr<Locale> getLocale();

  /**
   * @param locale the locale to set
   */
  virtual void setLocale(std::shared_ptr<Locale> locale);

  /**
   * @return Returns the config.
   */
  virtual std::shared_ptr<Config> getConfig();

  virtual void resetInputs() ;

  /**
   * @return Returns the queryMaker by read task type (class)
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<QueryMaker>
  getQueryMaker(std::shared_ptr<ReadTask> readTask);

  virtual std::unordered_map<std::wstring, std::shared_ptr<AnalyzerFactory>>
  getAnalyzerFactories();
};

} // namespace org::apache::lucene::benchmark::byTask
