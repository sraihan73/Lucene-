#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/RAMDirectory.h"

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
namespace org::apache::lucene::store
{

class TestFilterDirectory : public BaseDirectoryTestCase
{
  GET_CLASS_NAME(TestFilterDirectory)

protected:
  std::shared_ptr<Directory> getDirectory(std::shared_ptr<Path> path) override;

private:
  class FilterDirectoryAnonymousInnerClass : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestFilterDirectory> outerInstance;

  public:
    FilterDirectoryAnonymousInnerClass(
        std::shared_ptr<TestFilterDirectory> outerInstance,
        std::shared_ptr<RAMDirectory> new) new;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
          FilterDirectory::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOverrides() throws Exception
  virtual void testOverrides() ;

  virtual void testUnwrap() ;

private:
  class FilterDirectoryAnonymousInnerClass2 : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass2)
  private:
    std::shared_ptr<TestFilterDirectory> outerInstance;

  public:
    FilterDirectoryAnonymousInnerClass2(
        std::shared_ptr<TestFilterDirectory> outerInstance,
        std::shared_ptr<org::apache::lucene::store::Directory> dir);

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass2>(
          FilterDirectory::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestFilterDirectory> shared_from_this()
  {
    return std::static_pointer_cast<TestFilterDirectory>(
        BaseDirectoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
