#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexDeletionPolicy.h"

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
 * An {@link IndexDeletionPolicy} which keeps all index commits around, never
 * deleting them. This class is a singleton and can be accessed by referencing
 * {@link #INSTANCE}.
GET_CLASS_NAME(is)
 */
class NoDeletionPolicy final : public IndexDeletionPolicy
{
  GET_CLASS_NAME(NoDeletionPolicy)

  /** The single instance of this class. */
public:
  static const std::shared_ptr<IndexDeletionPolicy> INSTANCE;

private:
  NoDeletionPolicy();

public:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
  // extends IndexCommit> commits)
  void onCommit(std::deque<T1> commits);
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
  // extends IndexCommit> commits)
  void onInit(std::deque<T1> commits);

protected:
  std::shared_ptr<NoDeletionPolicy> shared_from_this()
  {
    return std::static_pointer_cast<NoDeletionPolicy>(
        IndexDeletionPolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
