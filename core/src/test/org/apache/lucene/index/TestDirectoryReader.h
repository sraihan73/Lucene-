#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

class TestDirectoryReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestDirectoryReader)

public:
  virtual void testDocument() ;

  virtual void testMultiTermDocs() ;

private:
  void addDoc(std::shared_ptr<Random> random,
              std::shared_ptr<Directory> ramDir1, const std::wstring &s,
              bool create) ;

public:
  virtual void testIsCurrent() ;

  /**
   * Tests the IndexReader.getFieldNames implementation
   * @throws Exception on error
   */
  virtual void testGetFieldNames() ;

  virtual void testTermVectors() ;

  virtual void assertTermDocsCount(const std::wstring &msg,
                                   std::shared_ptr<IndexReader> reader,
                                   std::shared_ptr<Term> term,
                                   int expected) ;

  virtual void testBinaryFields() ;

  /* ??? public void testOpenEmptyDirectory() throws IOException{
    std::wstring dirName = "test.empty";
    File fileDirName = new File(dirName);
    if (!fileDirName.exists()) {
      fileDirName.mkdir();
    }
    try {
      DirectoryReader.open(fileDirName);
      fail("opening DirectoryReader on empty directory failed to produce
  FileNotFoundException/NoSuchFileException"); } catch (FileNotFoundException |
  NoSuchFileException e) {
      // GOOD
    }
    rmDir(fileDirName);
  }*/

  virtual void testFilesOpenClose() ;

  virtual void testOpenReaderAfterDelete() ;

  static void
  addDocumentWithFields(std::shared_ptr<IndexWriter> writer) ;

  static void addDocumentWithDifferentFields(
      std::shared_ptr<IndexWriter> writer) ;

  static void addDocumentWithTermVectorFields(
      std::shared_ptr<IndexWriter> writer) ;

  static void addDoc(std::shared_ptr<IndexWriter> writer,
                     const std::wstring &value) ;

  // TODO: maybe this can reuse the logic of test dueling codecs?
  static void
  assertIndexEquals(std::shared_ptr<DirectoryReader> index1,
                    std::shared_ptr<DirectoryReader> index2) ;

  virtual void testGetIndexCommit() ;

  static std::shared_ptr<Document> createDocument(const std::wstring &id);

  // LUCENE-1468 -- make sure on attempting to open an
  // DirectoryReader on a non-existent directory, you get a
  // good exception
  virtual void testNoDir() ;

  // LUCENE-1509
  virtual void testNoDupCommitFileNames() ;

  // LUCENE-1586: getUniqueTermCount
  virtual void testUniqueTermCount() ;

  // LUCENE-2046
  virtual void testPrepareCommitIsCurrent() ;

  // LUCENE-2753
  virtual void testListCommits() ;

  // Make sure totalTermFreq works correctly in the terms
  // dict cache
  virtual void testTotalTermFreqCached() ;

  virtual void testGetSumDocFreq() ;

  virtual void testGetDocCount() ;

  virtual void testGetSumTotalTermFreq() ;

  // LUCENE-2474
  virtual void testReaderFinishedListener() ;

  virtual void testOOBDocID() ;

  virtual void testTryIncRef() ;

  virtual void testStressTryIncRef() ;

public:
  class IncThread : public Thread
  {
    GET_CLASS_NAME(IncThread)
  public:
    const std::shared_ptr<IndexReader> toInc;
    const std::shared_ptr<Random> random;
    std::runtime_error failed;

    IncThread(std::shared_ptr<IndexReader> toInc,
              std::shared_ptr<Random> random);

    void run() override;

  protected:
    std::shared_ptr<IncThread> shared_from_this()
    {
      return std::static_pointer_cast<IncThread>(Thread::shared_from_this());
    }
  };

public:
  virtual void testLoadCertainFields() ;

  virtual void
  testIndexExistsOnNonExistentDirectory() ;

protected:
  std::shared_ptr<TestDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<TestDirectoryReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
