#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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
namespace org::apache::lucene::spatial::query
{

using org::locationtech::spatial4j::shape::Shape;

/**
 * A predicate that compares a stored geometry to a supplied geometry. It's
 * enum-like. For more explanation of each predicate, consider looking at the
 * source implementation of {@link
 * #evaluate(org.locationtech.spatial4j.shape.Shape,
 * org.locationtech.spatial4j.shape.Shape)}. It's important to be aware that
 * Lucene-spatial makes no distinction of shape boundaries, unlike many
 * standardized definitions. Nor does it make dimensional distinctions (e.g.
 * line vs polygon). You can lookup a predicate by "Covers" or "Contains", for
 * example, and you will get the same underlying predicate implementation.
 *
 * @see <a href="http://en.wikipedia.org/wiki/DE-9IM">DE-9IM at Wikipedia, based
 * on OGC specs</a>
 * @see <a
 * href="http://edndoc.esri.com/arcsde/9.1/general_topics/understand_spatial_relations.htm">
 *   ESRIs docs on spatial relations</a>
 *
 * @lucene.experimental
 */
class SpatialOperation : public std::enable_shared_from_this<SpatialOperation>,
                         public Serializable
{
  GET_CLASS_NAME(SpatialOperation)
  // TODO rename to SpatialPredicate. Use enum?  LUCENE-5771

  // Private registry
private:
  static const std::unordered_map<std::wstring,
                                  std::shared_ptr<SpatialOperation>>
      registry; // has aliases
  static const std::deque<std::shared_ptr<SpatialOperation>> deque;

  // Geometry Operations

  /** Bounding box of the *indexed* shape, then {@link #Intersects}. */
public:
  static const std::shared_ptr<SpatialOperation> BBoxIntersects;

private:
  class SpatialOperationAnonymousInnerClass;
  /** Bounding box of the *indexed* shape, then {@link #IsWithin}. */
  GET_CLASS_NAME(SpatialOperationAnonymousInnerClass;)
public:
  static const std::shared_ptr<SpatialOperation> BBoxWithin;

private:
  class SpatialOperationAnonymousInnerClass2;
  /** Meets the "Covers" OGC definition (boundary-neutral). */
public:
  static const std::shared_ptr<SpatialOperation> Contains;

private:
  class SpatialOperationAnonymousInnerClass3;
  /** Meets the "Intersects" OGC definition. */
public:
  static const std::shared_ptr<SpatialOperation> Intersects;

private:
  class SpatialOperationAnonymousInnerClass4;
  /** Meets the "Equals" OGC definition. */
public:
  static const std::shared_ptr<SpatialOperation> IsEqualTo;

private:
  class SpatialOperationAnonymousInnerClass5;
  /** Meets the "Disjoint" OGC definition. */
public:
  static const std::shared_ptr<SpatialOperation> IsDisjointTo;

private:
  class SpatialOperationAnonymousInnerClass6;
  /** Meets the "CoveredBy" OGC definition (boundary-neutral). */
public:
  static const std::shared_ptr<SpatialOperation> IsWithin;

private:
  class SpatialOperationAnonymousInnerClass7;
  /** Almost meets the "Overlaps" OGC definition, but boundary-neutral
   * (boundary==interior). */
public:
  static const std::shared_ptr<SpatialOperation> Overlaps;

private:
  class SpatialOperationAnonymousInnerClass8;

private:
  const std::wstring name;

protected:
  SpatialOperation(const std::wstring &name);

  virtual void register_(const std::wstring &name);

public:
  static std::shared_ptr<SpatialOperation> get(const std::wstring &v);

  static std::deque<std::shared_ptr<SpatialOperation>> values();

  static bool is(std::shared_ptr<SpatialOperation> op,
                 std::deque<SpatialOperation> &tst);

  /**
   * Returns whether the relationship between indexedShape and queryShape is
   * satisfied by this operation.
   */
  virtual bool evaluate(std::shared_ptr<Shape> indexedShape,
                        std::shared_ptr<Shape> queryShape) = 0;

  virtual std::wstring getName();

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::spatial::query
class SpatialOperation::SpatialOperationAnonymousInnerClass
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass)
public:
  SpatialOperationAnonymousInnerClass();

  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass2
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass2)
public:
  SpatialOperationAnonymousInnerClass2();

  //      {
  //        @register("BBoxCoveredBy"); //alias -- the better name
  //      }
  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass2>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass3
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass3)
public:
  SpatialOperationAnonymousInnerClass3();

  //      {
  //        @register("Covers"); //alias -- the better name
  //      }
  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass3> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass3>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass4
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass4)
public:
  SpatialOperationAnonymousInnerClass4();

  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass4> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass4>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass5
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass5)
public:
  SpatialOperationAnonymousInnerClass5();

  //      {
  //        @register("IsEqualTo"); //alias (deprecated)
  //      }
  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass5> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass5>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass6
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass6)
public:
  SpatialOperationAnonymousInnerClass6();

  //      {
  //        @register("IsDisjointTo"); //alias (deprecated)
  //      }
  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass6> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass6>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass7
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass7)
public:
  SpatialOperationAnonymousInnerClass7();

  //      {
  //        @register("IsWithin"); //alias (deprecated)
  //        @register("CoveredBy"); //alias -- the more appropriate name.
  //      }
  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass7> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass7>(
        SpatialOperation::shared_from_this());
  }
};
class SpatialOperation::SpatialOperationAnonymousInnerClass8
    : public SpatialOperation
{
  GET_CLASS_NAME(SpatialOperation::SpatialOperationAnonymousInnerClass8)
public:
  SpatialOperationAnonymousInnerClass8();

  bool evaluate(std::shared_ptr<Shape> indexedShape,
                std::shared_ptr<Shape> queryShape) override;

protected:
  std::shared_ptr<SpatialOperationAnonymousInnerClass8> shared_from_this()
  {
    return std::static_pointer_cast<SpatialOperationAnonymousInnerClass8>(
        SpatialOperation::shared_from_this());
  }
};
