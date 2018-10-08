using namespace std;

#include "VirtualMethod.h"

namespace org::apache::lucene::util
{

VirtualMethod<C>::ClassValueAnonymousInnerClass::ClassValueAnonymousInnerClass()
{
}

optional<int> VirtualMethod<C>::ClassValueAnonymousInnerClass::computeValue(
    type_info subclazz)
{
  return static_cast<Integer>(
      outerInstance->reflectImplementationDistance(subclazz));
}
} // namespace org::apache::lucene::util