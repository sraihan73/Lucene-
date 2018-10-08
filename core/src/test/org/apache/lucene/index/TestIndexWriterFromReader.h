#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_set>
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterFromReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterFromReader)

  // Pull NRT reader immediately after writer has committed
public:
  virtual void testRightAfterCommit() ;

  // Open from non-NRT reader
  virtual void testFromNonNRTReader() ;

  // Pull NRT reader from a writer on a new index with no commit:
  virtual void testWithNoFirstCommit() ;

  // Pull NRT reader after writer has committed and then indexed another doc:
  virtual void testAfterCommitThenIndex() ;

  // NRT rollback: pull NRT reader after writer has committed and then before
  // indexing another doc
  virtual void testNRTRollback() ;

  virtual void testRandom() ;

  virtual void testConsistentFieldNumbers() ;

  virtual void testInvalidOpenMode() ;

  virtual void testOnClosedReader() ;

  virtual void testStaleNRTReader() ;

  virtual void testAfterRollback() ;

  // Pull NRT reader after writer has committed and then indexed another doc:
  virtual void testAfterCommitThenIndexKeepCommits() ;

private:
  class IndexDeletionPolicyAnonymousInnerClass : public IndexDeletionPolicy
  {
    GET_CLASS_NAME(IndexDeletionPolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterFromReader> outerInstance;

  public:
    IndexDeletionPolicyAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterFromReader> outerInstance);

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
    // extends IndexCommit> commits)
    void onInit(std::deque<T1> commits);

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
    // extends IndexCommit> commits)
    void onCommit(std::deque<T1> commits);

  protected:
    std::shared_ptr<IndexDeletionPolicyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexDeletionPolicyAnonymousInnerClass>(
          IndexDeletionPolicy::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexWriterFromReader> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterFromReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
