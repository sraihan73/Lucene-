#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
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
namespace org::apache::lucene::validation::ivyde
{

using org::apache::ivy::core::module::id::ModuleRevisionId;

/**
 * Assists in the further separation of concerns between the view and the Ivy
 * resolve report. The view looks at the IvyNode in a unique way that can lead
 * to expensive operations if we do not achieve this separation.
 *
 * This class is copied from
 * org/apache/ivyde/eclipse/resolvevisualizer/model/IvyNodeElement.java at
 * https://svn.apache.org/repos/asf/ant/ivy/ivyde/trunk/org.apache.ivyde.eclipse.resolvevisualizer/src/
 *
 * Changes include: uncommenting generics and converting to diamond operators
 * where appropriate; removing unnecessary casts; removing javadoc tags with no
 * description; and adding a hashCode() implementation.
 */
class IvyNodeElement : public std::enable_shared_from_this<IvyNodeElement>
{
  GET_CLASS_NAME(IvyNodeElement)
private:
  std::shared_ptr<ModuleRevisionId> moduleRevisionId;
  bool evicted = false;
  int depth = std::numeric_limits<int>::max() / 10;
  std::shared_ptr<std::deque<std::shared_ptr<IvyNodeElement>>> dependencies =
      std::unordered_set<std::shared_ptr<IvyNodeElement>>();
  std::shared_ptr<std::deque<std::shared_ptr<IvyNodeElement>>> callers =
      std::unordered_set<std::shared_ptr<IvyNodeElement>>();
  std::shared_ptr<std::deque<std::shared_ptr<IvyNodeElement>>> conflicts =
      std::unordered_set<std::shared_ptr<IvyNodeElement>>();

  /**
   * The caller configurations that caused this node to be reached in the
   * resolution, grouped by caller.
   */
  std::unordered_map<std::shared_ptr<IvyNodeElement>, std::deque<std::wstring>>
      callerConfigurationMap =
          std::unordered_map<std::shared_ptr<IvyNodeElement>,
                             std::deque<std::wstring>>();

  /**
   * We try to avoid building the deque of this nodes deep dependencies by
   * storing them in this cache by depth level.
   */
  std::deque<std::shared_ptr<IvyNodeElement>> deepDependencyCache;

public:
  bool equals(std::any obj) override;

  virtual int hashCode();

  virtual std::deque<std::shared_ptr<IvyNodeElement>> getDependencies();

  /**
   * Recursive dependency retrieval
   *
   * @return The array of nodes that represents a node's immediate and
   * transitive dependencies down to an arbitrary depth.
   */
  virtual std::deque<std::shared_ptr<IvyNodeElement>> getDeepDependencies();

  /**
   * Recursive dependency retrieval
   */
private:
  std::shared_ptr<std::deque<std::shared_ptr<IvyNodeElement>>>
  getDeepDependencies(std::shared_ptr<IvyNodeElement> node);

  /**
   * @return An array of configurations by which this module was resolved
   */
public:
  virtual std::deque<std::wstring>
  getCallerConfigurations(std::shared_ptr<IvyNodeElement> caller);

  virtual void
  setCallerConfigurations(std::shared_ptr<IvyNodeElement> caller,
                          std::deque<std::wstring> &configurations);

  virtual std::wstring getOrganization();

  virtual std::wstring getName();

  virtual std::wstring getRevision();

  virtual bool isEvicted();

  virtual void setEvicted(bool evicted);

  virtual int getDepth();

  /**
   * Set this node's depth and recursively update the node's children to
   * relative to the new value.
   */
  virtual void setDepth(int depth);

  virtual std::deque<std::shared_ptr<IvyNodeElement>> getConflicts();

  virtual void setConflicts(
      std::shared_ptr<std::deque<std::shared_ptr<IvyNodeElement>>> conflicts);

  virtual std::shared_ptr<ModuleRevisionId> getModuleRevisionId();

  virtual void
  setModuleRevisionId(std::shared_ptr<ModuleRevisionId> moduleRevisionId);

  virtual void addCaller(std::shared_ptr<IvyNodeElement> caller);

  virtual std::deque<std::shared_ptr<IvyNodeElement>> getCallers();
};
} // #include  "core/src/java/org/apache/lucene/validation/ivyde/
