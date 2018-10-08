using namespace std;

#include "IvyNodeElementAdapter.h"

namespace org::apache::lucene::validation::ivyde
{
using org::apache::ivy::core::module::id::ModuleId;
using org::apache::ivy::core::module::id::ModuleRevisionId;
using org::apache::ivy::core::report::ResolveReport;
using org::apache::ivy::core::resolve::IvyNode;
using org::apache::ivy::core::resolve::IvyNodeCallers;

shared_ptr<IvyNodeElement>
IvyNodeElementAdapter::adapt(shared_ptr<ResolveReport> report)
{
  unordered_map<std::shared_ptr<ModuleRevisionId>,
                std::shared_ptr<IvyNodeElement>>
      resolvedNodes = unordered_map<std::shared_ptr<ModuleRevisionId>,
                                    std::shared_ptr<IvyNodeElement>>();

  shared_ptr<IvyNodeElement> root = make_shared<IvyNodeElement>();
  root->setModuleRevisionId(
      report->getModuleDescriptor().getModuleRevisionId());
  resolvedNodes.emplace(report->getModuleDescriptor().getModuleRevisionId(),
                        root);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked")
  // java.util.List<org.apache.ivy.core.resolve.IvyNode> dependencies =
  // report.getDependencies();
  deque<std::shared_ptr<IvyNode>> dependencies = report->getDependencies();

  // First pass - build the map_obj of resolved nodes by revision id
  for (deque<std::shared_ptr<IvyNode>>::const_iterator iter =
           dependencies.begin();
       iter != dependencies.end(); ++iter) {
    shared_ptr<IvyNode> node = *iter;
    if (node->getAllEvictingNodes() != nullptr) {
      // Nodes that are evicted as a result of conf inheritance still appear
      // as dependencies, but with eviction data. They also appear as evictions.
      // We map_obj them as evictions rather than dependencies.
      continue;
    }
    shared_ptr<IvyNodeElement> nodeElement = make_shared<IvyNodeElement>();
    nodeElement->setModuleRevisionId(node->getResolvedId());
    resolvedNodes.emplace(node->getResolvedId(), nodeElement);
  }

  // Second pass - establish relationships between the resolved nodes
  for (deque<std::shared_ptr<IvyNode>>::const_iterator iter =
           dependencies.begin();
       iter != dependencies.end(); ++iter) {
    shared_ptr<IvyNode> node = *iter;
    if (node->getAllEvictingNodes() != nullptr) {
      continue; // see note above
    }

    shared_ptr<IvyNodeElement> nodeElement =
        resolvedNodes[node->getResolvedId()];
    std::deque<std::shared_ptr<IvyNodeCallers::Caller>> callers =
        node->getAllRealCallers();
    for (int i = 0; i < callers.size(); i++) {
      shared_ptr<IvyNodeElement> caller =
          resolvedNodes[callers[i]->getModuleRevisionId()];
      if (caller != nullptr) {
        nodeElement->addCaller(caller);
        nodeElement->setCallerConfigurations(
            caller, callers[i]->getCallerConfigurations());
      }
    }
  }

  std::deque<std::shared_ptr<IvyNode>> evictions = report->getEvictedNodes();
  for (int i = 0; i < evictions.size(); i++) {
    shared_ptr<IvyNode> eviction = evictions[i];
    shared_ptr<IvyNodeElement> evictionElement = make_shared<IvyNodeElement>();
    evictionElement->setModuleRevisionId(eviction->getResolvedId());
    evictionElement->setEvicted(true);

    std::deque<std::shared_ptr<IvyNodeCallers::Caller>> callers =
        eviction->getAllCallers();
    for (int j = 0; j < callers.size(); j++) {
      shared_ptr<IvyNodeElement> caller =
          resolvedNodes[callers[j]->getModuleRevisionId()];
      if (caller != nullptr) {
        evictionElement->addCaller(caller);
        evictionElement->setCallerConfigurations(
            caller, callers[j]->getCallerConfigurations());
      }
    }
  }

  // Recursively set depth starting at root
  root->setDepth(0);
  findConflictsBeneathNode(root);

  return root;
}

void IvyNodeElementAdapter::findConflictsBeneathNode(
    shared_ptr<IvyNodeElement> node)
{
  // Derive conflicts
  unordered_map<std::shared_ptr<ModuleId>,
                deque<std::shared_ptr<IvyNodeElement>>>
      moduleRevisionMap =
          unordered_map<std::shared_ptr<ModuleId>,
                        deque<std::shared_ptr<IvyNodeElement>>>();
  std::deque<std::shared_ptr<IvyNodeElement>> deepDependencies =
      node->getDeepDependencies();
  for (int i = 0; i < deepDependencies.size(); i++) {
    if (deepDependencies[i]->isEvicted()) {
      continue;
    }

    shared_ptr<ModuleId> moduleId =
        deepDependencies[i]->getModuleRevisionId().getModuleId();
    if (moduleRevisionMap.find(moduleId) != moduleRevisionMap.end()) {
      shared_ptr<deque<std::shared_ptr<IvyNodeElement>>> conflicts =
          moduleRevisionMap[moduleId];
      conflicts->add(deepDependencies[i]);
      for (shared_ptr<
               deque<std::shared_ptr<IvyNodeElement>>::const_iterator>
               iter = conflicts->begin();
           iter != conflicts->end(); ++iter) {
        shared_ptr<IvyNodeElement> conflict = *iter;
        conflict->setConflicts(conflicts);
      }
    } else {
      deque<std::shared_ptr<IvyNodeElement>> immutableMatchingSet =
          Arrays::asList(deepDependencies[i]);
      moduleRevisionMap.emplace(moduleId,
                                unordered_set<>(immutableMatchingSet));
    }
  }
}
} // namespace org::apache::lucene::validation::ivyde