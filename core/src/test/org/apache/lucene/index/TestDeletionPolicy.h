#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexCommit;
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/*
  Verify we can read the pre-2.1 file format, do searches
  against it, and add documents to it.
*/
class TestDeletionPolicy : public LuceneTestCase
{
  GET_CLASS_NAME(TestDeletionPolicy)

private:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private void verifyCommitOrder(java.util.List<?
  // extends IndexCommit> commits)
  void verifyCommitOrder(std::deque<T1> commits);

public:
  class KeepAllDeletionPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(KeepAllDeletionPolicy)
  private:
    std::shared_ptr<TestDeletionPolicy> outerInstance;

  public:
    int numOnInit = 0;
    int numOnCommit = 0;
    std::shared_ptr<Directory> dir;

    KeepAllDeletionPolicy(std::shared_ptr<TestDeletionPolicy> outerInstance,
                          std::shared_ptr<Directory> dir);

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;

  protected:
    std::shared_ptr<KeepAllDeletionPolicy> shared_from_this()
    {
      return std::static_pointer_cast<KeepAllDeletionPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

  /**
   * This is useful for adding to a big index when you know
   * readers are not using it.
   */
public:
  class KeepNoneOnInitDeletionPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(KeepNoneOnInitDeletionPolicy)
  private:
    std::shared_ptr<TestDeletionPolicy> outerInstance;

  public:
    KeepNoneOnInitDeletionPolicy(
        std::shared_ptr<TestDeletionPolicy> outerInstance);

    int numOnInit = 0;
    int numOnCommit = 0;
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;

  protected:
    std::shared_ptr<KeepNoneOnInitDeletionPolicy> shared_from_this()
    {
      return std::static_pointer_cast<KeepNoneOnInitDeletionPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

public:
  class KeepLastNDeletionPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(KeepLastNDeletionPolicy)
  private:
    std::shared_ptr<TestDeletionPolicy> outerInstance;

  public:
    int numOnInit = 0;
    int numOnCommit = 0;
    int numToKeep = 0;
    int numDelete = 0;
    std::shared_ptr<Set<std::wstring>> seen =
        std::unordered_set<std::wstring>();

    KeepLastNDeletionPolicy(std::shared_ptr<TestDeletionPolicy> outerInstance,
                            int numToKeep);

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;

  private:
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: private void doDeletes(java.util.List<? extends
    // IndexCommit> commits, bool isCommit)
    void doDeletes(std::deque<T1> commits, bool isCommit);

  protected:
    std::shared_ptr<KeepLastNDeletionPolicy> shared_from_this()
    {
      return std::static_pointer_cast<KeepLastNDeletionPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

public:
  static int64_t
  getCommitTime(std::shared_ptr<IndexCommit> commit) ;

  /*
   * Delete a commit only when it has been obsoleted by N
   * seconds.
   */
public:
  class ExpirationTimeDeletionPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(ExpirationTimeDeletionPolicy)
  private:
    std::shared_ptr<TestDeletionPolicy> outerInstance;

  public:
    std::shared_ptr<Directory> dir;
    double expirationTimeSeconds = 0;
    int numDelete = 0;

    ExpirationTimeDeletionPolicy(
        std::shared_ptr<TestDeletionPolicy> outerInstance,
        std::shared_ptr<Directory> dir, double seconds);

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;

  protected:
    std::shared_ptr<ExpirationTimeDeletionPolicy> shared_from_this()
    {
      return std::static_pointer_cast<ExpirationTimeDeletionPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

  /*
   * Test "by time expiration" deletion policy:
   */
public:
  virtual void testExpirationTimeDeletionPolicy() throw(IOException,
                                                        InterruptedException);

  /*
   * Test a silly deletion policy that keeps all commits around.
   */
  virtual void testKeepAllDeletionPolicy() ;

  /* Uses KeepAllDeletionPolicy to keep all commits around,
   * then, opens a new IndexWriter on a previous commit
   * point. */
  virtual void testOpenPriorSnapshot() ;

  /* Test keeping NO commit points.  This is a viable and
   * useful case eg where you want to build a big index and
   * you know there are no readers.
   */
  virtual void testKeepNoneOnInitDeletionPolicy() ;

  /*
   * Test a deletion policy that keeps last N commits.
   */
  virtual void testKeepLastNDeletionPolicy() ;

  /*
   * Test a deletion policy that keeps last N commits
   * around, through creates.
   */
  virtual void testKeepLastNDeletionPolicyWithCreates() ;

private:
  void addDocWithID(std::shared_ptr<IndexWriter> writer,
                    int id) ;

  void addDoc(std::shared_ptr<IndexWriter> writer) ;

protected:
  std::shared_ptr<TestDeletionPolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestDeletionPolicy>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
