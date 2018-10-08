#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::index
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Iterates through the postings.
 *  NOTE: you must first call {@link #nextDoc} before using
 *  any of the per-doc methods. */
class PostingsEnum : public DocIdSetIterator
{
  GET_CLASS_NAME(PostingsEnum)

  /**
   * Flag to pass to {@link TermsEnum#postings(PostingsEnum, int)} if you don't
   * require per-document postings in the returned enum.
   */
public:
  static constexpr short NONE = 0;

  /** Flag to pass to {@link TermsEnum#postings(PostingsEnum, int)}
   *  if you require term frequencies in the returned enum. */
  static const short FREQS = 1 << 3;

  /** Flag to pass to {@link TermsEnum#postings(PostingsEnum, int)}
   * if you require term positions in the returned enum. */
  static const short POSITIONS = FREQS | 1 << 4;

  /** Flag to pass to {@link TermsEnum#postings(PostingsEnum, int)}
   *  if you require offsets in the returned enum. */
  static const short OFFSETS = POSITIONS | 1 << 5;

  /** Flag to pass to  {@link TermsEnum#postings(PostingsEnum, int)}
   *  if you require payloads in the returned enum. */
  static const short PAYLOADS = POSITIONS | 1 << 6;

  /**
   * Flag to pass to {@link TermsEnum#postings(PostingsEnum, int)}
   * to get positions, payloads and offsets in the returned enum
   */
  static const short ALL = OFFSETS | PAYLOADS;

  /**
   * Returns true if the given feature is requested in the flags, false
   * otherwise.
   */
  static bool featureRequested(int flags, short feature);

private:
  std::shared_ptr<AttributeSource> atts = nullptr;

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  PostingsEnum();

  /**
   * Returns term frequency in the current document, or 1 if the field was
   * indexed with {@link IndexOptions#DOCS}. Do not call this before
   * {@link #nextDoc} is first called, nor after {@link #nextDoc} returns
   * {@link DocIdSetIterator#NO_MORE_DOCS}.
   *
   * <p>
   * <b>NOTE:</b> if the {@link PostingsEnum} was obtain with {@link #NONE},
   * the result of this method is undefined.
   */
public:
  virtual int freq() = 0;

  /** Returns the related attributes. */
  virtual std::shared_ptr<AttributeSource> attributes();

  /**
   * Returns the next position, or -1 if positions were not indexed.
   * Calling this more than {@link #freq()} times is undefined.
   */
  virtual int nextPosition() = 0;

  /** Returns start offset for the current position, or -1
   *  if offsets were not indexed. */
  virtual int startOffset() = 0;

  /** Returns end offset for the current position, or -1 if
   *  offsets were not indexed. */
  virtual int endOffset() = 0;

  /** Returns the payload at this position, or null if no
   *  payload was indexed. You should not modify anything
   *  (neither members of the returned BytesRef nor bytes
   *  in the byte[]). */
  virtual std::shared_ptr<BytesRef> getPayload() = 0;

protected:
  std::shared_ptr<PostingsEnum> shared_from_this()
  {
    return std::static_pointer_cast<PostingsEnum>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
