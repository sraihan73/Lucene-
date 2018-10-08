using namespace std;

#include "AttributeFactory.h"

namespace org::apache::lucene::util
{

shared_ptr<MethodHandle>
AttributeFactory::findAttributeImplCtor(type_info clazz)
{
  try {
    return lookup->findConstructor(clazz, NO_ARG_CTOR)
        .asType(NO_ARG_RETURNING_ATTRIBUTEIMPL);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchMethodException | IllegalAccessException e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Cannot lookup
    // accessible no-arg constructor for: " + clazz.getName(), e);
    throw invalid_argument(
        L"Cannot lookup accessible no-arg constructor for: " + clazz.getName());
  }
}

const shared_ptr<MethodHandles::Lookup> AttributeFactory::lookup =
    MethodHandles::publicLookup();
const shared_ptr<MethodType> AttributeFactory::NO_ARG_CTOR =
    MethodType::methodType(void ::typeid);
const shared_ptr<MethodType> AttributeFactory::NO_ARG_RETURNING_ATTRIBUTEIMPL =
    MethodType::methodType(AttributeImpl::typeid);
const shared_ptr<AttributeFactory> AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY =
    make_shared<DefaultAttributeFactory>();

AttributeFactory::DefaultAttributeFactory::ClassValueAnonymousInnerClass::
    ClassValueAnonymousInnerClass()
{
}

shared_ptr<MethodHandle> AttributeFactory::DefaultAttributeFactory::
    ClassValueAnonymousInnerClass::computeValue(type_info attClass)
{
  return findAttributeImplCtor(
      outerInstance->findImplClass(attClass.asSubclass(Attribute::typeid)));
}

AttributeFactory::DefaultAttributeFactory::DefaultAttributeFactory() {}

shared_ptr<AttributeImpl>
AttributeFactory::DefaultAttributeFactory::createAttributeInstance(
    type_info attClass)
{
  try {
    return std::static_pointer_cast<AttributeImpl>(
        constructors->get(attClass).invokeExact());
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (Error | runtime_error e) {
    throw e;
  } catch (const runtime_error &e) {
    throw make_shared<UndeclaredThrowableException>(e);
  }
}

type_info
AttributeFactory::DefaultAttributeFactory::findImplClass(type_info attClass)
{
  try {
    return type_info::forName(attClass.getName() + L"Impl", true,
                              attClass.getClassLoader())
        .asSubclass(AttributeImpl::typeid);
  } catch (const ClassNotFoundException &cnfe) {
    throw invalid_argument(L"Cannot find implementing class for: " +
                           attClass.getName());
  }
}

template <typename A>
shared_ptr<AttributeFactory> AttributeFactory::getStaticImplementation(
    shared_ptr<AttributeFactory> delegate_, type_info<A> &clazz)
{
  static_assert(is_base_of<AttributeImpl, A>::value,
                L"A must inherit from AttributeImpl");

  shared_ptr<MethodHandle> *const constr = findAttributeImplCtor(clazz);
  return make_shared<StaticImplementationAttributeFactoryAnonymousInnerClass>(
      delegate_, clazz, constr);
}

AttributeFactory::StaticImplementationAttributeFactoryAnonymousInnerClass::
    StaticImplementationAttributeFactoryAnonymousInnerClass(
        shared_ptr<org::apache::lucene::util::AttributeFactory> delegate_,
        type_info<std::shared_ptr<A>> &clazz, shared_ptr<MethodHandle> constr)
    : StaticImplementationAttributeFactory<A>(delegate_, clazz)
{
  this->constr = constr;
}

shared_ptr<A> AttributeFactory::
    StaticImplementationAttributeFactoryAnonymousInnerClass::createInstance()
{
  try {
    return std::static_pointer_cast<A>(constr->invokeExact());
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (Error | runtime_error e) {
    throw e;
  } catch (const runtime_error &e) {
    throw make_shared<UndeclaredThrowableException>(e);
  }
}
} // namespace org::apache::lucene::util