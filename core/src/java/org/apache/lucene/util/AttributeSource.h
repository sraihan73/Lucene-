#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"

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
 * An AttributeSource contains a deque of different {@link AttributeImpl}s,
 * and methods to add and get them. There can only be a single instance
 * of an attribute in the same AttributeSource instance. This is ensured
 * by passing in the actual type of the Attribute (Class&lt;Attribute&gt;) to
 * the {@link #addAttribute(Class)}, which then checks if an instance of
 * that type is already present. If yes, it returns the instance, otherwise
 * it creates a new instance and returns it.
 */
class AttributeSource : public std::enable_shared_from_this<AttributeSource>
{
  GET_CLASS_NAME(AttributeSource)

  /**
   * This class holds the state of an AttributeSource.
   * @see #captureState
   * @see #restoreState
   */
public:
  class State final : public std::enable_shared_from_this<State>,
                      public Cloneable
  {
    GET_CLASS_NAME(State)
  public:
    std::shared_ptr<AttributeImpl> attribute;
    std::shared_ptr<State> next;

    std::shared_ptr<State> clone() override;
  };

  // These two maps must always be in sync!!!
  // So they are private, final and read-only from the outside (read-only
  // iterators)
private:
  const std::unordered_map<std::type_info, std::shared_ptr<AttributeImpl>>
      attributes;
  const std::unordered_map<std::type_info, std::shared_ptr<AttributeImpl>>
      attributeImpls;
  std::deque<std::shared_ptr<State>> const currentState;

  const std::shared_ptr<AttributeFactory> factory;

  /**
   * An AttributeSource using the default attribute factory {@link
   * AttributeFactory#DEFAULT_ATTRIBUTE_FACTORY}.
   */
public:
  AttributeSource();

  /**
   * An AttributeSource that uses the same attributes as the supplied one.
   */
  AttributeSource(std::shared_ptr<AttributeSource> input);

  /**
   * An AttributeSource using the supplied {@link AttributeFactory} for creating
   * new {@link Attribute} instances.
   */
  AttributeSource(std::shared_ptr<AttributeFactory> factory);

  /**
   * returns the used AttributeFactory.
   */
  std::shared_ptr<AttributeFactory> getAttributeFactory();

  /** Returns a new iterator that iterates the attribute classes
   * in the same order they were added in.
   */
  std::shared_ptr<Iterator<std::type_info>> getAttributeClassesIterator();

  /** Returns a new iterator that iterates all unique Attribute implementations.
   * This iterator may contain less entries that {@link
   * #getAttributeClassesIterator}, if one instance implements more than one
   * Attribute interface.
   */
  std::shared_ptr<Iterator<std::shared_ptr<AttributeImpl>>>
  getAttributeImplsIterator();

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<AttributeImpl>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<AttributeSource> outerInstance;

    std::shared_ptr<org::apache::lucene::util::AttributeSource::State>
        initState;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<AttributeSource> outerInstance,
        std::shared_ptr<org::apache::lucene::util::AttributeSource::State>
            initState);

  private:
    std::shared_ptr<State> state;

  public:
    void remove();

    std::shared_ptr<AttributeImpl> next();

    bool hasNext();
  };

  /** a cache that stores all interfaces for known implementation classes for
   * performance (slow reflection) */
private:
  static const std::shared_ptr<ClassValue<std::deque<std::type_info>>>
      implInterfaces;

private:
  class ClassValueAnonymousInnerClass
      : public ClassValue<std::deque<std::type_info>>
  {
    GET_CLASS_NAME(ClassValueAnonymousInnerClass)
  public:
    ClassValueAnonymousInnerClass();

  protected:
    std::deque<std::type_info> computeValue(std::type_info clazz) override;

  protected:
    std::shared_ptr<ClassValueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ClassValueAnonymousInnerClass>(
          ClassValue<Class[]>::shared_from_this());
    }
  };

public:
  static std::deque<std::type_info>
  getAttributeInterfaces(std::type_info const clazz);

  /** <b>Expert:</b> Adds a custom AttributeImpl instance with one or more
Attribute interfaces.
   * <p><b>NOTE:</b> It is not guaranteed, that <code>att</code> is added to
   * the <code>AttributeSource</code>, because the provided attributes may
already exist.
   * You should always retrieve the wanted attributes using {@link
#getAttribute} after adding
   * with this method and cast to your class.
   * The recommended way to use custom implementations is using an {@link
AttributeFactory}. GET_CLASS_NAME(.)
   * </p>
   */
  void addAttributeImpl(std::shared_ptr<AttributeImpl> att);

  /**
   * The caller must pass in a Class&lt;? extends Attribute&gt; value.
   * This method first checks if an instance of that class is
   * already in this AttributeSource and returns it. Otherwise a
   * new instance is created, added to this AttributeSource and returned.
   */
  template <typename T>
  T addAttribute(std::type_info<T> &attClass);

  /** Returns true, iff this AttributeSource has any attributes */
  bool hasAttributes();

  /**
   * The caller must pass in a Class&lt;? extends Attribute&gt; value.
   * Returns true, iff this AttributeSource contains the passed-in Attribute.
   */
  bool hasAttribute(std::type_info attClass);

  /**
   * Returns the instance of the passed in Attribute contained in this
   * AttributeSource <p> The caller must pass in a Class&lt;? extends
   * Attribute&gt; value.
   *
   * @return instance of the passed in Attribute, or {@code null} if this
   * AttributeSource does not contain the Attribute. It is recommended to always
   * use
   *         {@link #addAttribute} even in consumers  of TokenStreams, because
   * you cannot know if a specific TokenStream really uses a specific Attribute.
   *         {@link #addAttribute} will automatically make the attribute
   * available. If you want to only use the attribute, if it is available (to
   * optimize consuming), use {@link #hasAttribute}.
   */
  template <typename T>
  T getAttribute(std::type_info<T> &attClass);

private:
  std::shared_ptr<State> getCurrentState();

  /**
   * Resets all Attributes in this AttributeSource by calling
   * {@link AttributeImpl#clear()} on each Attribute implementation.
   */
public:
  void clearAttributes();

  /**
   * Resets all Attributes in this AttributeSource by calling
   * {@link AttributeImpl#end()} on each Attribute implementation.
   */
  void endAttributes();

  /**
   * Removes all attributes and their implementations from this AttributeSource.
   */
  void removeAllAttributes();

  /**
   * Captures the state of all Attributes. The return value can be passed to
   * {@link #restoreState} to restore the state of this or another
   * AttributeSource.
   */
  std::shared_ptr<State> captureState();

  /**
   * Restores this state by copying the values of all attribute implementations
   * that this state contains into the attributes implementations of the
   * targetStream. The targetStream must contain a corresponding instance for
   * each argument contained in this state (e.g. it is not possible to restore
   * the state of an AttributeSource containing a TermAttribute into a
   * AttributeSource using a Token instance as implementation). <p> Note that
   * this method does not affect attributes of the targetStream that are not
   * contained in this state. In other words, if for example the targetStream
   * contains an OffsetAttribute, but this state doesn't, then the value of the
   * OffsetAttribute remains unchanged. It might be desirable to reset its value
   * to the default, in which case the caller should first call {@link
   * TokenStream#clearAttributes()} on the targetStream.
   */
  void restoreState(std::shared_ptr<State> state);

  virtual int hashCode();

  bool equals(std::any obj) override;

  /**
   * This method returns the current attribute values as a string in the
   * following format by calling the {@link #reflectWith(AttributeReflector)}
   * method:
   *
   * <ul>
   * <li><em>iff {@code prependAttClass=true}:</em> {@code
   * "AttributeClass#key=value,AttributeClass#key=value"} <li><em>iff {@code
   * prependAttClass=false}:</em> {@code "key=value,key=value"}
   * </ul>
   *
   * @see #reflectWith(AttributeReflector)
   */
  std::wstring reflectAsString(bool const prependAttClass);

  /**
   * This method is for introspection of attributes, it should simply
   * add the key/values this AttributeSource holds to the given {@link
   * AttributeReflector}.
   *
   * <p>This method iterates over all Attribute implementations and calls the
   * corresponding {@link AttributeImpl#reflectWith} method.</p>
   *
   * @see AttributeImpl#reflectWith
   */
  void reflectWith(AttributeReflector reflector);

  /**
   * Performs a clone of all {@link AttributeImpl} instances returned in a new
   * {@code AttributeSource} instance. This method can be used to e.g. create
   * another TokenStream with exactly the same attributes (using {@link
   * #AttributeSource(AttributeSource)}). You can also use it as a
   * (non-performant) replacement for {@link #captureState}, if you need to look
   * into / modify the captured state.
   */
  std::shared_ptr<AttributeSource> cloneAttributes();

  /**
   * Copies the contents of this {@code AttributeSource} to the given target
   * {@code AttributeSource}. The given instance has to provide all {@link
   * Attribute}s this instance contains. The actual attribute implementations
   * must be identical in both {@code AttributeSource} instances; ideally both
   * AttributeSource instances should use the same {@link AttributeFactory}. You
   * can use this method as a replacement for {@link #restoreState}, if you use
   * {@link #cloneAttributes} instead of {@link #captureState}.
   */
  void copyTo(std::shared_ptr<AttributeSource> target);

  /**
   * Returns a string consisting of the class's simple name, the hex
   * representation of the identity hash code, and the current reflection of all
   * attributes.
   * @see #reflectAsString(bool)
   */
  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/util/
