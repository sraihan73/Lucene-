#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cctype>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/MergeScheduler.h"
#include  "core/src/java/org/apache/lucene/index/CheckIndex.h"
#include  "core/src/java/org/apache/lucene/index/Status.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/store/RAMDirectory.h"

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

using Codec = org::apache::lucene::codecs::Codec;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Document = org::apache::lucene::document::Document;
using CheckIndex = org::apache::lucene::index::CheckIndex;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReader = org::apache::lucene::index::LeafReader;
using MergeScheduler = org::apache::lucene::index::MergeScheduler;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

/**
 * General utility methods for Lucene unit tests.
 */
class TestUtil final : public std::enable_shared_from_this<TestUtil>
{
  GET_CLASS_NAME(TestUtil)
private:
  TestUtil();

  /**
   * A comparator that compares UTF-16 strings / char sequences according to
   * Unicode code point order. This can be used to verify {@link BytesRef}
   * order. <p> <b>Warning:</b> This comparator is rather inefficient, because
   * it converts the strings to a {@code int[]} array on each invocation.
   * */
public:
  static const std::shared_ptr<Comparator<std::shared_ptr<std::wstring>>>
      STRING_CODEPOINT_COMPARATOR;

  /**
   * Convenience method unzipping zipName into destDir. You must pass it a clean
   * destDir.
   *
   * Closes the given InputStream after extracting!
   */
  static void unzip(std::shared_ptr<InputStream> in_,
                    std::shared_ptr<Path> destDir) ;

  /**
   * Checks that the provided iterator is well-formed.
   * <ul>
   *   <li>is read-only: does not allow {@code remove}
   *   <li>returns {@code expectedSize} number of elements
   *   <li>does not return null elements, unless {@code allowNull} is true.
   *   <li>throws NoSuchElementException if {@code next} is called
   *       after {@code hasNext} returns false.
   * </ul>
   */
  template <typename T>
  static void checkIterator(std::shared_ptr<Iterator<T>> iterator,
                            int64_t expectedSize, bool allowNull);

  /**
   * Checks that the provided iterator is well-formed.
   * <ul>
   *   <li>is read-only: does not allow {@code remove}
   *   <li>does not return null elements.
   *   <li>throws NoSuchElementException if {@code next} is called
   *       after {@code hasNext} returns false.
   * </ul>
   */
  template <typename T>
  static void checkIterator(std::shared_ptr<Iterator<T>> iterator);

  /**
   * Checks that the provided collection is read-only.
   * @see #checkIterator(Iterator)
   */
  template <typename T>
  static void checkReadOnly(std::shared_ptr<std::deque<T>> coll);

  static void syncConcurrentMerges(std::shared_ptr<IndexWriter> writer);

  static void syncConcurrentMerges(std::shared_ptr<MergeScheduler> ms);

  /** This runs the CheckIndex tool on the index in.  If any
   *  issues are hit, a RuntimeException is thrown; else,
   *  true is returned. */
  static std::shared_ptr<CheckIndex::Status>
  checkIndex(std::shared_ptr<Directory> dir) ;

  static std::shared_ptr<CheckIndex::Status>
  checkIndex(std::shared_ptr<Directory> dir,
             bool crossCheckTermVectors) ;

  /** If failFast is true, then throw the first exception when index corruption
   * is hit, instead of moving on to other fields/segments to
   *  look for any other corruption.  */
  static std::shared_ptr<CheckIndex::Status>
  checkIndex(std::shared_ptr<Directory> dir, bool crossCheckTermVectors,
             bool failFast,
             std::shared_ptr<ByteArrayOutputStream> output) ;

  /** This runs the CheckIndex tool on the Reader.  If any
   *  issues are hit, a RuntimeException is thrown */
  static void
  checkReader(std::shared_ptr<IndexReader> reader) ;

  static void checkReader(std::shared_ptr<LeafReader> reader,
                          bool crossCheckTermVectors) ;

  // used by TestUtil.checkReader to check some things really unrelated to the
  // index, just looking for bugs in indexreader implementations.
private:
  static void
  checkReaderSanity(std::shared_ptr<LeafReader> reader) ;

  /** start and end are BOTH inclusive */
public:
  static int nextInt(std::shared_ptr<Random> r, int start, int end);

  /** start and end are BOTH inclusive */
  static int64_t nextLong(std::shared_ptr<Random> r, int64_t start,
                            int64_t end);

  /**
   * Returns a randomish big integer with {@code 1 .. maxBytes} storage.
   */
  static std::shared_ptr<int64_t>
  nextBigInteger(std::shared_ptr<Random> random, int maxBytes);

  static std::wstring randomSimpleString(std::shared_ptr<Random> r,
                                         int maxLength);

  static std::wstring randomSimpleString(std::shared_ptr<Random> r,
                                         int minLength, int maxLength);

  static std::wstring randomSimpleStringRange(std::shared_ptr<Random> r,
                                              wchar_t minChar, wchar_t maxChar,
                                              int maxLength);

  static std::wstring randomSimpleString(std::shared_ptr<Random> r);

  /** Returns random string, including full unicode range. */
  static std::wstring randomUnicodeString(std::shared_ptr<Random> r);

  /**
   * Returns a random string up to a certain length.
   */
  static std::wstring randomUnicodeString(std::shared_ptr<Random> r,
                                          int maxLength);

  /**
   * Fills provided char[] with valid random unicode code
   * unit sequence.
   */
  static void randomFixedLengthUnicodeString(std::shared_ptr<Random> random,
                                             std::deque<wchar_t> &chars,
                                             int offset, int length);

  /**
   * Returns a std::wstring thats "regexpish" (contains lots of operators typically
   * found in regular expressions) If you call this enough times, you might get
   * a valid regex!
   */
  static std::wstring randomRegexpishString(std::shared_ptr<Random> r);

  /**
   * Maximum recursion bound for '+' and '*' replacements in
   * {@link #randomRegexpishString(Random, int)}.
   */
private:
  static constexpr int maxRecursionBound = 5;

  /**
   * Operators for {@link #randomRegexpishString(Random, int)}.
   */
  static const std::deque<std::wstring> ops;

  /**
   * Returns a std::wstring thats "regexpish" (contains lots of operators typically
   * found in regular expressions) If you call this enough times, you might get
   * a valid regex!
   *
   * <P>Note: to avoid practically endless backtracking patterns we replace
   * asterisk and plus operators with bounded repetitions. See LUCENE-4111 for
   * more info.
   *
   * @param maxLength A hint about maximum length of the regexpish string. It
   * may be exceeded by a few characters.
   */
public:
  static std::wstring randomRegexpishString(std::shared_ptr<Random> r,
                                            int maxLength);

private:
  static std::deque<std::wstring> const HTML_CHAR_ENTITIES;

public:
  static std::wstring randomHtmlishString(std::shared_ptr<Random> random,
                                          int numElements);

  /**
   * Randomly upcases, downcases, or leaves intact each code point in the given
   * string
   */
  static std::wstring randomlyRecaseCodePoints(std::shared_ptr<Random> random,
                                               const std::wstring &str);

private:
  static std::deque<int> const blockStarts;

  static std::deque<int> const blockEnds;

  /** Returns random string of length between 0-20 codepoints, all codepoints
   * within the same unicode block. */
public:
  static std::wstring randomRealisticUnicodeString(std::shared_ptr<Random> r);

  /** Returns random string of length up to maxLength codepoints , all
   * codepoints within the same unicode block. */
  static std::wstring randomRealisticUnicodeString(std::shared_ptr<Random> r,
                                                   int maxLength);

  /** Returns random string of length between min and max codepoints, all
   * codepoints within the same unicode block. */
  static std::wstring randomRealisticUnicodeString(std::shared_ptr<Random> r,
                                                   int minLength,
                                                   int maxLength);

  /** Returns random string, with a given UTF-8 byte length*/
  static std::wstring
  randomFixedByteLengthUnicodeString(std::shared_ptr<Random> r, int length);

  /** Returns a random binary term. */
  static std::shared_ptr<BytesRef> randomBinaryTerm(std::shared_ptr<Random> r);

  /** Return a Codec that can read any of the
   *  default codecs and formats, but always writes in the specified
   *  format. */
  static std::shared_ptr<Codec>
  alwaysPostingsFormat(std::shared_ptr<PostingsFormat> format);

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<PostingsFormat> format;

  public:
    AssertingCodecAnonymousInnerClass(std::shared_ptr<PostingsFormat> format);

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

  /** Return a Codec that can read any of the
   *  default codecs and formats, but always writes in the specified
   *  format. */
public:
  static std::shared_ptr<Codec>
  alwaysDocValuesFormat(std::shared_ptr<DocValuesFormat> format);

private:
  class AssertingCodecAnonymousInnerClass2 : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass2)
  private:
    std::shared_ptr<DocValuesFormat> format;

  public:
    AssertingCodecAnonymousInnerClass2(std::shared_ptr<DocValuesFormat> format);

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass2>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

  /**
   * Returns the actual default codec (e.g. LuceneMNCodec) for this version of
   * Lucene. This may be different than {@link Codec#getDefault()} because that
   * is randomized.
   */
public:
  static std::shared_ptr<Codec> getDefaultCodec();

  /**
   * Returns the actual default postings format (e.g. LuceneMNPostingsFormat for
   * this version of Lucene.
   */
  static std::shared_ptr<PostingsFormat> getDefaultPostingsFormat();

  /**
   * Returns the actual default postings format (e.g. LuceneMNPostingsFormat for
   * this version of Lucene.
   * @lucene.internal this may disappear at any time
   */
  static std::shared_ptr<PostingsFormat>
  getDefaultPostingsFormat(int minItemsPerBlock, int maxItemsPerBlock);

  /** Returns a random postings format that supports term ordinals */
  static std::shared_ptr<PostingsFormat>
  getPostingsFormatWithOrds(std::shared_ptr<Random> r);

  /**
   * Returns the actual default docvalues format (e.g. LuceneMNDocValuesFormat
   * for this version of Lucene.
   */
  static std::shared_ptr<DocValuesFormat> getDefaultDocValuesFormat();

  // TODO: generalize all 'test-checks-for-crazy-codecs' to
  // annotations (LUCENE-3489)
  static std::wstring getPostingsFormat(const std::wstring &field);

  static std::wstring getPostingsFormat(std::shared_ptr<Codec> codec,
                                        const std::wstring &field);

  static std::wstring getDocValuesFormat(const std::wstring &field);

  static std::wstring getDocValuesFormat(std::shared_ptr<Codec> codec,
                                         const std::wstring &field);

  // TODO: remove this, push this test to Lucene40/Lucene42 codec tests
  static bool fieldSupportsHugeBinaryDocValues(const std::wstring &field);

  static bool
  anyFilesExceptWriteLock(std::shared_ptr<Directory> dir) ;

  static void
  addIndexesSlowly(std::shared_ptr<IndexWriter> writer,
                   std::deque<DirectoryReader> &readers) ;

  /** just tries to configure things to keep the open file
   * count lowish */
  static void reduceOpenFiles(std::shared_ptr<IndexWriter> w);

  /** Checks some basic behaviour of an AttributeImpl
   * @param reflectedValues contains a map_obj with "AttributeClass#key" as values
   */
  template <typename T>
  static void assertAttributeReflection(
      std::shared_ptr<AttributeImpl> att,
      std::unordered_map<std::wstring, T> &reflectedValues);

  static void assertEquals(std::shared_ptr<TopDocs> expected,
                           std::shared_ptr<TopDocs> actual);

  // NOTE: this is likely buggy, and cannot clone fields
  // with tokenStreamValues, etc.  Use at your own risk!!

  // TODO: is there a pre-existing way to do this!!!
  static std::shared_ptr<Document>
  cloneDocument(std::shared_ptr<Document> doc1);

  // Returns a DocsEnum, but randomly sometimes uses a
  // DocsAndFreqsEnum, DocsAndPositionsEnum.  Returns null
  // if field/term doesn't exist:
  static std::shared_ptr<PostingsEnum>
  docs(std::shared_ptr<Random> random, std::shared_ptr<IndexReader> r,
       const std::wstring &field, std::shared_ptr<BytesRef> term,
       std::shared_ptr<PostingsEnum> reuse, int flags) ;

  // Returns a PostingsEnum with random features available
  static std::shared_ptr<PostingsEnum>
  docs(std::shared_ptr<Random> random, std::shared_ptr<TermsEnum> termsEnum,
       std::shared_ptr<PostingsEnum> reuse, int flags) ;

  static std::shared_ptr<std::wstring>
  stringToCharSequence(const std::wstring &string,
                       std::shared_ptr<Random> random);

  static std::shared_ptr<std::wstring>
  bytesToCharSequence(std::shared_ptr<BytesRef> ref,
                      std::shared_ptr<Random> random);

  /**
   * Shutdown {@link ExecutorService} and wait for its.
   */
  static void shutdownExecutorService(std::shared_ptr<ExecutorService> ex);

  /**
   * Returns a valid (compiling) Pattern instance with random stuff inside. Be
   * careful when applying random patterns to longer strings as certain types of
   * patterns may explode into exponential times in backtracking implementations
   * (such as Java's).
   */
  static std::shared_ptr<Pattern> randomPattern(std::shared_ptr<Random> random);

  static std::wstring randomAnalysisString(std::shared_ptr<Random> random,
                                           int maxLength, bool simple);

  static std::wstring randomSubString(std::shared_ptr<Random> random,
                                      int wordLength, bool simple);

  /** For debugging: tries to include br.utf8ToString(), but if that
   *  fails (because it's not valid utf8, which is fine!), just
   *  use ordinary toString. */
  static std::wstring bytesRefToString(std::shared_ptr<BytesRef> br);

  /** Returns a copy of directory, entirely in RAM */
  static std::shared_ptr<RAMDirectory>
  ramCopyOf(std::shared_ptr<Directory> dir) ;

  static bool hasWindowsFS(std::shared_ptr<Directory> dir);

  static bool hasWindowsFS(std::shared_ptr<Path> path);

  static bool hasVirusChecker(std::shared_ptr<Directory> dir);

  static bool hasVirusChecker(std::shared_ptr<Path> path);

  /** Returns true if VirusCheckingFS is in use and was in fact already enabled
   */
  static bool disableVirusChecker(std::shared_ptr<Directory> in_);

  static void enableVirusChecker(std::shared_ptr<Directory> in_);
};

} // #include  "core/src/java/org/apache/lucene/util/
