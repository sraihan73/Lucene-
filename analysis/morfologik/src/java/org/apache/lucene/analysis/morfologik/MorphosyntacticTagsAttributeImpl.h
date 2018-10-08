#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include "MorphosyntacticTagsAttribute.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"

// -*- c-basic-offset: 2 -*-
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
namespace org::apache::lucene::analysis::morfologik
{

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

/**
 * Morphosyntactic annotations for surface forms.
 * @see MorphosyntacticTagsAttribute
 */
class MorphosyntacticTagsAttributeImpl : public AttributeImpl,
                                         public MorphosyntacticTagsAttribute,
                                         public Cloneable
{
  GET_CLASS_NAME(MorphosyntacticTagsAttributeImpl)

  /** Initializes this attribute with no tags */
public:
  MorphosyntacticTagsAttributeImpl();

  /**
   * A deque of potential tag variants for the current token.
   */
private:
  std::deque<std::shared_ptr<StringBuilder>> tags;

  /**
   * Returns the POS tag of the term. If you need a copy of this char sequence,
   * copy its contents (and clone {@link StringBuilder}s) because it changes
   * with each new term to avoid unnecessary memory allocations.
   */
public:
  std::deque<std::shared_ptr<StringBuilder>> getTags() override;

  void clear() override;

  bool equals(std::any other) override;

private:
  bool equal(std::any l1, std::any l2);

public:
  virtual int hashCode();

  /**
   * Sets the internal tags reference to the given deque. The contents
   * is not copied.
   */
  void setTags(std::deque<std::shared_ptr<StringBuilder>> &tags) override;

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  std::shared_ptr<MorphosyntacticTagsAttributeImpl> clone() override;

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<MorphosyntacticTagsAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<MorphosyntacticTagsAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/morfologik/
