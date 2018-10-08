#pragma once
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/StreamUtils.h"
#include "../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/StreamUtils.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/WriteLineDocTask.h"

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
namespace org::apache::lucene::benchmark::byTask::tasks
{

using org::apache::commons::compress::compressors::CompressorStreamFactory;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Type = org::apache::lucene::benchmark::byTask::utils::StreamUtils::Type;
using Document = org::apache::lucene::document::Document;

/** Tests the functionality of {@link WriteLineDocTask}. */
class WriteLineDocTaskTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(WriteLineDocTaskTest)

  // class has to be public so that Class.forName.newInstance() will work
public:
  class WriteLineDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(WriteLineDocMaker)

  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<WriteLineDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<WriteLineDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
public:
  class NewLinesDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(NewLinesDocMaker)

  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<NewLinesDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<NewLinesDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
public:
  class NoBodyDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(NoBodyDocMaker)
  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<NoBodyDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<NoBodyDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
public:
  class NoTitleDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(NoTitleDocMaker)
  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<NoTitleDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<NoTitleDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
public:
  class JustDateDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(JustDateDocMaker)
  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<JustDateDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<JustDateDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
  // same as JustDate just that this one is treated as legal
public:
  class LegalJustDateDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(LegalJustDateDocMaker)
  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<LegalJustDateDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<LegalJustDateDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
public:
  class EmptyDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(EmptyDocMaker)
  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<EmptyDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<EmptyDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

  // class has to be public so that Class.forName.newInstance() will work
public:
  class ThreadingDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(ThreadingDocMaker)

  public:
    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<ThreadingDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<ThreadingDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

private:
  static const std::shared_ptr<CompressorStreamFactory> csFactory;

  std::shared_ptr<PerfRunData>
  createPerfRunData(std::shared_ptr<Path> file, bool allowEmptyDocs,
                    const std::wstring &docMakerName) ;

  void doReadTest(std::shared_ptr<Path> file, Type fileType,
                  const std::wstring &expTitle, const std::wstring &expDate,
                  const std::wstring &expBody) ;

public:
  static void assertHeaderLine(const std::wstring &line);

  /* Tests WriteLineDocTask with a bzip2 format. */
  virtual void testBZip2() ;

  /* Tests WriteLineDocTask with a gzip format. */
  virtual void testGZip() ;

  virtual void testRegularFile() ;

  virtual void testCharsReplace() ;

  virtual void testEmptyBody() ;

  virtual void testEmptyTitle() ;

  /** Fail by default when there's only date */
  virtual void testJustDate() ;

  virtual void testLegalJustDate() ;

  virtual void testEmptyDoc() ;

  virtual void testMultiThreaded() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<WriteLineDocTaskTest> outerInstance;

    std::shared_ptr<
        org::apache::lucene::benchmark::byTask::tasks::WriteLineDocTask>
        wldt;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<WriteLineDocTaskTest> outerInstance,
        std::wstring L"t" + i,
        std::shared_ptr<
            org::apache::lucene::benchmark::byTask::tasks::WriteLineDocTask>
            wldt);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<WriteLineDocTaskTest> shared_from_this()
  {
    return std::static_pointer_cast<WriteLineDocTaskTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
