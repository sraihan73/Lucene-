#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"

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
namespace org::apache::lucene::queryparser::flexible::core::util
{

using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

/**
 * Allow joining 2 QueryNode Trees, into one.
 */
class QueryNodeOperation final
    : public std::enable_shared_from_this<QueryNodeOperation>
{
  GET_CLASS_NAME(QueryNodeOperation)
private:
  QueryNodeOperation();

private:
  enum class ANDOperation { GET_CLASS_NAME(ANDOperation) BOTH, Q1, Q2, NONE };

  /**
   * perform a logical and of 2 QueryNode trees. if q1 and q2 are ANDQueryNode
   * nodes it uses head Node from q1 and adds the children of q2 to q1 if q1 is
   * a AND node and q2 is not, add q2 as a child of the head node of q1 if q2 is
   * a AND node and q1 is not, add q1 as a child of the head node of q2 if q1
   * and q2 are not ANDQueryNode nodes, create a AND node and make q1 and q2
   * children of that node if q1 or q2 is null it returns the not null node if
   * q1 = q2 = null it returns null
   */
public:
  static std::shared_ptr<QueryNode> logicalAnd(std::shared_ptr<QueryNode> q1,
                                               std::shared_ptr<QueryNode> q2);
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/util/
