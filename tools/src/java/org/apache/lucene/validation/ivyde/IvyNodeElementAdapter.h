#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::validation::ivyde
{
class IvyNodeElement;
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
namespace org::apache::lucene::validation::ivyde
{

using org::apache::ivy::core::report::ResolveReport;

/**
 * This class is copied from
 * org/apache/ivyde/eclipse/resolvevisualizer/model/IvyNodeElementAdapter.java
 * at
 * https://svn.apache.org/repos/asf/ant/ivy/ivyde/trunk/org.apache.ivyde.eclipse.resolvevisualizer/src/
 *
 * Changes include: uncommenting generics and converting to diamond operators
 * where appropriate; removing unnecessary casts; and removing javadoc tags with
 * no description.
 */
class IvyNodeElementAdapter
    : public std::enable_shared_from_this<IvyNodeElementAdapter>
{
  GET_CLASS_NAME(IvyNodeElementAdapter)
  /**
   * Adapt all dependencies and evictions from the ResolveReport.
   * @return the root node adapted from the ResolveReport
   */
public:
  static std::shared_ptr<IvyNodeElement>
  adapt(std::shared_ptr<ResolveReport> report);

  /**
   * Derives configuration conflicts that exist between node and all of its
   * descendant dependencies.
   */
private:
  static void findConflictsBeneathNode(std::shared_ptr<IvyNodeElement> node);
};
} // namespace org::apache::lucene::validation::ivyde
