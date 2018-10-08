#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeImpl;
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
 * An AttributeFactory creates instances of {@link AttributeImpl}s.
 */
class AttributeFactory : public std::enable_shared_from_this<AttributeFactory>
{
  GET_CLASS_NAME(AttributeFactory)

  /**
   * Returns an {@link AttributeImpl} for the supplied {@link Attribute}
   * interface class.
   *
   * @throws UndeclaredThrowableException A wrapper runtime exception thrown if
   * the constructor of the attribute class throws a checked exception. Note
   * that attributes should not throw or declare checked exceptions; this may be
   * verified and fail early in the future.
   */
public:
  virtual std::shared_ptr<AttributeImpl>
  createAttributeInstance(std::type_info attClass) = 0;

  /**
   * Returns a correctly typed {@link MethodHandle} for the no-arg ctor of the
   * given class.
   */
  static std::shared_ptr<MethodHandle>
  findAttributeImplCtor(std::type_info clazz);

private:
  static const std::shared_ptr<MethodHandles::Lookup> lookup;
  static const std::shared_ptr<MethodType> NO_ARG_CTOR;
  static const std::shared_ptr<MethodType> NO_ARG_RETURNING_ATTRIBUTEIMPL;

  /**
   * This is the default factory that creates {@link AttributeImpl}s using the
   * class name of the supplied {@link Attribute} interface class by appending
   * <code>Impl</code> to it.
   */
public:
  static const std::shared_ptr<AttributeFactory> DEFAULT_ATTRIBUTE_FACTORY;

private:
  class DefaultAttributeFactory;

  /** <b>Expert</b>: AttributeFactory returning an instance of the given {@code
   * clazz} for the attributes it implements. For all other attributes it calls
   * the given delegate factory as fallback. This class can be used to prefer a
   * specific {@code AttributeImpl} which combines multiple attributes over
   * separate classes.
   * @lucene.internal
   */
public:
  template <typename A>
  class StaticImplementationAttributeFactory;

  /** Returns an AttributeFactory returning an instance of the given {@code
   * clazz} for the attributes it implements. The given {@code clazz} must have
   * a public no-arg constructor. For all other attributes it calls the given
   * delegate factory as fallback. This method can be used to prefer a specific
   * {@code AttributeImpl} which combines multiple attributes over separate
   * classes. <p>Please save instances created by this method in a static final
   * field, because on each call, this does reflection for creating a {@link
   * MethodHandle}.
   */
public:
  template <typename A>
  static std::shared_ptr<AttributeFactory>
  getStaticImplementation(std::shared_ptr<AttributeFactory> delegate_,
                          std::type_info<A> &clazz);

private:
  class StaticImplementationAttributeFactoryAnonymousInnerClass
      : public StaticImplementationAttributeFactory<std::shared_ptr<A>>
  {
    GET_CLASS_NAME(StaticImplementationAttributeFactoryAnonymousInnerClass)
  private:
    std::shared_ptr<MethodHandle> constr;

  public:
    StaticImplementationAttributeFactoryAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::util::AttributeFactory> delegate_,
        std::type_info<std::shared_ptr<A>> &clazz,
        std::shared_ptr<MethodHandle> constr);

  protected:
    std::shared_ptr<A> createInstance() override;

  protected:
    std::shared_ptr<StaticImplementationAttributeFactoryAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          StaticImplementationAttributeFactoryAnonymousInnerClass>(
          StaticImplementationAttributeFactory<A>::shared_from_this());
    }
  };
};
} // namespace org::apache::lucene::util
class AttributeFactory::DefaultAttributeFactory final : public AttributeFactory
{
  GET_CLASS_NAME(AttributeFactory::DefaultAttributeFactory)
private:
  const std::shared_ptr<ClassValue<std::shared_ptr<MethodHandle>>>
      constructors = std::make_shared<ClassValueAnonymousInnerClass>();

private:
  class ClassValueAnonymousInnerClass
      : public ClassValue<std::shared_ptr<MethodHandle>>
  {
    GET_CLASS_NAME(ClassValueAnonymousInnerClass)
  public:
    ClassValueAnonymousInnerClass();

  protected:
    std::shared_ptr<MethodHandle>
    computeValue(std::type_info attClass) override;

  protected:
    std::shared_ptr<ClassValueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ClassValueAnonymousInnerClass>(
          ClassValue<MethodHandle>::shared_from_this());
    }
  };

public:
  DefaultAttributeFactory();

  std::shared_ptr<AttributeImpl>
  createAttributeInstance(std::type_info attClass) override;

private:
  std::type_info findImplClass(std::type_info attClass);

protected:
  std::shared_ptr<DefaultAttributeFactory> shared_from_this()
  {
    return std::static_pointer_cast<DefaultAttributeFactory>(
        AttributeFactory::shared_from_this());
  }
};
class AttributeFactory::StaticImplementationAttributeFactory
    : public AttributeFactory
{
  GET_CLASS_NAME(AttributeFactory::StaticImplementationAttributeFactory)
  static_assert(std::is_base_of<AttributeImpl, A>::value,
                L"A must inherit from AttributeImpl");

private:
  const std::shared_ptr<AttributeFactory> delegate_;
  const std::type_info<A> clazz;

  /** <b>Expert</b>: Creates an AttributeFactory returning {@code clazz} as
   * instance for the attributes it implements and for all other attributes
   * calls the given delegate factory. */
public:
  StaticImplementationAttributeFactory(
      std::shared_ptr<AttributeFactory> delegate_, std::type_info<A> &clazz)
      : delegate_(delegate_), clazz(clazz)
  {
  }

  std::shared_ptr<AttributeImpl>
  createAttributeInstance(std::type_info attClass) override final
  {
    return attClass.isAssignableFrom(clazz)
               ? createInstance()
               : delegate_->createAttributeInstance(attClass);
  }

  /** Creates an instance of {@code A}. */
protected:
  virtual A createInstance() = 0;

public:
  bool equals(std::any other) override
  {
    if (shared_from_this() == other) {
      return true;
    }
    if (other == nullptr || other.type() != this->getClass()) {
      return false;
    }
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("rawtypes") final
    // StaticImplementationAttributeFactory af =
    // (StaticImplementationAttributeFactory) other;
    std::shared_ptr<StaticImplementationAttributeFactory> *const af =
        std::any_cast<std::shared_ptr<StaticImplementationAttributeFactory>>(
            other);
    return this->delegate_->equals(af->delegate_) && this->clazz == af->clazz;
  }

  int hashCode() override
  {
    return 31 * delegate_->hashCode() + clazz.hash_code();
  }

protected:
  std::shared_ptr<StaticImplementationAttributeFactory> shared_from_this()
  {
    return std::static_pointer_cast<StaticImplementationAttributeFactory>(
        AttributeFactory::shared_from_this());
  }
};
