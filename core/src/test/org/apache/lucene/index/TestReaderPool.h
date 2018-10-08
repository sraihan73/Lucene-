#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class StandardDirectoryReader;
}

namespace org::apache::lucene::index
{
class MergePolicy;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class FieldNumbers;
}
namespace org::apache::lucene::store
{
class Directory;
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

namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestReaderPool : public LuceneTestCase
{
  GET_CLASS_NAME(TestReaderPool)

public:
  virtual void testDrop() ;

  virtual void testPoolReaders() ;

  virtual void testUpdate() ;

  virtual void testDeletes() ;

  virtual void
  testPassReaderToMergePolicyConcurrently() ;

private:
  class FilterMergePolicyAnonymousInnerClass : public FilterMergePolicy
  {
    GET_CLASS_NAME(FilterMergePolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestReaderPool> outerInstance;

    std::shared_ptr<org::apache::lucene::index::StandardDirectoryReader> reader;

  public:
    FilterMergePolicyAnonymousInnerClass(
        std::shared_ptr<TestReaderPool> outerInstance,
        std::shared_ptr<org::apache::lucene::index::MergePolicy> newMergePolicy,
        std::shared_ptr<org::apache::lucene::index::StandardDirectoryReader>
            reader);

    bool
    keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                                readerIOSupplier)  override;

  protected:
    std::shared_ptr<FilterMergePolicyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterMergePolicyAnonymousInnerClass>(
          FilterMergePolicy::shared_from_this());
    }
  };

private:
  std::shared_ptr<FieldInfos::FieldNumbers>
  buildIndex(std::shared_ptr<Directory> directory) ;

public:
  virtual void testGetReaderByRam() ;

protected:
  std::shared_ptr<TestReaderPool> shared_from_this()
  {
    return std::static_pointer_cast<TestReaderPool>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
