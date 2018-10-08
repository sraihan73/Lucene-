using namespace std;

#include "AttributeSource.h"

namespace org::apache::lucene::util
{
using TokenStream = org::apache::lucene::analysis::TokenStream;

shared_ptr<State> AttributeSource::State::clone()
{
  shared_ptr<State> clone = make_shared<State>();
  clone->attribute = attribute->clone();

  if (next != nullptr) {
    clone->next = next->clone();
  }

  return clone;
}

AttributeSource::AttributeSource()
    : AttributeSource(AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY)
{
}

AttributeSource::AttributeSource(shared_ptr<AttributeSource> input)
    : attributes(input->attributes), attributeImpls(input->attributeImpls),
      currentState(input->currentState), factory(input->factory)
{
  Objects::requireNonNull(input, L"input AttributeSource must not be null");
}

AttributeSource::AttributeSource(shared_ptr<AttributeFactory> factory)
    : attributes(make_shared<LinkedHashMap<>>()),
      attributeImpls(make_shared<LinkedHashMap<>>()),
      currentState(std::deque<std::shared_ptr<State>>(1)),
      factory(Objects::requireNonNull(factory,
                                      L"AttributeFactory must not be null"))
{
}

shared_ptr<AttributeFactory> AttributeSource::getAttributeFactory()
{
  return this->factory;
}

shared_ptr<Iterator<type_info>> AttributeSource::getAttributeClassesIterator()
{
  return Collections::unmodifiableSet(attributes.keySet()).begin();
}

shared_ptr<Iterator<std::shared_ptr<AttributeImpl>>>
AttributeSource::getAttributeImplsIterator()
{
  shared_ptr<State> *const initState = getCurrentState();
  if (initState != nullptr) {
    return make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                    initState);
  } else {
    return Collections::emptySet<std::shared_ptr<AttributeImpl>>().begin();
  }
}

AttributeSource::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<AttributeSource> outerInstance,
    shared_ptr<org::apache::lucene::util::AttributeSource::State> initState)
{
  this->outerInstance = outerInstance;
  this->initState = initState;
  state = initState;
}

void AttributeSource::IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<AttributeImpl> AttributeSource::IteratorAnonymousInnerClass::next()
{
  if (state == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }
  shared_ptr<AttributeImpl> *const att = state::attribute;
  state = state->next;
  return att;
}

bool AttributeSource::IteratorAnonymousInnerClass::hasNext()
{
  return state != nullptr;
}

const shared_ptr<ClassValue<std::deque<type_info>>>
    AttributeSource::implInterfaces =
        make_shared<ClassValueAnonymousInnerClass>();

AttributeSource::ClassValueAnonymousInnerClass::ClassValueAnonymousInnerClass()
{
}

std::deque<type_info>
AttributeSource::ClassValueAnonymousInnerClass::computeValue(type_info clazz)
{
  shared_ptr<Set<type_info>> *const intfSet =
      make_shared<LinkedHashSet<type_info>>();
  // find all interfaces that this attribute instance implements
  // and that extend the Attribute interface
  do {
    for (type_info curInterface : clazz.getInterfaces()) {
      if (curInterface != Attribute::typeid &&
          Attribute::typeid->isAssignableFrom(curInterface)) {
        intfSet->add(curInterface.asSubclass(Attribute::typeid));
      }
    }
    clazz = clazz.getSuperclass();
  } while (clazz != nullptr);
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) final Class[] a
  // = intfSet.toArray(new Class[intfSet.size()]);
  const std::deque<type_info> a =
      intfSet->toArray(std::deque<type_info>(intfSet->size()));
  return a;
}

std::deque<type_info>
AttributeSource::getAttributeInterfaces(type_info const clazz)
{
  return implInterfaces->get(clazz);
}

void AttributeSource::addAttributeImpl(shared_ptr<AttributeImpl> att)
{
  constexpr type_info clazz = att->getClass();
  if (attributeImpls.find(clazz) != attributeImpls.end()) {
    return;
  }

  // add all interfaces of this AttributeImpl to the maps
  for (auto curInterface : getAttributeInterfaces(clazz)) {
    // Attribute is a superclass of this interface
    if (attributes.find(curInterface) == attributes.end()) {
      // invalidate state to force recomputation in captureState()
      this->currentState[0].reset();
      attributes.emplace(curInterface, att);
      attributeImpls.emplace(clazz, att);
    }
  }
}

template <typename T>
T AttributeSource::addAttribute(type_info<T> &attClass)
{
  static_assert(is_base_of<Attribute, T>::value,
                L"T must inherit from Attribute");

  shared_ptr<AttributeImpl> attImpl = attributes[attClass];
  if (attImpl == nullptr) {
    if (!(attClass.isInterface() &&
          Attribute::typeid->isAssignableFrom(attClass))) {
      throw invalid_argument(L"addAttribute() only accepts an interface that "
                             L"extends Attribute, but " +
                             attClass.getName() +
                             L" does not fulfil this contract.");
    }
    addAttributeImpl(attImpl =
                         this->factory->createAttributeInstance(attClass));
  }
  return attClass.cast(attImpl);
}

bool AttributeSource::hasAttributes() { return !this->attributes.empty(); }

bool AttributeSource::hasAttribute(type_info attClass)
{
  return this->attributes.find(attClass) != this->attributes.end();
}

template <typename T>
T AttributeSource::getAttribute(type_info<T> &attClass)
{
  static_assert(is_base_of<Attribute, T>::value,
                L"T must inherit from Attribute");

  return attClass.cast(attributes[attClass]);
}

shared_ptr<State> AttributeSource::getCurrentState()
{
  shared_ptr<State> s = currentState[0];
  if (s != nullptr || !hasAttributes()) {
    return s;
  }
  shared_ptr<State> c = s = currentState[0] = make_shared<State>();
  const Iterator<std::shared_ptr<AttributeImpl>> it =
      attributeImpls.values().begin();
  c->attribute = it->next();
  while (it->hasNext()) {
    c->next = make_shared<State>();
    c = c->next;
    c->attribute = it->next();
    it++;
  }
  return s;
}

void AttributeSource::clearAttributes()
{
  for (shared_ptr<State> state = getCurrentState(); state != nullptr;
       state = state->next) {
    state->attribute->clear();
  }
}

void AttributeSource::endAttributes()
{
  for (shared_ptr<State> state = getCurrentState(); state != nullptr;
       state = state->next) {
    state->attribute->end();
  }
}

void AttributeSource::removeAllAttributes()
{
  attributes.clear();
  attributeImpls.clear();
}

shared_ptr<State> AttributeSource::captureState()
{
  shared_ptr<State> *const state = this->getCurrentState();
  return (state == nullptr) ? nullptr : state->clone();
}

void AttributeSource::restoreState(shared_ptr<State> state)
{
  if (state == nullptr) {
    return;
  }

  do {
    shared_ptr<AttributeImpl> targetImpl =
        attributeImpls[state->attribute->getClass()];
    if (targetImpl == nullptr) {
      throw invalid_argument(L"State contains AttributeImpl of type " +
                             state->attribute->getClassName() +
                             L" that is not in in this AttributeSource");
    }
    state->attribute->copyTo(targetImpl);
    state = state->next;
  } while (state != nullptr);
}

int AttributeSource::hashCode()
{
  int code = 0;
  for (shared_ptr<State> state = getCurrentState(); state != nullptr;
       state = state->next) {
    code = code * 31 + state->attribute->hashCode();
  }
  return code;
}

bool AttributeSource::equals(any obj)
{
  if (obj == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<AttributeSource>(obj) != nullptr) {
    shared_ptr<AttributeSource> other =
        any_cast<std::shared_ptr<AttributeSource>>(obj);

    if (hasAttributes()) {
      if (!other->hasAttributes()) {
        return false;
      }

      if (this->attributeImpls.size() != other->attributeImpls.size()) {
        return false;
      }

      // it is only equal if all attribute impls are the same in the same order
      shared_ptr<State> thisState = this->getCurrentState();
      shared_ptr<State> otherState = other->getCurrentState();
      while (thisState != nullptr && otherState != nullptr) {
        if (otherState->attribute->getClass() !=
                thisState->attribute->getClass() ||
            !otherState->attribute->equals(thisState->attribute)) {
          return false;
        }
        thisState = thisState->next;
        otherState = otherState->next;
      }
      return true;
    } else {
      return !other->hasAttributes();
    }
  } else {
    return false;
  }
}

wstring AttributeSource::reflectAsString(bool const prependAttClass)
{
  shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>();
  reflectWith([&](type_info attClass, wstring key, any value) {
    if (buffer->length() > 0) {
      buffer->append(L',');
    }
    if (prependAttClass) {
      buffer->append(attClass::getName())->append(L'#');
    }
    buffer->append(key)->append(L'=')->append((value == nullptr) ? L"null"
                                                                 : value);
  });
  return buffer->toString();
}

void AttributeSource::reflectWith(AttributeReflector reflector)
{
  for (shared_ptr<State> state = getCurrentState(); state != nullptr;
       state = state->next) {
    state->attribute->reflectWith(reflector);
  }
}

shared_ptr<AttributeSource> AttributeSource::cloneAttributes()
{
  shared_ptr<AttributeSource> *const clone =
      make_shared<AttributeSource>(this->factory);

  if (hasAttributes()) {
    // first clone the impls
    for (shared_ptr<State> state = getCurrentState(); state != nullptr;
         state = state->next) {
      clone->attributeImpls.emplace(state->attribute->getClass(),
                                    state->attribute->clone());
    }

    // now the interfaces
    for (auto entry : this->attributes) {
      clone->attributes.emplace(
          entry.first, clone->attributeImpls[entry.second::getClass()]);
    }
  }

  return clone;
}

void AttributeSource::copyTo(shared_ptr<AttributeSource> target)
{
  for (shared_ptr<State> state = getCurrentState(); state != nullptr;
       state = state->next) {
    shared_ptr<AttributeImpl> *const targetImpl =
        target->attributeImpls[state->attribute->getClass()];
    if (targetImpl == nullptr) {
      throw invalid_argument(
          L"This AttributeSource contains AttributeImpl of type " +
          state->attribute->getClassName() + L" that is not in the target");
    }
    state->attribute->copyTo(targetImpl);
  }
}

wstring AttributeSource::toString()
{
  return getClass().getSimpleName() + L'@' +
         Integer::toHexString(System::identityHashCode(shared_from_this())) +
         L" " + reflectAsString(false);
}
} // namespace org::apache::lucene::util