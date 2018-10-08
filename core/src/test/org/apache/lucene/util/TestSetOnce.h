#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
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
namespace org::apache::lucene::util
{

class TestSetOnce : public LuceneTestCase
{
  GET_CLASS_NAME(TestSetOnce)

private:
  class SetOnceThread final : public Thread
  {
    GET_CLASS_NAME(SetOnceThread)
  public:
    std::shared_ptr<SetOnce<int>> set;
    bool success = false;
    const std::shared_ptr<Random> RAND;

    SetOnceThread(std::shared_ptr<Random> random);

    void run() override;

  protected:
    std::shared_ptr<SetOnceThread> shared_from_this()
    {
      return std::static_pointer_cast<SetOnceThread>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptyCtor() throws Exception
  virtual void testEmptyCtor() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE:
  // @Test(expected=org.apache.lucene.util.SetOnce.AlreadySetException.class)
  // public void testSettingCtor() throws Exception
  virtual void testSettingCtor() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE:
  // @Test(expected=org.apache.lucene.util.SetOnce.AlreadySetException.class)
  // public void testSetOnce() throws Exception
  virtual void testSetOnce() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSetMultiThreaded() throws Exception
  virtual void testSetMultiThreaded() ;

protected:
  std::shared_ptr<TestSetOnce> shared_from_this()
  {
    return std::static_pointer_cast<TestSetOnce>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
