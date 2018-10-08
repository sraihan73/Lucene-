#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <memory>
#include <stdexcept>

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
namespace org::apache::lucene::queryparser::flexible::core::nodes
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestQueryNode : public LuceneTestCase
{
  GET_CLASS_NAME(TestQueryNode)

  /* LUCENE-2227 bug in QueryNodeImpl.add() */
public:
  virtual void testAddChildren() ;

  /* LUCENE-3045 bug in QueryNodeImpl.containsTag(std::wstring key)*/
  virtual void testTags() ;

  /* LUCENE-5099 - QueryNodeProcessorImpl should set parent to null before
   * returning on processing */
  virtual void testRemoveFromParent() ;

  virtual void testRemoveChildren() ;

protected:
  std::shared_ptr<TestQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<TestQueryNode>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
