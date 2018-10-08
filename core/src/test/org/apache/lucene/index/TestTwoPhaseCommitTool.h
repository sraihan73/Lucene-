#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTwoPhaseCommitTool : public LuceneTestCase
{
  GET_CLASS_NAME(TestTwoPhaseCommitTool)

private:
  class TwoPhaseCommitImpl
      : public std::enable_shared_from_this<TwoPhaseCommitImpl>,
        public TwoPhaseCommit
  {
    GET_CLASS_NAME(TwoPhaseCommitImpl)
  public:
    static bool commitCalled;
    const bool failOnPrepare;
    const bool failOnCommit;
    const bool failOnRollback;
    bool rollbackCalled = false;
    std::unordered_map<std::wstring, std::wstring> prepareCommitData;
    std::unordered_map<std::wstring, std::wstring> commitData;

    TwoPhaseCommitImpl(bool failOnPrepare, bool failOnCommit,
                       bool failOnRollback);

    int64_t prepareCommit()  override;

    virtual int64_t
    prepareCommit(std::unordered_map<std::wstring, std::wstring>
                      &commitData) ;

    int64_t commit()  override;

    virtual int64_t commit(std::unordered_map<std::wstring, std::wstring>
                                 &commitData) ;

    void rollback()  override;
  };

public:
  void setUp()  override;

  virtual void testPrepareThenCommit() ;

  virtual void testRollback() ;

  virtual void testNullTPCs() ;

protected:
  std::shared_ptr<TestTwoPhaseCommitTool> shared_from_this()
  {
    return std::static_pointer_cast<TestTwoPhaseCommitTool>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
