#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

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
 * Manages reference counting for a given object. Extensions can override
 * {@link #release()} to do custom logic when reference counting hits 0.
 */
template <typename T>
class RefCount : public std::enable_shared_from_this<RefCount>
{
  GET_CLASS_NAME(RefCount)

private:
  const std::shared_ptr<AtomicInteger> refCount =
      std::make_shared<AtomicInteger>(1);

protected:
  const T object;

public:
  RefCount(T object) : object(object) {}

  /**
   * Called when reference counting hits 0. By default this method does nothing,
   * but extensions can override to e.g. release resources attached to object
   * that is managed by this class.
   */
protected:
  virtual void release()  {}

  /**
   * Decrements the reference counting of this object. When reference counting
   * hits 0, calls {@link #release()}.
   */
public:
  void decRef() 
  {
    constexpr int rc = refCount->decrementAndGet();
    if (rc == 0) {
      bool success = false;
      try {
        release();
        success = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (!success) {
          // Put reference back on failure
          refCount->incrementAndGet();
        }
      }
    } else if (rc < 0) {
      throw std::make_shared<IllegalStateException>(
          L"too many decRef calls: refCount is " + std::to_wstring(rc) +
          L" after decrement");
    }
  }

  T get() { return object; }

  /** Returns the current reference count. */
  int getRefCount() { return refCount->get(); }

  /**
   * Increments the reference count. Calls to this method must be matched with
   * calls to {@link #decRef()}.
   */
  void incRef() { refCount->incrementAndGet(); }
};

} // #include  "core/src/java/org/apache/lucene/util/
