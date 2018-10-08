#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>

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

class TestRollingBuffer : public LuceneTestCase
{
  GET_CLASS_NAME(TestRollingBuffer)

private:
  class Position : public std::enable_shared_from_this<Position>,
                   public RollingBuffer::Resettable
  {
    GET_CLASS_NAME(Position)
  public:
    int pos = 0;

    void reset() override;
  };

public:
  virtual void test();

private:
  class RollingBufferAnonymousInnerClass
      : public RollingBuffer<std::shared_ptr<Position>>
  {
    GET_CLASS_NAME(RollingBufferAnonymousInnerClass)
  private:
    std::shared_ptr<TestRollingBuffer> outerInstance;

  public:
    RollingBufferAnonymousInnerClass(
        std::shared_ptr<TestRollingBuffer> outerInstance);

  protected:
    std::shared_ptr<Position> newInstance() override;

  protected:
    std::shared_ptr<RollingBufferAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RollingBufferAnonymousInnerClass>(
          RollingBuffer<Position>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestRollingBuffer> shared_from_this()
  {
    return std::static_pointer_cast<TestRollingBuffer>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
