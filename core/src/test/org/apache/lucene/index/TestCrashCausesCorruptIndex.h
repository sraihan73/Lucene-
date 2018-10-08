#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestCrashCausesCorruptIndex : public LuceneTestCase
{
  GET_CLASS_NAME(TestCrashCausesCorruptIndex)

public:
  std::shared_ptr<Path> path;

  /**
   * LUCENE-3627: This test fails.
   */
  virtual void testCrashCorruptsIndexing() ;

  /**
   * index 1 document and commit.
   * prepare for crashing.
   * index 1 more document, and upon commit, creation of segments_2 will crash.
   */
private:
  void indexAndCrashOnCreateOutputSegments2() ;

  /**
   * Attempts to index another 1 document.
   */
  void indexAfterRestart() ;

  /**
   * Run an example search.
   */
  void searchForFleas(int const expectedTotalHits) ;

  static const std::wstring TEXT_FIELD;

  /**
   * Gets a document with content "my dog has fleas".
   */
  std::shared_ptr<Document> getDocument();

  /**
   * The marker RuntimeException that we use in lieu of an
   * actual machine crash.
   */
private:
  class CrashingException : public std::runtime_error
  {
    GET_CLASS_NAME(CrashingException)
  public:
    CrashingException(const std::wstring &msg);

  protected:
    std::shared_ptr<CrashingException> shared_from_this()
    {
      return std::static_pointer_cast<CrashingException>(
          RuntimeException::shared_from_this());
    }
  };

  /**
   * This test class provides direct access to "simulating" a crash right after
   * realDirectory.createOutput(..) has been called on a certain specified name.
   */
private:
  class CrashAfterCreateOutput : public FilterDirectory
  {
    GET_CLASS_NAME(CrashAfterCreateOutput)

  private:
    std::wstring crashAfterCreateOutput;

  public:
    CrashAfterCreateOutput(std::shared_ptr<Directory> realDirectory) throw(
        IOException);

    virtual void setCrashAfterCreateOutput(const std::wstring &name);

    std::shared_ptr<IndexOutput>
    createOutput(const std::wstring &name,
                 std::shared_ptr<IOContext> cxt)  override;

  protected:
    std::shared_ptr<CrashAfterCreateOutput> shared_from_this()
    {
      return std::static_pointer_cast<CrashAfterCreateOutput>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestCrashCausesCorruptIndex> shared_from_this()
  {
    return std::static_pointer_cast<TestCrashCausesCorruptIndex>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
