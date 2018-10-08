using namespace std;

#include "IvyNodeElement.h"

namespace org::apache::lucene::validation::ivyde
{
using org::apache::ivy::core::module::id::ModuleRevisionId;

bool IvyNodeElement::equals(any obj)
{
  if (std::dynamic_pointer_cast<IvyNodeElement>(obj) != nullptr) {
    shared_ptr<IvyNodeElement> elem =
        any_cast<std::shared_ptr<IvyNodeElement>>(obj);
    if (elem->getOrganization() == getOrganization() &&
        elem->getName() == getName() && elem->getRevision() == getRevision()) {
      return true;
    }
  }
  return false;
}

int IvyNodeElement::hashCode()
{
  int result = 1;
  result = result * 31 +
           (L"" == getOrganization() ? 0 : getOrganization().hashCode());
  result = result * 31 + (L"" == getName() ? 0 : getName().hashCode());
  result = result * 31 + (L"" == getRevision() ? 0 : getRevision().hashCode());
  return result;
}

std::deque<std::shared_ptr<IvyNodeElement>> IvyNodeElement::getDependencies()
{
  return dependencies->toArray(
      std::deque<std::shared_ptr<IvyNodeElement>>(dependencies->size()));
}

std::deque<std::shared_ptr<IvyNodeElement>>
IvyNodeElement::getDeepDependencies()
{
  if (deepDependencyCache.empty()) {
    shared_ptr<deque<std::shared_ptr<IvyNodeElement>>> deepDependencies =
        getDeepDependencies(shared_from_this());
    deepDependencyCache = deepDependencies->toArray(
        std::deque<std::shared_ptr<IvyNodeElement>>(deepDependencies->size()));
  }
  return deepDependencyCache;
}

shared_ptr<deque<std::shared_ptr<IvyNodeElement>>>
IvyNodeElement::getDeepDependencies(shared_ptr<IvyNodeElement> node)
{
  shared_ptr<deque<std::shared_ptr<IvyNodeElement>>> deepDependencies =
      unordered_set<std::shared_ptr<IvyNodeElement>>();
  deepDependencies->add(node);

  std::deque<std::shared_ptr<IvyNodeElement>> directDependencies =
      node->getDependencies();
  for (int i = 0; i < directDependencies.size(); i++) {
    deepDependencies->addAll(getDeepDependencies(directDependencies[i]));
  }

  return deepDependencies;
}

std::deque<wstring>
IvyNodeElement::getCallerConfigurations(shared_ptr<IvyNodeElement> caller)
{
  return callerConfigurationMap[caller];
}

void IvyNodeElement::setCallerConfigurations(
    shared_ptr<IvyNodeElement> caller, std::deque<wstring> &configurations)
{
  callerConfigurationMap.emplace(caller, configurations);
}

wstring IvyNodeElement::getOrganization()
{
  return moduleRevisionId->getOrganisation();
}

wstring IvyNodeElement::getName() { return moduleRevisionId->getName(); }

wstring IvyNodeElement::getRevision()
{
  return moduleRevisionId->getRevision();
}

bool IvyNodeElement::isEvicted() { return evicted; }

void IvyNodeElement::setEvicted(bool evicted) { this->evicted = evicted; }

int IvyNodeElement::getDepth() { return depth; }

void IvyNodeElement::setDepth(int depth)
{
  this->depth = depth;
  for (shared_ptr<deque<std::shared_ptr<IvyNodeElement>>::const_iterator>
           iter = dependencies->begin();
       iter != dependencies->end(); ++iter) {
    shared_ptr<IvyNodeElement> dependency = *iter;
    dependency->setDepth(depth + 1);
  }
}

std::deque<std::shared_ptr<IvyNodeElement>> IvyNodeElement::getConflicts()
{
  return conflicts->toArray(
      std::deque<std::shared_ptr<IvyNodeElement>>(conflicts->size()));
}

void IvyNodeElement::setConflicts(
    shared_ptr<deque<std::shared_ptr<IvyNodeElement>>> conflicts)
{
  this->conflicts = conflicts;
}

shared_ptr<ModuleRevisionId> IvyNodeElement::getModuleRevisionId()
{
  return moduleRevisionId;
}

void IvyNodeElement::setModuleRevisionId(
    shared_ptr<ModuleRevisionId> moduleRevisionId)
{
  this->moduleRevisionId = moduleRevisionId;
}

void IvyNodeElement::addCaller(shared_ptr<IvyNodeElement> caller)
{
  callers->add(caller);
  caller->dependencies->add(shared_from_this());
}

std::deque<std::shared_ptr<IvyNodeElement>> IvyNodeElement::getCallers()
{
  return callers->toArray(
      std::deque<std::shared_ptr<IvyNodeElement>>(callers->size()));
}
} // namespace org::apache::lucene::validation::ivyde