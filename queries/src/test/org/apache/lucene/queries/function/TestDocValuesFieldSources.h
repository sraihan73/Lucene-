#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
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
namespace org::apache::lucene::queries::function
{

using DocValuesType = org::apache::lucene::index::DocValuesType;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDocValuesFieldSources : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocValuesFieldSources)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") public void
  // test(org.apache.lucene.index.DocValuesType type) throws java.io.IOException
  virtual void test(DocValuesType type) ;

  virtual void test() ;

protected:
  std::shared_ptr<TestDocValuesFieldSources> shared_from_this()
  {
    return std::static_pointer_cast<TestDocValuesFieldSources>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
