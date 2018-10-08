#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::analysis::th
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * Simple tests to ensure the Thai word filter factory is working.
 */
class TestThaiTokenizerFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestThaiTokenizerFactory)
  /**
   * Ensure the filter actually decomposes text.
   */
public:
  virtual void testWordBreak() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

protected:
  std::shared_ptr<TestThaiTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestThaiTokenizerFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::th
