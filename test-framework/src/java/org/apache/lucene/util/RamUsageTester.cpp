using namespace std;

#include "RamUsageTester.h"

namespace org::apache::lucene::util
{

int64_t RamUsageTester::Accumulator::accumulateObject(
    any o, int64_t shallowSize,
    unordered_map<std::shared_ptr<Field>, any> &fieldValues,
    shared_ptr<deque<any>> queue)
{
  queue->addAll(fieldValues.values());
  return shallowSize;
}

int64_t
RamUsageTester::Accumulator::accumulateArray(any array_, int64_t shallowSize,
                                             deque<any> &values,
                                             shared_ptr<deque<any>> queue)
{
  queue->addAll(values);
  return shallowSize;
}

int64_t RamUsageTester::sizeOf(any obj, shared_ptr<Accumulator> accumulator)
{
  return measureObjectSize(obj, accumulator);
}

int64_t RamUsageTester::sizeOf(any obj)
{
  return sizeOf(obj, make_shared<Accumulator>());
}

wstring RamUsageTester::humanSizeOf(any object)
{
  return RamUsageEstimator::humanReadableUnits(sizeOf(object));
}

int64_t RamUsageTester::measureObjectSize(any root,
                                            shared_ptr<Accumulator> accumulator)
{
  // Objects seen so far.
  shared_ptr<Set<any>> *const seen =
      Collections::newSetFromMap(make_shared<IdentityHashMap<any, bool>>());
  // Class cache with reference Field and precalculated shallow size.
  shared_ptr<IdentityHashMap<type_info,
                             std::shared_ptr<ClassCache>>> *const classCache =
      make_shared<IdentityHashMap<type_info, std::shared_ptr<ClassCache>>>();
  // Stack of objects pending traversal. Recursion caused stack overflows.
  const deque<any> stack = deque<any>();
  stack.push_back(root);

  int64_t totalSize = 0;
  while (!stack.empty()) {
    constexpr any ob = stack.pop_back();

    if (ob == nullptr || seen->contains(ob)) {
      continue;
    }
    seen->add(ob);

    constexpr type_info obClazz = ob.type();
    assert((obClazz != nullptr, L"jvm bug detected (Object.getClass() == "
                                L"null). please report this to your vendor"));
    if (obClazz.isArray()) {
      /*
       * Consider an array, possibly of primitive types. Push any of its
       * references to the processing stack and accumulate this array's shallow
       * size.
       */
      constexpr int64_t shallowSize = RamUsageEstimator::shallowSizeOf(ob);
      constexpr int len = Array::getLength(ob);
      const deque<any> values;
      type_info componentClazz = obClazz.getComponentType();
      if (componentClazz.isPrimitive()) {
        values = Collections::emptyList();
      } else {
        values = make_shared<AbstractListAnonymousInnerClass>(ob, len);
      }
      totalSize += accumulator->accumulateArray(ob, shallowSize, values, stack);
    } else {
      /*
       * Consider an object. Push any references it has to the processing stack
       * and accumulate this object's shallow size.
       */
      try {
        shared_ptr<ClassCache> cachedInfo = classCache->get(obClazz);
        if (cachedInfo == nullptr) {
          classCache->put(obClazz, cachedInfo = createCacheEntry(obClazz));
        }

        bool needsReflection = true;
        if (Constants::JRE_IS_MINIMUM_JAVA9 &&
            obClazz.getName()->startsWith(L"java.")) {
          // Java 9: Best guess for some known types, as we cannot precisely
          // look into runtime classes:
          const function<int64_t(any *)> func = SIMPLE_TYPES->get(obClazz);
          if (func != nullptr) { // some simple type like std::wstring where the size
                                 // is easy to get from public properties
            totalSize += accumulator->accumulateObject(
                ob, cachedInfo->alignedShallowInstanceSize + func(ob),
                Collections::emptyMap(), stack);
            needsReflection = false;
          } else if (dynamic_cast<deque>(ob) != nullptr) {
            // C++ TODO: Java wildcard generics are not converted to C++:
            // ORIGINAL LINE: final java.util.List<Object> values =
            // java.util.stream.StreamSupport.stream(((Iterable<?>)
            // ob).spliterator(),
            // false).collect(java.util.stream.Collectors.toList());
                const deque<any> values = StreamSupport::stream((any_cast<deque<?>>(ob)).spliterator(), false).collect(Collectors::toList());
                totalSize += accumulator->accumulateArray(
                    ob,
                    cachedInfo->alignedShallowInstanceSize +
                        RamUsageEstimator::NUM_BYTES_ARRAY_HEADER,
                    values, stack);
                needsReflection = false;
          } else if (dynamic_cast<unordered_map>(ob) != nullptr) {
            // C++ TODO: Java wildcard generics are not converted to C++:
            // ORIGINAL LINE: final java.util.List<Object> values =
            // ((java.util.Map<?,?>) ob).entrySet().stream().flatMap(e ->
            // java.util.stream.Stream.of(e.getKey(),
            // e.getValue())).collect(java.util.stream.Collectors.toList());
                const deque<any> values = (any_cast<unordered_map<?, ?>>(ob)).entrySet().stream().flatMap([&] (any e)
                {
              java::util::stream::Stream::of(e::getKey(), e::getValue());
                }).collect(Collectors::toList());
                totalSize += accumulator->accumulateArray(
                    ob,
                    cachedInfo->alignedShallowInstanceSize +
                        RamUsageEstimator::NUM_BYTES_ARRAY_HEADER,
                    values, stack);
                totalSize += RamUsageEstimator::NUM_BYTES_ARRAY_HEADER;
                needsReflection = false;
          }
        }
        if (needsReflection) {
          const unordered_map<std::shared_ptr<Field>, any> fieldValues =
              unordered_map<std::shared_ptr<Field>, any>();
          for (auto f : cachedInfo->referenceFields) {
            fieldValues.emplace(f, f->get(ob));
          }
          totalSize += accumulator->accumulateObject(
              ob, cachedInfo->alignedShallowInstanceSize, fieldValues, stack);
        }
      } catch (const IllegalAccessException &e) {
        // this should never happen as we enabled setAccessible().
        // C++ TODO: This exception's constructor requires only one argument:
        // ORIGINAL LINE: throw new RuntimeException("Reflective field access
        // failed?", e);
        throw runtime_error(L"Reflective field access failed?");
      }
    }
  }

  // Help the GC (?).
  seen->clear();
  stack.clear();
  classCache->clear();

  return totalSize;
}

RamUsageTester::AbstractListAnonymousInnerClass::
    AbstractListAnonymousInnerClass(any ob, int len)
{
  this->ob = ob;
  this->len = len;
}

any RamUsageTester::AbstractListAnonymousInnerClass::get(int index)
{
  return Array->get(ob, index);
}

int RamUsageTester::AbstractListAnonymousInnerClass::size() { return len; }

const unordered_map<type_info, function<int64_t(any *)>>
    RamUsageTester::SIMPLE_TYPES = make_shared<IdentityHashMapAnonymousInnerClass>(any *);

RamUsageTester::IdentityHashMapAnonymousInnerClass::
    IdentityHashMapAnonymousInnerClass(shared_ptr<UnknownType> std)
    : java::util::IdentityHashMap<Class, function<long(any *)>>(any *)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "We measure some forbidden
// classes") private void init()
void RamUsageTester::IdentityHashMapAnonymousInnerClass::init()
{
  // std::wstring types:
  a(wstring::typeid, [&](any v) {
    charArraySize(v->length());
  }); // may not be correct with Java 9's compact strings!
  a(StringBuilder::typeid, [&](any v) { charArraySize(v->capacity()); });
  a(StringBuilder::typeid, [&](any v) { charArraySize(v->capacity()); });
  // Types with large buffers:
  a(ByteArrayOutputStream::typeid, [&](any v) { byteArraySize(v->size()); });
  // For File and Path, we just take the length of std::wstring representation as
  // approximation:
  a(File::typeid, [&](any v) { charArraySize(v->toString()->length()); });
  a(Path::typeid, [&](any v) { charArraySize(v->toString()->length()); });
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") private <T> void a(Class<T>
// clazz, java.util.function.ToLongFunction<T> func)
void RamUsageTester::IdentityHashMapAnonymousInnerClass::a(
    type_info<T> &clazz, function<int64_t(T *)> &func)
{
  put(clazz, static_cast<function<int64_t(any *)>>(func));
}

int64_t
RamUsageTester::IdentityHashMapAnonymousInnerClass::charArraySize(int len)
{
  return RamUsageEstimator::alignObjectSize(
      static_cast<int64_t>(RamUsageEstimator::NUM_BYTES_ARRAY_HEADER) +
      static_cast<int64_t>(Character::BYTES) * len);
}

int64_t
RamUsageTester::IdentityHashMapAnonymousInnerClass::byteArraySize(int len)
{
  return RamUsageEstimator::alignObjectSize(
      static_cast<int64_t>(RamUsageEstimator::NUM_BYTES_ARRAY_HEADER) + len);
}

RamUsageTester::ClassCache::ClassCache(
    int64_t alignedShallowInstanceSize,
    std::deque<std::shared_ptr<Field>> &referenceFields)
    : alignedShallowInstanceSize(alignedShallowInstanceSize),
      referenceFields(referenceFields)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "We need to access private fields
// of measured objects.") private static ClassCache createCacheEntry(final Class
// clazz)
shared_ptr<ClassCache> RamUsageTester::createCacheEntry(type_info const clazz)
{
  return AccessController::doPrivileged(
      std::static_pointer_cast<
          PrivilegedAction<std::shared_ptr<ClassCache>>>[&]() {
        shared_ptr<ClassCache> cachedInfo;
        int64_t shallowInstanceSize =
            RamUsageEstimator::NUM_BYTES_OBJECT_HEADER;
        const deque<std::shared_ptr<Field>> referenceFields =
            deque<std::shared_ptr<Field>>(32);
        for (type_info c = clazz; c != nullptr; c = c.getSuperclass()) {
          if (c == type_info::typeid) {
            continue;
          }
          std::deque<std::shared_ptr<Field>> fields = c.getDeclaredFields();
          for (auto f : fields) {
            if (!Modifier::isStatic(f->getModifiers())) {
              shallowInstanceSize =
                  RamUsageEstimator::adjustForField(shallowInstanceSize, f);
              if (!f->getType().isPrimitive()) {
                try {
                  f->setAccessible(true);
                  referenceFields.add(f);
                } catch (const runtime_error &re) {
                  if (L"java.lang.reflect.InaccessibleObjectException" ==
                      re.getClassName()) {
                  } else {
                    throw re;
                  }
                }
              }
            }
          }
        }
        cachedInfo = make_shared<ClassCache>(
            RamUsageEstimator::alignObjectSize(shallowInstanceSize),
            referenceFields.toArray(
                std::deque<std::shared_ptr<Field>>(referenceFields.size())));
        return cachedInfo;
      });
}
} // namespace org::apache::lucene::util