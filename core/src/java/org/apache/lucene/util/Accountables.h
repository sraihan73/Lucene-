#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Accountable;
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
namespace org::apache::lucene::util
{

/**
 * Helper methods for constructing nested resource descriptions
 * and debugging RAM usage.
 * <p>
 * {@code toString(Accountable}} can be used to quickly debug the nested
 * structure of any Accountable.
 * <p>
 * The {@code namedAccountable} and {@code namedAccountables} methods return
 * type-safe, point-in-time snapshots of the provided resources.
 */
class Accountables : public std::enable_shared_from_this<Accountables>
{
  GET_CLASS_NAME(Accountables)
private:
  Accountables();

  /**
   * Returns a std::wstring description of an Accountable and any nested resources.
   * This is intended for development and debugging.
   */
public:
  static std::wstring toString(std::shared_ptr<Accountable> a);

private:
  static std::shared_ptr<StringBuilder>
  toString(std::shared_ptr<StringBuilder> dest, std::shared_ptr<Accountable> a,
           int depth);

  /**
   * Augments an existing accountable with the provided description.
   * <p>
   * The resource description is constructed in this format:
   * {@code description [toString()]}
   * <p>
   * This is a point-in-time type safe view: consumers
   * will not be able to cast or manipulate the resource in any way.
   */
public:
  static std::shared_ptr<Accountable>
  namedAccountable(const std::wstring &description,
                   std::shared_ptr<Accountable> in_);

  /**
   * Returns an accountable with the provided description and bytes.
   */
  static std::shared_ptr<Accountable>
  namedAccountable(const std::wstring &description, int64_t bytes);

  /**
   * Converts a map_obj of resources to a collection.
   * <p>
   * The resource descriptions are constructed in this format:
   * {@code prefix 'key' [toString()]}
   * <p>
   * This is a point-in-time type safe view: consumers
   * will not be able to cast or manipulate the resources in any way.
   */
  template <typename T1, typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static java.util.std::deque<Accountable>
  // namedAccountables(std::wstring prefix, java.util.Map<?,? extends Accountable> in)
  static std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  namedAccountables(const std::wstring &prefix, std::unordered_map<T1> in_);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<Accountable>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<Accountable> o1,
                std::shared_ptr<Accountable> o2);
  };

  /**
   * Returns an accountable with the provided description, children and bytes.
   * <p>
   * The resource descriptions are constructed in this format:
   * {@code description [toString()]}
   * <p>
   * This is a point-in-time type safe view: consumers
   * will not be able to cast or manipulate the resources in any way, provided
   * that the passed in children Accountables (and all their descendants) were
   * created with one of the namedAccountable functions.
   */
public:
  static std::shared_ptr<Accountable> namedAccountable(
      const std::wstring &description,
      std::shared_ptr<std::deque<std::shared_ptr<Accountable>>> children,
      int64_t const bytes);

private:
  class AccountableAnonymousInnerClass
      : public std::enable_shared_from_this<AccountableAnonymousInnerClass>,
        public Accountable
  {
    GET_CLASS_NAME(AccountableAnonymousInnerClass)
  private:
    std::wstring description;
    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>> children;
    int64_t bytes = 0;

  public:
    AccountableAnonymousInnerClass(
        const std::wstring &description,
        std::shared_ptr<std::deque<std::shared_ptr<Accountable>>> children,
        int64_t bytes);

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources();

    std::wstring toString();
  };
};

} // namespace org::apache::lucene::util
