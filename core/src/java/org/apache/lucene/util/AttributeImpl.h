#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include "core/src/java/org/apache/lucene/util/Attribute.h"
#include "core/src/java/org/apache/lucene/util/AttributeReflector.h"

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
 * Base class for Attributes that can be added to a
 * {@link org.apache.lucene.util.AttributeSource}.
GET_CLASS_NAME(for)
 * <p>
 * Attributes are used to add data in a dynamic, yet type-safe way to a source
 * of usually streamed objects, e. g. a {@link
org.apache.lucene.analysis.TokenStream}.
 */
class AttributeImpl : public std::enable_shared_from_this<AttributeImpl>,
                      public Attribute
{
  GET_CLASS_NAME(AttributeImpl)
  /**
   * Clears the values in this AttributeImpl and resets it to its
   * default value. If this implementation implements more than one Attribute
   * interface it clears all.
   */
public:
  virtual void clear() = 0;

  /**
   * Clears the values in this AttributeImpl and resets it to its value
   * at the end of the field. If this implementation implements more than one
   * Attribute interface it clears all. <p> The default implementation simply
   * calls {@link #clear()}
   */
  virtual void end();

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
   * add the key/values this attribute holds to the given {@link
AttributeReflector}.
   *
   * <p>Implementations look like this (e.g. for a combined attribute
implementation):
   * <pre class="prettyprint">
   *   public void reflectWith(AttributeReflector reflector) {
GET_CLASS_NAME(="prettyprint">)
   *     reflector.reflect(CharTermAttribute.class, "term", term());
   *     reflector.reflect(PositionIncrementAttribute.class,
"positionIncrement", getPositionIncrement());
   *   }
   * </pre>
   *
   * <p>If you implement this method, make sure that for each invocation, the
same set of {@link Attribute}
   * interfaces and keys are passed to {@link AttributeReflector#reflect} in the
same order, but possibly
   * different values. So don't automatically exclude e.g. {@code null}
properties!
   *
   * @see #reflectAsString(bool)
   */
  virtual void reflectWith(AttributeReflector reflector) = 0;

  /**
   * Copies the values from this Attribute into the passed-in
   * target attribute. The target implementation must support all the
   * Attributes this implementation supports.
   */
  virtual void copyTo(std::shared_ptr<AttributeImpl> target) = 0;

  /**
   * In most cases the clone is, and should be, deep in order to be able to
   * properly capture the state of all attributes.
   */
  virtual std::shared_ptr<AttributeImpl> clone();
};

} // #include  "core/src/java/org/apache/lucene/util/
