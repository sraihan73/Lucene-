#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class MockDirectoryWrapper;
}

namespace org::apache::lucene::store
{
class Failure;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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

using Failure = org::apache::lucene::store::MockDirectoryWrapper::Failure;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Causes a bunch of fake VM errors and checks that no other exceptions are
 * delivered instead, no index corruption is ever created.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs("SimpleText") public class
// TestIndexWriterOnVMError extends org.apache.lucene.util.LuceneTestCase
class TestIndexWriterOnVMError : public LuceneTestCase
{

  // just one thread, serial merge policy, hopefully debuggable
private:
  void doTest(std::shared_ptr<Failure> failOn) ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterOnVMError> outerInstance;

    int64_t analyzerSeed = 0;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterOnVMError> outerInstance,
        int64_t analyzerSeed);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  std::shared_ptr<VirtualMachineError>
  getTragedy(std::runtime_error disaster, std::shared_ptr<IndexWriter> writer,
             std::shared_ptr<PrintStream> log);

public:
  virtual void testOOM() ;

private:
  class FailureAnonymousInnerClass : public Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterOnVMError> outerInstance;

    std::shared_ptr<Random> r;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterOnVMError> outerInstance,
        std::shared_ptr<Random> r);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  virtual void testUnknownError() ;

private:
  class FailureAnonymousInnerClass2 : public Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexWriterOnVMError> outerInstance;

    std::shared_ptr<Random> r;

  public:
    FailureAnonymousInnerClass2(
        std::shared_ptr<TestIndexWriterOnVMError> outerInstance,
        std::shared_ptr<Random> r);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass2>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testCheckpoint() throws Exception
  virtual void testCheckpoint() ;

private:
  class FailureAnonymousInnerClass3 : public Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass3)
  private:
    std::shared_ptr<TestIndexWriterOnVMError> outerInstance;

    std::shared_ptr<Random> r;

  public:
    FailureAnonymousInnerClass3(
        std::shared_ptr<TestIndexWriterOnVMError> outerInstance,
        std::shared_ptr<Random> r);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass3>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexWriterOnVMError> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterOnVMError>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
