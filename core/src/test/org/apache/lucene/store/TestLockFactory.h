#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Lock;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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
namespace org::apache::lucene::store
{

using IndexWriter = org::apache::lucene::index::IndexWriter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestLockFactory : public LuceneTestCase
{
  GET_CLASS_NAME(TestLockFactory)

  // Verify: we can provide our own LockFactory implementation, the right
  // methods are called at the right time, locks are created, etc.

public:
  virtual void testCustomLockFactory() ;

  // Verify: we can use the NoLockFactory with RAMDirectory w/ no
  // exceptions raised:
  // Verify: NoLockFactory allows two IndexWriters
  virtual void testRAMDirectoryNoLocking() ;

public:
  class MockLockFactory : public LockFactory
  {
    GET_CLASS_NAME(MockLockFactory)

  public:
    std::unordered_map<std::wstring, std::shared_ptr<Lock>> locksCreated =
        Collections::synchronizedMap(
            std::unordered_map<std::wstring, std::shared_ptr<Lock>>());

    // C++ WARNING: The following method was originally marked 'synchronized':
    std::shared_ptr<Lock> obtainLock(std::shared_ptr<Directory> dir,
                                     const std::wstring &lockName) override;

  public:
    class MockLock : public Lock
    {
      GET_CLASS_NAME(MockLock)

    public:
      virtual ~MockLock();

      void ensureValid()  override;

    protected:
      std::shared_ptr<MockLock> shared_from_this()
      {
        return std::static_pointer_cast<MockLock>(Lock::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<MockLockFactory> shared_from_this()
    {
      return std::static_pointer_cast<MockLockFactory>(
          LockFactory::shared_from_this());
    }
  };

private:
  void addDoc(std::shared_ptr<IndexWriter> writer) ;

protected:
  std::shared_ptr<TestLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestLockFactory>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
