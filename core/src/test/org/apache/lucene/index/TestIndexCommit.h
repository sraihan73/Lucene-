#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexCommit : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexCommit)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEqualsHashCode() throws Exception
  virtual void testEqualsHashCode() ;

private:
  class IndexCommitAnonymousInnerClass : public IndexCommit
  {
    GET_CLASS_NAME(IndexCommitAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexCommit> outerInstance;

    std::shared_ptr<Directory> dir;

  public:
    IndexCommitAnonymousInnerClass(
        std::shared_ptr<TestIndexCommit> outerInstance,
        std::shared_ptr<Directory> dir);

    std::wstring getSegmentsFileName() override;
    std::shared_ptr<Directory> getDirectory() override;
    std::shared_ptr<std::deque<std::wstring>> getFileNames() override;
    void delete_() override;
    int64_t getGeneration() override;
    std::unordered_map<std::wstring, std::wstring> getUserData() override;
    bool isDeleted() override;
    int getSegmentCount() override;

  protected:
    std::shared_ptr<IndexCommitAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexCommitAnonymousInnerClass>(
          IndexCommit::shared_from_this());
    }
  };

private:
  class IndexCommitAnonymousInnerClass2 : public IndexCommit
  {
    GET_CLASS_NAME(IndexCommitAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexCommit> outerInstance;

    std::shared_ptr<Directory> dir;

  public:
    IndexCommitAnonymousInnerClass2(
        std::shared_ptr<TestIndexCommit> outerInstance,
        std::shared_ptr<Directory> dir);

    std::wstring getSegmentsFileName() override;
    std::shared_ptr<Directory> getDirectory() override;
    std::shared_ptr<std::deque<std::wstring>> getFileNames() override;
    void delete_() override;
    int64_t getGeneration() override;
    std::unordered_map<std::wstring, std::wstring> getUserData() override;
    bool isDeleted() override;
    int getSegmentCount() override;

  protected:
    std::shared_ptr<IndexCommitAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<IndexCommitAnonymousInnerClass2>(
          IndexCommit::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexCommit> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexCommit>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
