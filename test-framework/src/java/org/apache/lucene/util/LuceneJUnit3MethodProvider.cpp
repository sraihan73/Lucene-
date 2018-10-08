using namespace std;

#include "LuceneJUnit3MethodProvider.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::ClassModel;
using com::carrotsearch::randomizedtesting::TestMethodProvider;
using com::carrotsearch::randomizedtesting::ClassModel::MethodModel;

shared_ptr<deque<std::shared_ptr<Method>>>
LuceneJUnit3MethodProvider::getTestMethods(type_info suiteClass,
                                           shared_ptr<ClassModel> classModel)
{
  unordered_map<std::shared_ptr<Method>,
                std::shared_ptr<ClassModel::MethodModel>>
      methods = classModel->getMethods();
  deque<std::shared_ptr<Method>> result = deque<std::shared_ptr<Method>>();
  for (auto mm : methods) {
    // Skip any methods that have overrieds/ shadows.
    if (mm->second.getDown() != nullptr) {
      continue;
    }

    shared_ptr<Method> m = mm->second.element;
    if (m->getName()->startsWith(L"test") &&
        Modifier::isPublic(m->getModifiers()) &&
        !Modifier::isStatic(m->getModifiers()) &&
        m->getParameterTypes()->length == 0) {
      result.push_back(m);
    }
  }
  return result;
}
} // namespace org::apache::lucene::util