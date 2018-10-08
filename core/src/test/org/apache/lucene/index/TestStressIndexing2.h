#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::index
{
class IndexingThread;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class Fields;
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

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestStressIndexing2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestStressIndexing2)
public:
  static int maxFields;
  static int bigFieldSize;
  static bool sameFieldOrder;
  static int mergeFactor;
  static int maxBufferedDocs;
  static int seed;

  virtual void testRandomIWReader() ;

  virtual void testRandom() ;

  virtual void testMultiConfig() ;

  static std::shared_ptr<Term> idTerm;
  std::deque<std::shared_ptr<IndexingThread>> threads;
  static std::shared_ptr<Comparator<std::shared_ptr<IndexableField>>>
      fieldNameComparator;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<IndexableField>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<IndexableField> o1,
                std::shared_ptr<IndexableField> o2) override;
  };

  // This test avoids using any extra synchronization in the multiple
  // indexing threads to test that IndexWriter does correctly synchronize
  // everything.

public:
  class DocsAndWriter : public std::enable_shared_from_this<DocsAndWriter>
  {
    GET_CLASS_NAME(DocsAndWriter)
  public:
    std::unordered_map<std::wstring, std::shared_ptr<Document>> docs;
    std::shared_ptr<IndexWriter> writer;
  };

public:
  virtual std::shared_ptr<DocsAndWriter> indexRandomIWReader(
      int nThreads, int iterations, int range,
      std::shared_ptr<Directory> dir) ;

  virtual std::unordered_map<std::wstring, std::shared_ptr<Document>>
  indexRandom(int nThreads, int iterations, int range,
              std::shared_ptr<Directory> dir,
              bool doReaderPooling) ;

  static void
  indexSerial(std::shared_ptr<Random> random,
              std::unordered_map<std::wstring, std::shared_ptr<Document>> &docs,
              std::shared_ptr<Directory> dir) ;

  virtual void
  verifyEquals(std::shared_ptr<Random> r, std::shared_ptr<DirectoryReader> r1,
               std::shared_ptr<Directory> dir2,
               const std::wstring &idField) ;

  virtual void
  verifyEquals(std::shared_ptr<Directory> dir1, std::shared_ptr<Directory> dir2,
               const std::wstring &idField) ;

private:
  static void
  printDocs(std::shared_ptr<DirectoryReader> r) ;

  static int
  nextNonDeletedDoc(std::shared_ptr<PostingsEnum> it,
                    std::shared_ptr<Bits> liveDocs) ;

public:
  virtual void
  verifyEquals(std::shared_ptr<DirectoryReader> r1,
               std::shared_ptr<DirectoryReader> r2,
               const std::wstring &idField) ;

  static void verifyEquals(std::shared_ptr<Document> d1,
                           std::shared_ptr<Document> d2);

  static void verifyEquals(std::shared_ptr<Fields> d1,
                           std::shared_ptr<Fields> d2) ;

private:
  class IndexingThread : public Thread
  {
    GET_CLASS_NAME(IndexingThread)
  public:
    std::shared_ptr<IndexWriter> w;
    int base = 0;
    int range = 0;
    int iterations = 0;
    std::unordered_map<std::wstring, std::shared_ptr<Document>> docs =
        std::unordered_map<std::wstring, std::shared_ptr<Document>>();
    std::shared_ptr<Random> r;

    virtual int nextInt(int lim);

    // start is inclusive and end is exclusive
    virtual int nextInt(int start, int end);

    std::deque<wchar_t> buffer = std::deque<wchar_t>(100);

  private:
    int addUTF8Token(int start);

  public:
    virtual std::wstring getString(int nTokens);

    virtual std::wstring getUTF8String(int nTokens);

    virtual std::wstring getIdString();

    virtual void indexDoc() ;

    virtual void deleteDoc() ;

    virtual void deleteByQuery() ;

    void run() override;

  protected:
    std::shared_ptr<IndexingThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexingThread>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestStressIndexing2> shared_from_this()
  {
    return std::static_pointer_cast<TestStressIndexing2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
