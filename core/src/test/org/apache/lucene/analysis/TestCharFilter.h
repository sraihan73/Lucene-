#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestCharFilter : public LuceneTestCase
{
  GET_CLASS_NAME(TestCharFilter)

public:
  virtual void testCharFilter1() ;

  virtual void testCharFilter2() ;

  virtual void testCharFilter12() ;

  virtual void testCharFilter11() ;

public:
  class CharFilter1 : public CharFilter
  {
    GET_CLASS_NAME(CharFilter1)

  protected:
    CharFilter1(std::shared_ptr<Reader> in_);

  public:
    int read(std::deque<wchar_t> &cbuf, int off,
             int len)  override;

  protected:
    int correct(int currentOff) override;

  protected:
    std::shared_ptr<CharFilter1> shared_from_this()
    {
      return std::static_pointer_cast<CharFilter1>(
          CharFilter::shared_from_this());
    }
  };

public:
  class CharFilter2 : public CharFilter
  {
    GET_CLASS_NAME(CharFilter2)

  protected:
    CharFilter2(std::shared_ptr<Reader> in_);

  public:
    int read(std::deque<wchar_t> &cbuf, int off,
             int len)  override;

  protected:
    int correct(int currentOff) override;

  protected:
    std::shared_ptr<CharFilter2> shared_from_this()
    {
      return std::static_pointer_cast<CharFilter2>(
          CharFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestCharFilter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
