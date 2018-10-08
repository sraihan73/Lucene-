using namespace std;

#include "RamUsageEstimator.h"

namespace org::apache::lucene::util
{

RamUsageEstimator::RamUsageEstimator() {}

const unordered_map<type_info, int> RamUsageEstimator::primitiveSizes =
    make_shared<java::util::IdentityHashMap<type_info, int>>();

RamUsageEstimator::StaticConstructor::StaticConstructor()
{
  primitiveSizes.emplace(bool ::typeid, 1);
  primitiveSizes.emplace(char ::typeid, 1);
  primitiveSizes.emplace(wchar_t::typeid,
                         static_cast<Integer>(Character::BYTES));
  primitiveSizes.emplace(short ::typeid, static_cast<Integer>(Short::BYTES));
  primitiveSizes.emplace(int ::typeid, static_cast<Integer>(Integer::BYTES));
  primitiveSizes.emplace(float ::typeid, static_cast<Integer>(Float::BYTES));
  primitiveSizes.emplace(double ::typeid, static_cast<Integer>(Double::BYTES));
  primitiveSizes.emplace(int64_t ::typeid, static_cast<Integer>(Long::BYTES));
  if (Constants::JRE_IS_64BIT) {
    // Try to get compressed oops and object alignment (the default seems to be
    // 8 on Hotspot); (this only works on 64 bit, on 32 bits the alignment and
    // reference size is fixed):
    bool compressedOops = false;
    int objectAlignment = 8;
    bool isHotspot = false;
    try {
      constexpr type_info beanClazz = type_info::forName(HOTSPOT_BEAN_CLASS);
      // we use reflection for this, because the management factory is not part
      // of Java 8's compact profile:
      constexpr any hotSpotBean =
          type_info::forName(MANAGEMENT_FACTORY_CLASS)
              .getMethod(L"getPlatformMXBean", type_info::typeid)
              .invoke(nullptr, beanClazz);
      if (hotSpotBean != nullptr) {
        isHotspot = true;
        shared_ptr<Method> *const getVMOptionMethod =
            beanClazz.getMethod(L"getVMOption", wstring::typeid);
        try {
          constexpr any vmOption =
              getVMOptionMethod->invoke(hotSpotBean, L"UseCompressedOops");
          // C++ TODO: There is no native C++ equivalent to 'toString':
          compressedOops =
              StringHelper::fromString<bool>(vmOption.type()
                                                 .getMethod(L"getValue")
                                                 .invoke(vmOption)
                                                 ->toString());
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (ReflectiveOperationException | runtime_error e) {
          isHotspot = false;
        }
        try {
          constexpr any vmOption =
              getVMOptionMethod->invoke(hotSpotBean, L"ObjectAlignmentInBytes");
          // C++ TODO: There is no native C++ equivalent to 'toString':
          objectAlignment = stoi(vmOption.type()
                                     .getMethod(L"getValue")
                                     .invoke(vmOption)
                                     ->toString());
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (ReflectiveOperationException | runtime_error e) {
          isHotspot = false;
        }
      }
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (ReflectiveOperationException | runtime_error e) {
      isHotspot = false;
    }
    JVM_IS_HOTSPOT_64BIT = isHotspot;
    COMPRESSED_REFS_ENABLED = compressedOops;
    NUM_BYTES_OBJECT_ALIGNMENT = objectAlignment;
    // reference size is 4, if we have compressed oops:
    NUM_BYTES_OBJECT_REF = COMPRESSED_REFS_ENABLED ? 4 : 8;
    // "best guess" based on reference size:
    NUM_BYTES_OBJECT_HEADER = 8 + NUM_BYTES_OBJECT_REF;
    // array header is NUM_BYTES_OBJECT_HEADER + NUM_BYTES_INT, but aligned
    // (object alignment):
    NUM_BYTES_ARRAY_HEADER = static_cast<int>(
        alignObjectSize(NUM_BYTES_OBJECT_HEADER + Integer::BYTES));
  } else {
    JVM_IS_HOTSPOT_64BIT = false;
    COMPRESSED_REFS_ENABLED = false;
    NUM_BYTES_OBJECT_ALIGNMENT = 8;
    NUM_BYTES_OBJECT_REF = 4;
    NUM_BYTES_OBJECT_HEADER = 8;
    // For 32 bit JVMs, no extra alignment of array header:
    NUM_BYTES_ARRAY_HEADER = NUM_BYTES_OBJECT_HEADER + Integer::BYTES;
  }

  // get min/max value of cached Long class instances:
  int64_t longCacheMinValue = 0;
  while (longCacheMinValue > numeric_limits<int64_t>::min() &&
         static_cast<int64_t>(longCacheMinValue - 1) ==
             static_cast<int64_t>(longCacheMinValue - 1)) {
    longCacheMinValue -= 1;
  }
  int64_t longCacheMaxValue = -1;
  while (longCacheMaxValue < numeric_limits<int64_t>::max() &&
         static_cast<int64_t>(longCacheMaxValue + 1) ==
             static_cast<int64_t>(longCacheMaxValue + 1)) {
    longCacheMaxValue += 1;
  }
  LONG_CACHE_MIN_VALUE = longCacheMinValue;
  LONG_CACHE_MAX_VALUE = longCacheMaxValue;
  LONG_SIZE = static_cast<int>(shallowSizeOfInstance(Long::typeid));
}

RamUsageEstimator::StaticConstructor RamUsageEstimator::staticConstructor;
const wstring RamUsageEstimator::MANAGEMENT_FACTORY_CLASS =
    L"java.lang.management.ManagementFactory";
const wstring RamUsageEstimator::HOTSPOT_BEAN_CLASS =
    L"com.sun.management.HotSpotDiagnosticMXBean";

int64_t RamUsageEstimator::alignObjectSize(int64_t size)
{
  size += static_cast<int64_t>(NUM_BYTES_OBJECT_ALIGNMENT) - 1LL;
  return size - (size % NUM_BYTES_OBJECT_ALIGNMENT);
}

int64_t RamUsageEstimator::sizeOf(optional<int64_t> &value)
{
  if (value >= LONG_CACHE_MIN_VALUE && value <= LONG_CACHE_MAX_VALUE) {
    return 0;
  }
  return LONG_SIZE;
}

int64_t RamUsageEstimator::sizeOf(std::deque<char> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<bool> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<wchar_t> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(Character::BYTES) * arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<short> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(Short::BYTES) * arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<int> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(Integer::BYTES) * arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<float> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(Float::BYTES) * arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<int64_t> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(Long::BYTES) * arr.size());
}

int64_t RamUsageEstimator::sizeOf(std::deque<double> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(Double::BYTES) * arr.size());
}

int64_t RamUsageEstimator::shallowSizeOf(std::deque<any> &arr)
{
  return alignObjectSize(static_cast<int64_t>(NUM_BYTES_ARRAY_HEADER) +
                         static_cast<int64_t>(NUM_BYTES_OBJECT_REF) *
                             arr.size());
}

int64_t RamUsageEstimator::shallowSizeOf(any obj)
{
  if (obj == nullptr) {
    return 0;
  }
  constexpr type_info clz = obj.type();
  if (clz.isArray()) {
    return shallowSizeOfArray(obj);
  } else {
    return shallowSizeOfInstance(clz);
  }
}

int64_t RamUsageEstimator::shallowSizeOfInstance(type_info clazz)
{
  if (clazz.isArray()) {
    throw invalid_argument(L"This method does not work with array classes.");
  }
  if (clazz.isPrimitive()) {
    return primitiveSizes[clazz];
  }

  int64_t size = NUM_BYTES_OBJECT_HEADER;

  // Walk type hierarchy
  for (; clazz != nullptr; clazz = clazz.getSuperclass()) {
    constexpr type_info target = clazz;
    std::deque<std::shared_ptr<Field>> fields = AccessController::doPrivileged(
        make_shared<PrivilegedActionAnonymousInnerClass>(target));
    for (auto f : fields) {
      if (!Modifier::isStatic(f->getModifiers())) {
        size = adjustForField(size, f);
      }
    }
  }
  return alignObjectSize(size);
}

RamUsageEstimator::PrivilegedActionAnonymousInnerClass::
    PrivilegedActionAnonymousInnerClass(type_info target)
{
  this->target = target;
}

std::deque<std::shared_ptr<Field>>
RamUsageEstimator::PrivilegedActionAnonymousInnerClass::run()
{
  return target.getDeclaredFields();
}

int64_t RamUsageEstimator::shallowSizeOfArray(any array_)
{
  int64_t size = NUM_BYTES_ARRAY_HEADER;
  constexpr int len = Array::getLength(array_);
  if (len > 0) {
    type_info arrayElementClazz = array_.type().getComponentType();
    if (arrayElementClazz.isPrimitive()) {
      size += static_cast<int64_t>(len) * primitiveSizes[arrayElementClazz];
    } else {
      size += static_cast<int64_t>(NUM_BYTES_OBJECT_REF) * len;
    }
  }
  return alignObjectSize(size);
}

int64_t RamUsageEstimator::adjustForField(int64_t sizeSoFar,
                                            shared_ptr<Field> f)
{
  constexpr type_info type = f->getType();
  constexpr int fsize =
      type.isPrimitive() ? primitiveSizes[type] : NUM_BYTES_OBJECT_REF;
  // TODO: No alignments based on field type/ subclass fields alignments?
  return sizeSoFar + fsize;
}

wstring RamUsageEstimator::humanReadableUnits(int64_t bytes)
{
  return humanReadableUnits(
      bytes, make_shared<DecimalFormat>(
                 L"0.#", DecimalFormatSymbols::getInstance(Locale::ROOT)));
}

wstring RamUsageEstimator::humanReadableUnits(int64_t bytes,
                                              shared_ptr<DecimalFormat> df)
{
  if (bytes / ONE_GB > 0) {
    return df->format(static_cast<float>(bytes) / ONE_GB) + L" GB";
  } else if (bytes / ONE_MB > 0) {
    return df->format(static_cast<float>(bytes) / ONE_MB) + L" MB";
  } else if (bytes / ONE_KB > 0) {
    return df->format(static_cast<float>(bytes) / ONE_KB) + L" KB";
  } else {
    return to_wstring(bytes) + L" bytes";
  }
}

int64_t RamUsageEstimator::sizeOf(
    std::deque<std::shared_ptr<Accountable>> &accountables)
{
  int64_t size = shallowSizeOf(accountables);
  for (auto accountable : accountables) {
    if (accountable != nullptr) {
      size += accountable->ramBytesUsed();
    }
  }
  return size;
}
} // namespace org::apache::lucene::util