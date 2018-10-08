#pragma once
#include "stringhelper.h"
#include <memory>
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

/**
 * This {@link IndexDeletionPolicy} implementation that
 * keeps only the most recent commit and immediately removes
 * all prior commits after a new commit is done.  This is
 * the default deletion policy.
 */

class KeepOnlyLastCommitDeletionPolicy final : public IndexDeletionPolicy
{
  GET_CLASS_NAME(KeepOnlyLastCommitDeletionPolicy)

  /** Sole constructor. */
public:
  KeepOnlyLastCommitDeletionPolicy();

  /**
   * Deletes all commits except the most recent one.
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
  // extends IndexCommit> commits)
  void onInit(std::deque<T1> commits);

  /**
   * Deletes all commits except the most recent one.
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
  // extends IndexCommit> commits)
  void onCommit(std::deque<T1> commits);

protected:
  std::shared_ptr<KeepOnlyLastCommitDeletionPolicy> shared_from_this()
  {
    return std::static_pointer_cast<KeepOnlyLastCommitDeletionPolicy>(
        IndexDeletionPolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
