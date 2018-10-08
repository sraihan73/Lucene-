#pragma once
#include "CharArrayMap.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
template <typename V>
class CharArrayMap;
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
namespace org::apache::lucene::analysis
{

/**
 * A simple class that stores Strings as char[]'s in a
 * hash table.  Note that this is not a general purpose
 * class.  For example, it cannot remove items from the
 * set, nor does it resize its hash table to be smaller,
 * etc.  It is designed to be quick to test if a char[]
 * is in the set without the necessity of converting it
 * to a std::wstring first.
 *
 * <P>
 * <em>Please note:</em> This class implements {@link java.util.Set Set} but
 * does not behave like it should in all cases. The generic type is
 * {@code Set<Object>}, because you can add any object to it,
 * that has a string representation. The add methods will use
 * {@link Object#toString} and store the result using a {@code char[]}
 * buffer. The same behavior have the {@code contains()} methods.
 * The {@link #iterator()} returns an {@code Iterator<char[]>}.
 */
class CharArraySet : public AbstractSet<std::any>
{
  GET_CLASS_NAME(CharArraySet)

  /** An empty {@code CharArraySet}. */
public:
  static const std::shared_ptr<CharArraySet> EMPTY_SET;

private:
  static const std::any PLACEHOLDER;

  const std::shared_ptr<CharArrayMap<std::any>> map_obj;

  /**
   * Create set with enough capacity to hold startSize terms
   *
   * @param startSize
   *          the initial capacity
   * @param ignoreCase
   *          <code>false</code> if and only if the set should be case sensitive
   *          otherwise <code>true</code>.
   */
public:
  CharArraySet(int startSize, bool ignoreCase);

  /**
   * Creates a set from a std::deque of objects.
   *
   * @param c
   *          a collection whose elements to be placed into the set
   * @param ignoreCase
   *          <code>false</code> if and only if the set should be case sensitive
   *          otherwise <code>true</code>.
   */
  template <typename T1>
  CharArraySet(std::shared_ptr<std::deque<T1>> c, bool ignoreCase);

  /** Create set from the specified map_obj (internal only), used also by {@link
   * CharArrayMap#keySet()} */
  CharArraySet(std::shared_ptr<CharArrayMap<std::any>> map_obj);

  /** Clears all entries in this set. This method is supported for reusing, but
   * not {@link Set#remove}. */
  void clear() override;

  /** true if the <code>len</code> chars of <code>text</code> starting at
   * <code>off</code> are in the set */
  virtual bool contains(std::deque<wchar_t> &text, int off, int len);

  /** true if the <code>std::wstring</code> is in the set */
  virtual bool contains(std::shared_ptr<std::wstring> cs);

  bool contains(std::any o) override;

  bool add(std::any o) override;

  /** Add this std::wstring into the set */
  virtual bool add(std::shared_ptr<std::wstring> text);

  /** Add this std::wstring into the set */
  virtual bool add(const std::wstring &text);

  /** Add this char[] directly to the set.
   * If ignoreCase is true for this Set, the text array will be directly
   * modified. The user should never modify this text array after calling this
   * method.
   */
  virtual bool add(std::deque<wchar_t> &text);

  int size() override;

  /**
   * Returns an unmodifiable {@link CharArraySet}. This allows to provide
   * unmodifiable views of internal sets for "read-only" use.
   *
   * @param set
   *          a set for which the unmodifiable set is returned.
   * @return an new unmodifiable {@link CharArraySet}.
   * @throws NullPointerException
   *           if the given set is <code>null</code>.
   */
  static std::shared_ptr<CharArraySet>
  unmodifiableSet(std::shared_ptr<CharArraySet> set);

  /**
   * Returns a copy of the given set as a {@link CharArraySet}. If the given set
   * is a {@link CharArraySet} the ignoreCase property will be preserved.
   *
   * @param set
   *          a set to copy
   * @return a copy of the given set as a {@link CharArraySet}. If the given set
   *         is a {@link CharArraySet} the ignoreCase property as well as the
   *         matchVersion will be of the given set will be preserved.
   */
  template <typename T1>
  static std::shared_ptr<CharArraySet> copy(std::shared_ptr<Set<T1>> set);

  /**
   * Returns an {@link Iterator} for {@code char[]} instances in this set.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public
  // java.util.Iterator<Object> iterator()
  std::shared_ptr<Iterator<std::any>> iterator() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CharArraySet> shared_from_this()
  {
    return std::static_pointer_cast<CharArraySet>(
        java.util.AbstractSet<Object>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
