#pragma once
#include "CharArraySet.h"
#include "CharacterUtils.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cctype>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/EntrySet.h"

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
 * A simple class that stores key Strings as char[]'s in a
 * hash table. Note that this is not a general purpose
 * class.  For example, it cannot remove items from the
 * map_obj, nor does it resize its hash table to be smaller,
 * etc.  It is designed to be quick to retrieve items
 * by char[] keys without the necessity of converting
 * to a std::wstring first.
 */
template <typename V>
class CharArrayMap : public AbstractMap<std::any, V>
{
  GET_CLASS_NAME(CharArrayMap)
  // private only because missing generics
private:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files: C++ TODO: Java wildcard generics are
  // not converted to C++: ORIGINAL LINE: private static final CharArrayMap<?>
  // EMPTY_MAP = new EmptyCharArrayMap<>();
  static const std::shared_ptr < CharArrayMap <
      ? >> EMPTY_MAP = std::make_shared < EmptyCharArrayMap < ? >> ();

  static constexpr int INIT_SIZE = 8;
  bool ignoreCase = false;
  int count = 0;

public:
  std::deque<std::deque<wchar_t>>
      keys; // package private because used in CharArraySet's non Set-conform
            // CharArraySetIterator
  std::deque<V> values; // package private because used in CharArraySet's non
                         // Set-conform CharArraySetIterator

  /**
   * Create map_obj with enough capacity to hold startSize terms
   *
   * @param startSize
   *          the initial capacity
   * @param ignoreCase
   *          <code>false</code> if and only if the set should be case sensitive
   *          otherwise <code>true</code>.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public CharArrayMap(int
  // startSize, bool ignoreCase)
  CharArrayMap(int startSize, bool ignoreCase)
  {
    this->ignoreCase = ignoreCase;
    int size = INIT_SIZE;
    while (startSize + (startSize >> 2) > size) {
      size <<= 1;
    }
    keys = std::deque<std::deque<wchar_t>>(size);
    values = static_cast<std::deque<V>>(std::deque<std::any>(size));
  }

  /**
   * Creates a map_obj from the mappings in another map_obj.
   *
   * @param c
   *          a map_obj whose mappings to be copied
   * @param ignoreCase
   *          <code>false</code> if and only if the set should be case sensitive
   *          otherwise <code>true</code>.
   */
  template <typename T1, typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public CharArrayMap(java.util.Map<?,? extends V>
  // c, bool ignoreCase)
  CharArrayMap(std::unordered_map<T1> c, bool ignoreCase)
      : CharArrayMap(c.size(), ignoreCase)
  {
    putAll(c);
  }

  /** Create set from the supplied map_obj (used internally for readonly maps...) */
private:
  CharArrayMap(std::shared_ptr<CharArrayMap<V>> toCopy)
  {
    this->keys = toCopy->keys;
    this->values = toCopy->values;
    this->ignoreCase = toCopy->ignoreCase;
    this->count = toCopy->count;
  }

  /** Clears all entries in this map_obj. This method is supported for reusing, but
   * not {@link Map#remove}. */
public:
  void clear() override
  {
    count = 0;
    Arrays::fill(keys, nullptr);
    Arrays::fill(values, nullptr);
  }

  /** true if the <code>len</code> chars of <code>text</code> starting at
   * <code>off</code> are in the {@link #keySet()} */
  virtual bool containsKey(std::deque<wchar_t> &text, int off, int len)
  {
    return keys[getSlot(text, off, len)] != nullptr;
  }

  /** true if the <code>std::wstring</code> is in the {@link #keySet()} */
  virtual bool containsKey(std::shared_ptr<std::wstring> cs)
  {
    return keys[getSlot(cs)] != nullptr;
  }

  bool containsKey(std::any o) override
  {
    if (dynamic_cast<std::deque<wchar_t>>(o) != nullptr) {
      const std::deque<wchar_t> text = std::any_cast<std::deque<wchar_t>>(o);
      return containsKey(text, 0, text.size());
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return containsKey(o.toString());
  }

  /** returns the value of the mapping of <code>len</code> chars of
   * <code>text</code> starting at <code>off</code> */
  virtual V get(std::deque<wchar_t> &text, int off, int len)
  {
    return values[getSlot(text, off, len)];
  }

  /** returns the value of the mapping of the chars inside this {@code
   * std::wstring} */
  virtual V get(std::shared_ptr<std::wstring> cs)
  {
    return values[getSlot(cs)];
  }

  V get(std::any o) override
  {
    if (dynamic_cast<std::deque<wchar_t>>(o) != nullptr) {
      const std::deque<wchar_t> text = std::any_cast<std::deque<wchar_t>>(o);
      return get(text, 0, text.size());
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return get(o.toString());
  }

private:
  int getSlot(std::deque<wchar_t> &text, int off, int len)
  {
    int code = getHashCode(text, off, len);
    int pos = code & (keys.size() - 1);
    std::deque<wchar_t> text2 = keys[pos];
    if (text2.size() > 0 && !equals(text, off, len, text2)) {
      constexpr int inc = ((code >> 8) + code) | 1;
      do {
        code += inc;
        pos = code & (keys.size() - 1);
        text2 = keys[pos];
      } while (text2.size() > 0 && !equals(text, off, len, text2));
    }
    return pos;
  }

  /** Returns true if the std::wstring is in the set */
  int getSlot(std::shared_ptr<std::wstring> text)
  {
    int code = getHashCode(text);
    int pos = code & (keys.size() - 1);
    std::deque<wchar_t> text2 = keys[pos];
    if (text2.size() > 0 && !equals(text, text2)) {
      constexpr int inc = ((code >> 8) + code) | 1;
      do {
        code += inc;
        pos = code & (keys.size() - 1);
        text2 = keys[pos];
      } while (text2.size() > 0 && !equals(text, text2));
    }
    return pos;
  }

  /** Add the given mapping. */
public:
  virtual V put(std::shared_ptr<std::wstring> text, V value)
  {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return put(text->toString(), value); // could be more efficient
  }

  V put(std::any o, V value) override
  {
    if (dynamic_cast<std::deque<wchar_t>>(o) != nullptr) {
      return put(std::any_cast<std::deque<wchar_t>>(o), value);
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return put(o.toString(), value);
  }

  /** Add the given mapping. */
  virtual V put(const std::wstring &text, V value)
  {
    return put(text.toCharArray(), value);
  }

  /** Add the given mapping.
   * If ignoreCase is true for this Set, the text array will be directly
   * modified. The user should never modify this text array after calling this
   * method.
   */
  virtual V put(std::deque<wchar_t> &text, V value)
  {
    if (ignoreCase) {
      CharacterUtils::toLowerCase(text, 0, text.size());
    }
    int slot = getSlot(text, 0, text.size());
    if (keys[slot] != nullptr) {
      constexpr V oldValue = values[slot];
      values[slot] = value;
      return oldValue;
    }
    keys[slot] = text;
    values[slot] = value;
    count++;

    if (count + (count >> 2) > keys.size()) {
      rehash();
    }

    return nullptr;
  }

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") private void rehash()
  void rehash()
  {
    assert(keys.size() == values.size());
    constexpr int newSize = 2 * keys.size();
    const std::deque<std::deque<wchar_t>> oldkeys = keys;
    const std::deque<V> oldvalues = values;
    keys = std::deque<std::deque<wchar_t>>(newSize);
    values = static_cast<std::deque<V>>(std::deque<std::any>(newSize));

    for (int i = 0; i < oldkeys.size(); i++) {
      std::deque<wchar_t> text = oldkeys[i];
      if (text.size() > 0) {
        // todo: could be faster... no need to compare strings on collision
        constexpr int slot = getSlot(text, 0, text.size());
        keys[slot] = text;
        values[slot] = oldvalues[i];
      }
    }
  }

  bool equals(std::deque<wchar_t> &text1, int off, int len,
              std::deque<wchar_t> &text2)
  {
    if (len != text2.size()) {
      return false;
    }
    constexpr int limit = off + len;
    if (ignoreCase) {
      for (int i = 0; i < len;) {
        constexpr int codePointAt =
            Character::codePointAt(text1, off + i, limit);
        if (std::towlower(codePointAt) !=
            Character::codePointAt(text2, i, text2.size())) {
          return false;
        }
        i += Character::charCount(codePointAt);
      }
    } else {
      for (int i = 0; i < len; i++) {
        if (text1[off + i] != text2[i]) {
          return false;
        }
      }
    }
    return true;
  }

  bool equals(std::shared_ptr<std::wstring> text1, std::deque<wchar_t> &text2)
  {
    int len = text1->length();
    if (len != text2.size()) {
      return false;
    }
    if (ignoreCase) {
      for (int i = 0; i < len;) {
        constexpr int codePointAt = Character::codePointAt(text1, i);
        if (std::towlower(codePointAt) !=
            Character::codePointAt(text2, i, text2.size())) {
          return false;
        }
        i += Character::charCount(codePointAt);
      }
    } else {
      for (int i = 0; i < len; i++) {
        if (text1->charAt(i) != text2[i]) {
          return false;
        }
      }
    }
    return true;
  }

  int getHashCode(std::deque<wchar_t> &text, int offset, int len)
  {
    if (text.empty()) {
      throw std::make_shared<NullPointerException>();
    }
    int code = 0;
    constexpr int stop = offset + len;
    if (ignoreCase) {
      for (int i = offset; i < stop;) {
        constexpr int codePointAt = Character::codePointAt(text, i, stop);
        code = code * 31 + std::towlower(codePointAt);
        i += Character::charCount(codePointAt);
      }
    } else {
      for (int i = offset; i < stop; i++) {
        code = code * 31 + text[i];
      }
    }
    return code;
  }

  int getHashCode(std::shared_ptr<std::wstring> text)
  {
    if (text == nullptr) {
      throw std::make_shared<NullPointerException>();
    }
    int code = 0;
    int len = text->length();
    if (ignoreCase) {
      for (int i = 0; i < len;) {
        int codePointAt = Character::codePointAt(text, i);
        code = code * 31 + std::towlower(codePointAt);
        i += Character::charCount(codePointAt);
      }
    } else {
      for (int i = 0; i < len; i++) {
        code = code * 31 + text->charAt(i);
      }
    }
    return code;
  }

public:
  V remove(std::any key) override
  {
    throw std::make_shared<UnsupportedOperationException>();
  }

  int size() override { return count; }

  std::wstring toString() override
  {
    std::shared_ptr<StringBuilder> *const sb =
        std::make_shared<StringBuilder>(L"{");
    for (auto entry : entrySet()) {
      if (sb->length() > 1) {
        sb->append(L", ");
      }
      sb->append(entry);
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return sb->append(L'}')->toString();
  }

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<EntrySet> entrySet_ = nullptr;
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<CharArraySet> keySet_ = nullptr;

public:
  virtual std::shared_ptr<EntrySet> createEntrySet()
  {
    return std::make_shared<EntrySet>(shared_from_this(), true);
  }

  std::shared_ptr<EntrySet> entrySet() override
  {
    if (entrySet_ == nullptr) {
      entrySet_ = createEntrySet();
    }
    return entrySet_;
  }

  // helper for CharArraySet to not produce endless recursion
  std::shared_ptr<Set<std::any>> originalKeySet()
  {
    return AbstractMap<Object, V>::keySet();
  }

  /** Returns an {@link CharArraySet} view on the map_obj's keys.
   * The set will use the same {@code matchVersion} as this map_obj. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings({"unchecked","rawtypes"}) public
  // final CharArraySet keySet()
  std::shared_ptr<CharArraySet> keySet() override
  {
    if (keySet_ == nullptr) {
      // prevent adding of entries
      keySet_ = std::make_shared<CharArraySetAnonymousInnerClass>(
          shared_from_this(),
          std::static_pointer_cast<CharArrayMap>(shared_from_this()));
    }
    return keySet_;
  }

private:
  class CharArraySetAnonymousInnerClass : public CharArraySet
  {
    GET_CLASS_NAME(CharArraySetAnonymousInnerClass)
  private:
    std::shared_ptr<CharArrayMap<std::shared_ptr<V>>> outerInstance;

  public:
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    CharArraySetAnonymousInnerClass(
        std::shared_ptr<CharArrayMap<std::shared_ptr<V>>> outerInstance,
        CharArrayMap(CharArrayMap) shared_from_this());

    bool add(std::any o) override;
    bool add(std::shared_ptr<std::wstring> text) override;
    bool add(const std::wstring &text) override;
    bool add(std::deque<wchar_t> &text) override;

  protected:
    std::shared_ptr<CharArraySetAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CharArraySetAnonymousInnerClass>(
          CharArraySet::shared_from_this());
    }
  };

  /** public iterator class so efficient methods are exposed to users */
public:
  class EntryIterator
      : public std::enable_shared_from_this<EntryIterator>,
        public Iterator<std::unordered_map::Entry<std::any, std::shared_ptr<V>>>
  {
    GET_CLASS_NAME(EntryIterator)
  private:
    std::shared_ptr<CharArrayMap<std::shared_ptr<V>>> outerInstance;

    int pos = -1;
    int lastPos = 0;
    const bool allowModify;

    EntryIterator(std::shared_ptr<CharArrayMap<V>> outerInstance,
                  bool allowModify);

    void goNext();

  public:
    bool hasNext() override;

    /** gets the next key... do not modify the returned char[] */
    virtual std::deque<wchar_t> nextKey();

    /** gets the next key as a newly created std::wstring object */
    virtual std::wstring nextKeyString();

    /** returns the value associated with the last key returned */
    virtual std::shared_ptr<V> currentValue();

    /** sets the value associated with the last key returned */
    virtual std::shared_ptr<V> setValue(std::shared_ptr<V> value);

    /** use nextCharArray() + currentValue() for better efficiency. */
    std::unordered_map::Entry<std::any, std::shared_ptr<V>> next() override;

    void remove() override;
  };

private:
  class MapEntry final
      : public std::enable_shared_from_this<MapEntry>,
        public std::unordered_map::Entry<std::any, std::shared_ptr<V>>
  {
    GET_CLASS_NAME(MapEntry)
  private:
    std::shared_ptr<CharArrayMap<std::shared_ptr<V>>> outerInstance;

    const int pos;
    const bool allowModify;

    MapEntry(std::shared_ptr<CharArrayMap<V>> outerInstance, int pos,
             bool allowModify);

  public:
    std::any getKey() override;

    std::shared_ptr<V> getValue() override;

    std::shared_ptr<V> setValue(std::shared_ptr<V> value) override;

    virtual std::wstring toString();
  };

  /** public EntrySet class so efficient methods are exposed to users */
public:
  class EntrySet final
      : public AbstractSet<
            std::unordered_map::Entry<std::any, std::shared_ptr<V>>>
  {
    GET_CLASS_NAME(EntrySet)
  private:
    std::shared_ptr<CharArrayMap<std::shared_ptr<V>>> outerInstance;

    const bool allowModify;

    EntrySet(std::shared_ptr<CharArrayMap<V>> outerInstance, bool allowModify);

  public:
    std::shared_ptr<EntryIterator> iterator() override;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public bool
    // contains(Object o)
    bool contains(std::any o) override;

    bool remove(std::any o) override;

    int size() override;

    void clear() override;

  protected:
    std::shared_ptr<EntrySet> shared_from_this()
    {
      return std::static_pointer_cast<EntrySet>(
          java.util
              .AbstractSet<java.util.Map.Entry<Object, V>>::shared_from_this());
    }
  };

  /**
   * Returns an unmodifiable {@link CharArrayMap}. This allows to provide
   * unmodifiable views of internal map_obj for "read-only" use.
   *
   * @param map_obj
   *          a map_obj for which the unmodifiable map_obj is returned.
   * @return an new unmodifiable {@link CharArrayMap}.
   * @throws NullPointerException
   *           if the given map_obj is <code>null</code>.
   */
public:
  template <typename V>
  static std::shared_ptr<CharArrayMap<V>>
  unmodifiableMap(std::shared_ptr<CharArrayMap<V>> map_obj)
  {
    if (map_obj == nullptr) {
      throw std::make_shared<NullPointerException>(L"Given map_obj is null");
    }
    if (map_obj == emptyMap() || map_obj->isEmpty()) {
      return emptyMap();
    }
    if (std::dynamic_pointer_cast<UnmodifiableCharArrayMap>(map_obj) != nullptr) {
      return map_obj;
    }
    return std::make_shared<UnmodifiableCharArrayMap<>>(map_obj);
  }

  /**
   * Returns a copy of the given map_obj as a {@link CharArrayMap}. If the given map_obj
   * is a {@link CharArrayMap} the ignoreCase property will be preserved.
   *
   * @param map_obj
   *          a map_obj to copy
   * @return a copy of the given map_obj as a {@link CharArrayMap}. If the given map_obj
   *         is a {@link CharArrayMap} the ignoreCase property as well as the
   *         matchVersion will be of the given map_obj will be preserved.
   */
  template <typename V, typename T1>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public static <V>
  // CharArrayMap<V> copy(final java.util.Map<?,? extends V> map_obj) C++ TODO: There
  // is no native C++ template equivalent to this generic constraint:
  static std::shared_ptr<CharArrayMap<V>> copy(std::unordered_map<T1> const map_obj)
  {
    if (map_obj == EMPTY_MAP) {
      return emptyMap();
    }
    if (std::dynamic_pointer_cast<CharArrayMap>(map_obj) != nullptr) {
      std::shared_ptr<CharArrayMap<V>> m =
          std::static_pointer_cast<CharArrayMap<V>>(map_obj);
      // use fast path instead of iterating all values
      // this is even on very small sets ~10 times faster than iterating
      const std::deque<std::deque<wchar_t>> keys =
          std::deque<std::deque<wchar_t>>(m->keys.size());
      System::arraycopy(m->keys, 0, keys, 0, keys.size());
      const std::deque<V> values =
          static_cast<std::deque<V>>(std::deque<std::any>(m->values.size()));
      System::arraycopy(m->values, 0, values, 0, values.size());
      m = std::make_shared<CharArrayMap<V>>(m);
      m->keys = keys;
      m->values = values;
      return m;
    }
    return std::make_shared<CharArrayMap<V>>(map_obj, false);
  }

  /** Returns an empty, unmodifiable map_obj. */
  template <typename V>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public static <V>
  // CharArrayMap<V> emptyMap()
  static std::shared_ptr<CharArrayMap<V>> emptyMap()
  {
    return std::static_pointer_cast<CharArrayMap<V>>(EMPTY_MAP);
  }

  // package private CharArraySet instanceof check in CharArraySet
public:
  template <typename V>
  class UnmodifiableCharArrayMap : public CharArrayMap<V>
  {
    GET_CLASS_NAME(UnmodifiableCharArrayMap)

  public:
    UnmodifiableCharArrayMap(std::shared_ptr<CharArrayMap<V>> map_obj)
        : CharArrayMap<V>(map_obj)
    {
    }

    void clear() override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    V put(std::any o, V val) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    V put(std::deque<wchar_t> &text, V val) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    V put(std::shared_ptr<std::wstring> text, V val) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    V put(const std::wstring &text, V val) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    V remove(std::any key) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    std::shared_ptr<EntrySet> createEntrySet() override
    {
      return std::make_shared<EntrySet>(shared_from_this(), false);
    }

  protected:
    std::shared_ptr<UnmodifiableCharArrayMap> shared_from_this()
    {
      return std::static_pointer_cast<UnmodifiableCharArrayMap>(
          CharArrayMap<V>::shared_from_this());
    }
  };

  /**
   * Empty {@link
   * org.apache.lucene.analysis.CharArrayMap.UnmodifiableCharArrayMap} optimized
   * for speed. Contains checks will always return <code>false</code> or throw
   * NPE if necessary.
   */
private:
  template <typename V>
  class EmptyCharArrayMap final : public UnmodifiableCharArrayMap<V>
  {
    GET_CLASS_NAME(EmptyCharArrayMap)
  public:
    EmptyCharArrayMap()
        : UnmodifiableCharArrayMap<V>(new CharArrayMap<V>(0, false))
    {
    }

    bool containsKey(std::deque<wchar_t> &text, int off, int len) override
    {
      if (text.empty()) {
        throw std::make_shared<NullPointerException>();
      }
      return false;
    }

    bool containsKey(std::shared_ptr<std::wstring> cs) override
    {
      if (cs == nullptr) {
        throw std::make_shared<NullPointerException>();
      }
      return false;
    }

    bool containsKey(std::any o) override
    {
      if (o == nullptr) {
        throw std::make_shared<NullPointerException>();
      }
      return false;
    }

    V get(std::deque<wchar_t> &text, int off, int len) override
    {
      if (text.empty()) {
        throw std::make_shared<NullPointerException>();
      }
      return nullptr;
    }

    V get(std::shared_ptr<std::wstring> cs) override
    {
      if (cs == nullptr) {
        throw std::make_shared<NullPointerException>();
      }
      return nullptr;
    }

    V get(std::any o) override
    {
      if (o == nullptr) {
        throw std::make_shared<NullPointerException>();
      }
      return nullptr;
    }

  protected:
    std::shared_ptr<EmptyCharArrayMap> shared_from_this()
    {
      return std::static_pointer_cast<EmptyCharArrayMap>(
          UnmodifiableCharArrayMap<V>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CharArrayMap> shared_from_this()
  {
    return std::static_pointer_cast<CharArrayMap>(
        java.util.AbstractMap<Object, V>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
