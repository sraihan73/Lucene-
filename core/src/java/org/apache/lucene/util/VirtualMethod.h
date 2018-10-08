#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
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
namespace org::apache::lucene::util
{

/**
 * A utility for keeping backwards compatibility on previously abstract methods
 * (or similar replacements).
 * <p>Before the replacement method can be made abstract, the old method must
kept deprecated.
 * If somebody still overrides the deprecated method in a non-final class,
 * you must keep track, of this and maybe delegate to the old method in the
subclass.
 * The cost of reflection is minimized by the following usage of this class:</p>
 * <p>Define <strong>static final</strong> fields in the base class ({@code
BaseClass}), GET_CLASS_NAME(:</p>)
 * where the old and new method are declared:</p>
 * <pre class="prettyprint">
 *  static final VirtualMethod&lt;BaseClass&gt; newMethod =
 *   new VirtualMethod&lt;BaseClass&gt;(BaseClass.class, "newName",
parameters...);
 *  static final VirtualMethod&lt;BaseClass&gt; oldMethod =
 *   new VirtualMethod&lt;BaseClass&gt;(BaseClass.class, "oldName",
parameters...);
 * </pre>
 * <p>This enforces the singleton status of these objects, as the maintenance of
the cache would be too costly else.
 * If you try to create a second instance of for the same method/{@code
baseClass} combination, an exception is thrown.
 * <p>To detect if e.g. the old method was overridden by a more far subclass on
the inheritance path to the current
 * instance's class, use a <strong>non-static</strong> field:</p>
 * <pre class="prettyprint">
 *  final bool isDeprecatedMethodOverridden =
 *   oldMethod.getImplementationDistance(this.getClass()) &gt;
newMethod.getImplementationDistance(this.getClass());
 *
 *  <em>// alternatively (more readable):</em>
 *  final bool isDeprecatedMethodOverridden =
 *   VirtualMethod.compareImplementationDistance(this.getClass(), oldMethod,
newMethod) &gt; 0
 * </pre>
 * <p>{@link #getImplementationDistance} returns the distance of the subclass
that overrides this method.
 * The one with the larger distance should be used preferable.
 * This way also more complicated method rename scenarios can be handled
 * (think of 2.9 {@code TokenStream} deprecations).</p>
 *
 * @lucene.internal
 */
template <typename C>
class VirtualMethod final : public std::enable_shared_from_this<VirtualMethod>
{
  GET_CLASS_NAME(VirtualMethod)

private:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<Set<std::shared_ptr<Method>>> singletonSet =
      Collections::synchronizedSet(
          std::unordered_set<std::shared_ptr<Method>>());

  const std::type_info<C> baseClass;
  const std::wstring method;
  std::deque<std::type_info> const parameters;
  const std::shared_ptr<ClassValue<int>> distanceOfClass =
      std::make_shared<ClassValueAnonymousInnerClass>();

private:
  class ClassValueAnonymousInnerClass : public ClassValue<int>
  {
    GET_CLASS_NAME(ClassValueAnonymousInnerClass)
  public:
    ClassValueAnonymousInnerClass();

  protected:
    std::optional<int> computeValue(std::type_info subclazz) override;

  protected:
    std::shared_ptr<ClassValueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ClassValueAnonymousInnerClass>(
          ClassValue<int>::shared_from_this());
    }
  };

  /**
   * Creates a new instance for the given {@code baseClass} and method
   * declaration.
   * @throws UnsupportedOperationException if you create a second instance of
   * the same
   *  {@code baseClass} and method declaration combination. This enforces the
   * singleton status.
   * @throws IllegalArgumentException if {@code baseClass} does not declare the
   * given method.
   */
public:
  VirtualMethod(std::type_info<C> &baseClass, const std::wstring &method,
                std::deque<std::type_info> &parameters)
      : baseClass(baseClass), method(method), parameters(parameters)
  {
    try {
      if (!singletonSet->add(baseClass.getDeclaredMethod(method, parameters))) {
        throw std::make_shared<UnsupportedOperationException>(
            std::wstring(
                L"VirtualMethod instances must be singletons and therefore ") +
            L"assigned to static final members in the same class, they use as "
            L"baseClass ctor param.");
      }
    } catch (const NoSuchMethodException &nsme) {
      throw std::invalid_argument(
          baseClass.getName() + L" has no such method: " + nsme->getMessage());
    }
  }

  /**
   * Returns the distance from the {@code baseClass} in which this method is
   * overridden/implemented in the inheritance path between {@code baseClass}
   * and the given subclass {@code subclazz}.
   * @return 0 iff not overridden, else the distance to the base class
   */
  int getImplementationDistance(std::type_info const subclazz)
  {
    return distanceOfClass->get(subclazz).intValue();
  }

  /**
   * Returns, if this method is overridden/implemented in the inheritance path
between
   * {@code baseClass} and the given subclass {@code subclazz}.
   * <p>You can use this method to detect if a method that should normally be
final was overridden
   * by the given instance's class.
   * @return {@code false} iff not overridden
GET_CLASS_NAME(.)
   */
  bool isOverriddenAsOf(std::type_info const subclazz)
  {
    return getImplementationDistance(subclazz) > 0;
  }

  int reflectImplementationDistance(std::type_info const subclazz)
  {
    if (!baseClass.isAssignableFrom(subclazz)) {
      throw std::invalid_argument(
          subclazz.getName() + L" is not a subclass of " + baseClass.getName());
    }
    bool overridden = false;
    int distance = 0;
    for (std::type_info clazz = subclazz;
         clazz != baseClass && clazz != nullptr;
         clazz = clazz.getSuperclass()) {
GET_CLASS_NAME(()))
          // lookup method, if success mark as overridden
          if (!overridden)
{
  try {
    clazz.getDeclaredMethod(method, parameters);
    overridden = true;
  } catch (const NoSuchMethodException &nsme) {
  }
}

// increment distance if overridden
if (overridden) {
  distance++;
}
    }
    return distance;
  }

  /**
   * Utility method that compares the implementation/override distance of two
methods.
   * @return <ul>
   *  <li>&gt; 1, iff {@code m1} is overridden/implemented in a subclass of the
class overriding/declaring {@code m2}
   *  <li>&lt; 1, iff {@code m2} is overridden in a subclass of the class
overriding/declaring {@code m1} GET_CLASS_NAME(of)
   *  <li>0, iff both methods are overridden in the same class (or are not
overridden at all)
   * </ul>
   */
  template <typename C>
  static int compareImplementationDistance(std::type_info const clazz,
                                           std::shared_ptr<VirtualMethod<C>> m1,
                                           std::shared_ptr<VirtualMethod<C>> m2)
  {
    return Integer::compare(m1->getImplementationDistance(clazz),
                            m2->getImplementationDistance(clazz));
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
