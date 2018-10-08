#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
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
namespace org::apache::lucene::analysis::util
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFilesystemResourceLoader : public LuceneTestCase
{
  GET_CLASS_NAME(TestFilesystemResourceLoader)

private:
  void
  assertNotFound(std::shared_ptr<ResourceLoader> rl) ;

  void assertClasspathDelegation(std::shared_ptr<ResourceLoader> rl) throw(
      std::runtime_error);

public:
  virtual void testBaseDir() ;

  virtual void testDelegation() ;

protected:
  std::shared_ptr<TestFilesystemResourceLoader> shared_from_this()
  {
    return std::static_pointer_cast<TestFilesystemResourceLoader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::util
