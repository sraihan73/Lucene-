#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest::analyzing
{
class SuggestStopFilterFactory;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestSuggestStopFilterFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestSuggestStopFilterFactory)

public:
  virtual void testInform() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusFormats() ;

private:
  std::shared_ptr<SuggestStopFilterFactory>
  createFactory(std::deque<std::wstring> &params) ;

protected:
  std::shared_ptr<TestSuggestStopFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestSuggestStopFilterFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing