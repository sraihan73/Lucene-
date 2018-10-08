#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
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
namespace org::apache::lucene::analysis::morfologik
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;

/**
 * Test for {@link MorfologikFilterFactory}.
 */
class TestMorfologikFilterFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMorfologikFilterFactory)
private:
  class ForbidResourcesLoader
      : public std::enable_shared_from_this<ForbidResourcesLoader>,
        public ResourceLoader
  {
    GET_CLASS_NAME(ForbidResourcesLoader)
  public:
    std::shared_ptr<InputStream>
    openResource(const std::wstring &resource)  override;

    template <typename T>
    std::type_info findClass(const std::wstring &cname,
                             std::type_info<T> &expectedType);

    template <typename T>
    T newInstance(const std::wstring &cname, std::type_info<T> &expectedType);
  };

public:
  virtual void testDefaultDictionary() ;

  virtual void testExplicitDictionary() ;

  virtual void testMissingDictionary() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

protected:
  std::shared_ptr<TestMorfologikFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestMorfologikFilterFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/morfologik/
