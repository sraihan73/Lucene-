#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test class to illustrate using IndexDeletionPolicy to provide multi-level
 * rollback capability. This test case creates an index of records 1 to 100,
 * introducing a commit point every 10 records.
 *
 * A "keep all" deletion policy is used to ensure we keep all commit points for
 * testing purposes
 */

class TestTransactionRollback : public LuceneTestCase
{
  GET_CLASS_NAME(TestTransactionRollback)

private:
  static const std::wstring FIELD_RECORD_ID;
  std::shared_ptr<Directory> dir;

  // Rolls back index to a chosen ID
  void rollBackLast(int id) ;

public:
  virtual void testRepeatedRollBacks() ;

private:
  void
  checkExpecteds(std::shared_ptr<BitSet> expecteds) ;

  /*
  private void showAvailableCommitPoints() throws Exception {
    std::deque commits = DirectoryReader.listCommits(dir);
    for (Iterator iterator = commits.iterator(); iterator.hasNext();) {
      IndexCommit comm = (IndexCommit) iterator.next();
      System.out.print("\t Available commit point:["+comm.getUserData()+"]
  files="); std::deque files = comm.getFileNames(); for (Iterator iterator2 =
  files.iterator(); iterator2.hasNext();) { std::wstring filename = (std::wstring)
  iterator2.next(); System.out.print(filename+", ");
      }
      System.out.println();
    }
  }
  */

public:
  void setUp()  override;

  void tearDown()  override;

  // Rolls back to previous commit point
public:
  class RollbackDeletionPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(RollbackDeletionPolicy)
  private:
    int rollbackPoint = 0;

  public:
    RollbackDeletionPolicy(int rollbackPoint);

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;

  protected:
    std::shared_ptr<RollbackDeletionPolicy> shared_from_this()
    {
      return std::static_pointer_cast<RollbackDeletionPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

public:
  class DeleteLastCommitPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(DeleteLastCommitPolicy)

  public:
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;

  protected:
    std::shared_ptr<DeleteLastCommitPolicy> shared_from_this()
    {
      return std::static_pointer_cast<DeleteLastCommitPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

public:
  virtual void testRollbackDeletionPolicy() ;

  // Keeps all commit points (used to build index)
public:
  class KeepAllDeletionPolicy : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(KeepAllDeletionPolicy)
  public:
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onCommit(std::deque<T1> commits) ;
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits) throws java.io.IOException
    void onInit(std::deque<T1> commits) ;

  protected:
    std::shared_ptr<KeepAllDeletionPolicy> shared_from_this()
    {
      return std::static_pointer_cast<KeepAllDeletionPolicy>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestTransactionRollback> shared_from_this()
  {
    return std::static_pointer_cast<TestTransactionRollback>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
