#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class TestPoint;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::util
{
class InfoStream;
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
namespace org::apache::lucene::index
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Field = org::apache::lucene::document::Field;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using InfoStream = org::apache::lucene::util::InfoStream;

/** Silly class that randomizes the indexing experience.  EG
 *  it may swap in a different merge policy/scheduler; may
 *  commit periodically; may or may not forceMerge in the end,
 *  may flush by doc count instead of RAM, etc.
 */

class RandomIndexWriter : public std::enable_shared_from_this<RandomIndexWriter>
{
  GET_CLASS_NAME(RandomIndexWriter)

public:
  const std::shared_ptr<IndexWriter> w;

private:
  const std::shared_ptr<Random> r;

public:
  int docCount = 0;
  int flushAt = 0;

private:
  double flushAtFactor = 1.0;
  bool getReaderCalled = false;
  const std::shared_ptr<Analyzer>
      analyzer; // only if WE created it (then we close it)
  const double softDeletesRatio;

  /** Returns an indexwriter that randomly mixes up thread scheduling (by
   * yielding at test points) */
public:
  static std::shared_ptr<IndexWriter>
  mockIndexWriter(std::shared_ptr<Directory> dir,
                  std::shared_ptr<IndexWriterConfig> conf,
                  std::shared_ptr<Random> r) ;

private:
  class TestPointAnonymousInnerClass
      : public std::enable_shared_from_this<TestPointAnonymousInnerClass>,
        public TestPoint
  {
    GET_CLASS_NAME(TestPointAnonymousInnerClass)
  private:
    std::shared_ptr<Random> random;

  public:
    TestPointAnonymousInnerClass(std::shared_ptr<Random> random);

    void apply(const std::wstring &message) override;
  };

  /** Returns an indexwriter that enables the specified test point */
public:
  static std::shared_ptr<IndexWriter>
  mockIndexWriter(std::shared_ptr<Random> r, std::shared_ptr<Directory> dir,
                  std::shared_ptr<IndexWriterConfig> conf,
                  std::shared_ptr<TestPoint> testPoint) ;

private:
  class IndexWriterAnonymousInnerClass : public IndexWriter
  {
    GET_CLASS_NAME(IndexWriterAnonymousInnerClass)
  public:
    IndexWriterAnonymousInnerClass(
        std::shared_ptr<Directory> dir,
        std::shared_ptr<org::apache::lucene::index::IndexWriterConfig> conf);

  protected:
    bool isEnableTestPoints() override;

  protected:
    std::shared_ptr<IndexWriterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexWriterAnonymousInnerClass>(
          IndexWriter::shared_from_this());
    }
  };

  /** create a RandomIndexWriter with a random config: Uses MockAnalyzer */
public:
  RandomIndexWriter(std::shared_ptr<Random> r,
                    std::shared_ptr<Directory> dir) ;

  /** create a RandomIndexWriter with a random config */
  RandomIndexWriter(std::shared_ptr<Random> r, std::shared_ptr<Directory> dir,
                    std::shared_ptr<Analyzer> a) ;

  /** create a RandomIndexWriter with the provided config */
  RandomIndexWriter(std::shared_ptr<Random> r, std::shared_ptr<Directory> dir,
                    std::shared_ptr<IndexWriterConfig> c) ;

  /** create a RandomIndexWriter with the provided config */
  RandomIndexWriter(std::shared_ptr<Random> r, std::shared_ptr<Directory> dir,
                    std::shared_ptr<IndexWriterConfig> c,
                    bool useSoftDeletes) ;

private:
  RandomIndexWriter(std::shared_ptr<Random> r, std::shared_ptr<Directory> dir,
                    std::shared_ptr<IndexWriterConfig> c, bool closeAnalyzer,
                    bool useSoftDeletes) ;

  /**
   * Adds a Document.
   * @see IndexWriter#addDocument(Iterable)
   */
public:
  template <typename T>
  int64_t addDocument(std::deque<T> &doc) ;

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::deque<std::shared_ptr<T>>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::shared_ptr<RandomIndexWriter> outerInstance;

    std::deque<std::shared_ptr<T>> doc;

  public:
    IterableAnonymousInnerClass(
        std::shared_ptr<RandomIndexWriter> outerInstance,
        std::deque<std::shared_ptr<T>> &doc);

    std::shared_ptr<Iterator<std::deque<std::shared_ptr<T>>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::deque<std::shared_ptr<T>>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass> outerInstance;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass> outerInstance);

      bool done = false;

      bool hasNext();

      void remove();

      std::deque<std::shared_ptr<T>> next();
    };
  };

private:
  void maybeFlushOrCommit() ;

public:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long addDocuments(Iterable<? extends
  // Iterable<? extends IndexableField>> docs) throws java.io.IOException
  int64_t addDocuments(std::deque<T1> docs) ;

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long updateDocuments(Term delTerm,
  // Iterable<? extends Iterable<? extends IndexableField>> docs) throws
  // java.io.IOException
  int64_t updateDocuments(std::shared_ptr<Term> delTerm,
                            std::deque<T1> docs) ;

private:
  bool useSoftDeletes();

  /**
   * Updates a document.
   * @see IndexWriter#updateDocument(Term, Iterable)
   */
public:
  template <typename T>
  int64_t updateDocument(std::shared_ptr<Term> t,
                           std::deque<T> &doc) ;

  virtual int64_t addIndexes(std::deque<Directory> &dirs) ;

  virtual int64_t
  addIndexes(std::deque<CodecReader> &readers) ;

  virtual int64_t
  updateNumericDocValue(std::shared_ptr<Term> term, const std::wstring &field,
                        std::optional<int64_t> &value) ;

  virtual int64_t
  updateBinaryDocValue(std::shared_ptr<Term> term, const std::wstring &field,
                       std::shared_ptr<BytesRef> value) ;

  virtual int64_t
  updateDocValues(std::shared_ptr<Term> term,
                  std::deque<Field> &updates) ;

  virtual int64_t
  deleteDocuments(std::shared_ptr<Term> term) ;

  virtual int64_t
  deleteDocuments(std::shared_ptr<Query> q) ;

  virtual int64_t commit() ;

  virtual int numDocs();

  virtual int maxDoc();

  virtual int64_t deleteAll() ;

  virtual std::shared_ptr<DirectoryReader> getReader() ;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool doRandomForceMerge_ = true;
  bool doRandomForceMergeAssert = true;

public:
  virtual void forceMergeDeletes(bool doWait) ;

  virtual void forceMergeDeletes() ;

  virtual void setDoRandomForceMerge(bool v);

  virtual void setDoRandomForceMergeAssert(bool v);

private:
  void doRandomForceMerge() ;

public:
  virtual std::shared_ptr<DirectoryReader>
  getReader(bool applyDeletions, bool writeAllDeletes) ;

  /**
   * Close this writer.
   * @see IndexWriter#close()
   */
  virtual ~RandomIndexWriter();

  /**
   * Forces a forceMerge.
   * <p>
   * NOTE: this should be avoided in tests unless absolutely necessary,
   * as it will result in less test coverage.
   * @see IndexWriter#forceMerge(int)
   */
  virtual void forceMerge(int maxSegmentCount) ;

public:
  class TestPointInfoStream final : public InfoStream
  {
    GET_CLASS_NAME(TestPointInfoStream)
  private:
    const std::shared_ptr<InfoStream> delegate_;
    const std::shared_ptr<TestPoint> testPoint;

  public:
    TestPointInfoStream(std::shared_ptr<InfoStream> delegate_,
                        std::shared_ptr<TestPoint> testPoint);

    virtual ~TestPointInfoStream();

    void message(const std::wstring &component,
                 const std::wstring &message) override;

    bool isEnabled(const std::wstring &component) override;

  protected:
    std::shared_ptr<TestPointInfoStream> shared_from_this()
    {
      return std::static_pointer_cast<TestPointInfoStream>(
          org.apache.lucene.util.InfoStream::shared_from_this());
    }
  };

  /** Writes all in-memory segments to the {@link Directory}. */
public:
  void flush() ;

  /**
   * Simple interface that is executed for each <tt>TP</tt> {@link InfoStream}
   * component message. See also {@link
   * RandomIndexWriter#mockIndexWriter(Random, Directory, IndexWriterConfig,
   * TestPoint)}
   */
public:
  class TestPoint
  {
    GET_CLASS_NAME(TestPoint)
  public:
    virtual void apply(const std::wstring &message) = 0;
  };
};

} // namespace org::apache::lucene::index
