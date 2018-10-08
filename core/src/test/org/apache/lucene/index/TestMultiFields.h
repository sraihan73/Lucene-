#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"

#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMultiFields : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiFields)

public:
  virtual void testRandom() ;

private:
  class FilterMergePolicyAnonymousInnerClass : public FilterMergePolicy
  {
    GET_CLASS_NAME(FilterMergePolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestMultiFields> outerInstance;

  public:
    FilterMergePolicyAnonymousInnerClass(
        std::shared_ptr<TestMultiFields> outerInstance,
        std::shared_ptr<org::apache::lucene::index::MergePolicy> INSTANCE);

    bool keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>> readerIOSupplier) override;

  protected:
    std::shared_ptr<FilterMergePolicyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterMergePolicyAnonymousInnerClass>(
          FilterMergePolicy::shared_from_this());
    }
  };

  /*
  private void verify(IndexReader r, std::wstring term, List<Integer> expected) throws
  Exception { DocsEnum docs = _TestUtil.docs(random, r, "field", new
  BytesRef(term), MultiFields.getLiveDocs(r), null, false); for(int docID :
  expected) { assertEquals(docID, docs.nextDoc());
    }
    assertEquals(docs.NO_MORE_DOCS, docs.nextDoc());
  }
  */

public:
  virtual void testSeparateEnums() ;

  virtual void testTermDocsEnum() ;

protected:
  std::shared_ptr<TestMultiFields> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiFields>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
