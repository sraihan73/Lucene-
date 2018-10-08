#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stack>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest::tst
{
class TernaryTreeNode;
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
namespace org::apache::lucene::search::suggest::tst
{

/**
 * Ternary Search Trie implementation.
 *
 * @see TernaryTreeNode
 */
class TSTAutocomplete : public std::enable_shared_from_this<TSTAutocomplete>
{
  GET_CLASS_NAME(TSTAutocomplete)

public:
  TSTAutocomplete();

  /**
   * Inserting keys in TST in the order middle,small,big (lexicographic measure)
   * recursively creates a balanced tree which reduces insertion and search
   * times significantly.
   *
   * @param tokens
   *          Sorted deque of keys to be inserted in TST.
   * @param lo
   *          stores the lower index of current deque.
   * @param hi
   *          stores the higher index of current deque.
   * @param root
   *          a reference object to root of TST.
   */
  virtual void balancedTree(std::deque<std::any> &tokens,
                            std::deque<std::any> &vals, int lo, int hi,
                            std::shared_ptr<TernaryTreeNode> root);

  /**
   * Inserts a key in TST creating a series of Binary Search Trees at each node.
   * The key is actually stored across the eqKid of each node in a successive
   * manner.
   *
   * @param currentNode
   *          a reference node where the insertion will take currently.
   * @param s
   *          key to be inserted in TST.
   * @param x
   *          index of character in key to be inserted currently.
   * @return currentNode The new reference to root node of TST
   */
  virtual std::shared_ptr<TernaryTreeNode>
  insert(std::shared_ptr<TernaryTreeNode> currentNode,
         std::shared_ptr<std::wstring> s, std::any val, int x);

  /**
   * Auto-completes a given prefix query using Depth-First Search with the end
   * of prefix as source node each time finding a new leaf to get a complete key
   * to be added in the suggest deque.
   *
   * @param root
   *          a reference to root node of TST.
   * @param s
   *          prefix query to be auto-completed.
   * @param x
   *          index of current character to be searched while traversing through
   *          the prefix in TST.
   * @return suggest deque of auto-completed keys for the given prefix query.
   */
  virtual std::deque<std::shared_ptr<TernaryTreeNode>>
  prefixCompletion(std::shared_ptr<TernaryTreeNode> root,
                   std::shared_ptr<std::wstring> s, int x);
};

} // namespace org::apache::lucene::search::suggest::tst
