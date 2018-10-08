#pragma once
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
 * A convenient class which offers a semi-immutable object wrapper
 * implementation which allows one to set the value of an object exactly once,
 * and retrieve it many times. If {@link #set(Object)} is called more than once,
 * {@link AlreadySetException} is thrown and the operation
 * will fail.
 *
 * @lucene.experimental
 */
template <typename T>
class SetOnce final : public std::enable_shared_from_this<SetOnce>,
                      public Cloneable
{
  GET_CLASS_NAME(SetOnce)

  /** Thrown when {@link SetOnce#set(Object)} is called more than once. */
public:
  class AlreadySetException final : public IllegalStateException
  {
    GET_CLASS_NAME(AlreadySetException)
  public:
    AlreadySetException();

  protected:
    std::shared_ptr<AlreadySetException> shared_from_this()
    {
      return std::static_pointer_cast<AlreadySetException>(
          IllegalStateException::shared_from_this());
    }
  };

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile T obj = null;
  T obj = nullptr;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicBoolean> set_;

  /**
   * A default constructor which does not set the internal object, and allows
   * setting it by calling {@link #set(Object)}.
   */
public:
  SetOnce() : set(std::make_shared<AtomicBoolean>(false)) {}

  /**
   * Creates a new instance with the internal object set to the given object.
   * Note that any calls to {@link #set(Object)} afterwards will result in
   * {@link AlreadySetException}
   *
   * @throws AlreadySetException if called more than once
   * @see #set(Object)
   */
  SetOnce(T obj) : set(std::make_shared<AtomicBoolean>(true))
  {
    this->obj = obj;
  }

  /** Sets the given object. If the object has already been set, an exception is
   * thrown. */
  void set(T obj)
  {
    if (set_->compareAndSet(false, true)) {
      this->obj = obj;
    } else {
      throw std::make_shared<AlreadySetException>();
    }
  }

  /** Returns the object set by {@link #set(Object)}. */
  T get() { return obj; }
};

} // namespace org::apache::lucene::util
