#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

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
namespace org::apache::lucene::search::suggest::tst
{

/**
 * The class creates a TST node.
 */

class TernaryTreeNode : public std::enable_shared_from_this<TernaryTreeNode>
{
  GET_CLASS_NAME(TernaryTreeNode)

  /** Creates a new empty node */
public:
  TernaryTreeNode();
  /** the character stored by a node. */
  wchar_t splitchar = L'\0';
  /** a reference object to the node containing character smaller than this
   * node's character. */
  std::shared_ptr<TernaryTreeNode> loKid;
  /**
   *  a reference object to the node containing character next to this node's
   * character as occurring in the inserted token.
   */
  std::shared_ptr<TernaryTreeNode> eqKid;
  /** a reference object to the node containing character higher than this
   * node's character. */
  std::shared_ptr<TernaryTreeNode> hiKid;
  /**
   * used by leaf nodes to store the complete tokens to be added to suggest deque
   * while auto-completing the prefix.
   */
  std::wstring token;
  std::any val;

  virtual int64_t sizeInBytes();
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/tst/
