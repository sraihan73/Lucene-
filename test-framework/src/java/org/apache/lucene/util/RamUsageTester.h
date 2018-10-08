#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
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

/** Crawls object graph to collect RAM usage for testing */
class RamUsageTester final : public std::enable_shared_from_this<RamUsageTester>
{
  GET_CLASS_NAME(RamUsageTester)

  /** An accumulator of object references. This class allows for customizing RAM
   * usage estimation. */
public:
  class Accumulator : public std::enable_shared_from_this<Accumulator>
  {
    GET_CLASS_NAME(Accumulator)

    /** Accumulate transitive references for the provided fields of the given
     *  object into <code>queue</code> and return the shallow size of this
     * object. */
  public:
    virtual int64_t accumulateObject(
        std::any o, int64_t shallowSize,
        std::unordered_map<std::shared_ptr<Field>, std::any> &fieldValues,
        std::shared_ptr<std::deque<std::any>> queue);

    /** Accumulate transitive references for the provided values of the given
     *  array into <code>queue</code> and return the shallow size of this array.
     */
    virtual int64_t
    accumulateArray(std::any array_, int64_t shallowSize,
                    std::deque<std::any> &values,
                    std::shared_ptr<std::deque<std::any>> queue);
  };

  /**
   * Estimates the RAM usage by the given object. It will
   * walk the object tree and sum up all referenced objects.
   *
   * <p><b>Resource Usage:</b> This method internally uses a set of
   * every object seen during traversals so it does allocate memory
   * (it isn't side-effect free). After the method exits, this memory
   * should be GCed.</p>
   */
public:
  static int64_t sizeOf(std::any obj,
                          std::shared_ptr<Accumulator> accumulator);

  /** Same as calling <code>sizeOf(obj, DEFAULT_FILTER)</code>. */
  static int64_t sizeOf(std::any obj);

  /**
   * Return a human-readable size of a given object.
   * @see #sizeOf(Object)
   * @see RamUsageEstimator#humanReadableUnits(long)
   */
  static std::wstring humanSizeOf(std::any object);

  /*
   * Non-recursive version of object descend. This consumes more memory than
   * recursive in-depth traversal but prevents stack overflows on long chains of
   * objects or complex graphs (a max. recursion depth on my machine was ~5000
   * objects linked in a chain so not too much).
   */
private:
  static int64_t measureObjectSize(std::any root,
                                     std::shared_ptr<Accumulator> accumulator);

private:
  class AbstractListAnonymousInnerClass : public AbstractList<std::any>
  {
    GET_CLASS_NAME(AbstractListAnonymousInnerClass)
  private:
    std::any ob;
    int len = 0;

  public:
    AbstractListAnonymousInnerClass(std::any ob, int len);

    std::any get(int index) override;

    int size() override;

  protected:
    std::shared_ptr<AbstractListAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AbstractListAnonymousInnerClass>(
          java.util.AbstractList<Object>::shared_from_this());
    }
  };

  /**
   * This map_obj contains a function to calculate sizes of some "simple types" like
   * std::wstring just from their public properties. This is needed for Java 9, which
   * does not allow to look into runtime class fields.
   */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("serial") private static final
  // java.util.Map<Class, java.util.function.ToLongFunction<Object>> SIMPLE_TYPES
  // = java.util.Collections.unmodifiableMap(new java.util.IdentityHashMap<Class,
  // java.util.function.ToLongFunction<Object>>()
  static const std::unordered_map<std::type_info,
                                  std::function<int64_t(std::any *)>>
      SIMPLE_TYPES;

private:
  class IdentityHashMapAnonymousInnerClass
      : public IdentityHashMap<std::type_info,
                               std::function<int64_t(std::any *)>>
  {
    GET_CLASS_NAME(IdentityHashMapAnonymousInnerClass)
  public:
    IdentityHashMapAnonymousInnerClass(std::shared_ptr<UnknownType> std);

    //      {
    //          init();
    //      }

  private:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressForbidden(reason = "We measure some forbidden
    // classes") private void init()
    void init();

    template <typename T>
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") private <T> void a(Class<T>
    // clazz, java.util.function.ToLongFunction<T> func)
    void a(std::type_info<T> &clazz, std::function<int64_t(T *)> &func);

    int64_t charArraySize(int len);

    int64_t byteArraySize(int len);

  protected:
    std::shared_ptr<IdentityHashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IdentityHashMapAnonymousInnerClass>(
          java.util.IdentityHashMap<
              Class, std::function<long(std::any *)>>::shared_from_this());
    }
  };

  /**
   * Cached information about a given class.
   */
private:
  class ClassCache final : public std::enable_shared_from_this<ClassCache>
  {
    GET_CLASS_NAME(ClassCache)
  public:
    const int64_t alignedShallowInstanceSize;
    std::deque<std::shared_ptr<Field>> const referenceFields;

    ClassCache(int64_t alignedShallowInstanceSize,
               std::deque<std::shared_ptr<Field>> &referenceFields);
  };

  /**
   * Create a cached information about shallow size and reference fields for
   * a given class.
   */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "We need to access private
  // fields of measured objects.") private static ClassCache
  // createCacheEntry(final Class clazz)
  static std::shared_ptr<ClassCache>
  createCacheEntry(std::type_info const clazz);
};

} // namespace org::apache::lucene::util
