#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
template <typename T>
class InputOutput;
}

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
namespace org::apache::lucene::util::fst
{

using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertEquals;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertFalse;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertNotNull;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertNull;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertTrue;

/** Helper class to test FSTs. */
template <typename T>
class FSTTester : public std::enable_shared_from_this<FSTTester>
{
  GET_CLASS_NAME(FSTTester)

public:
  const std::shared_ptr<Random> random;
  const std::deque<InputOutput<T>> pairs;
  const int inputMode;
  const std::shared_ptr<Outputs<T>> outputs;
  const std::shared_ptr<Directory> dir;
  const bool doReverseLookup;
  int64_t nodeCount = 0;
  int64_t arcCount = 0;

  FSTTester(std::shared_ptr<Random> random, std::shared_ptr<Directory> dir,
            int inputMode, std::deque<InputOutput<T>> &pairs,
            std::shared_ptr<Outputs<T>> outputs, bool doReverseLookup)
      : random(random), pairs(pairs), inputMode(inputMode), outputs(outputs),
        dir(dir), doReverseLookup(doReverseLookup)
  {
  }

  static std::wstring inputToString(int inputMode,
                                    std::shared_ptr<IntsRef> term)
  {
    return inputToString(inputMode, term, true);
  }

  static std::wstring inputToString(int inputMode,
                                    std::shared_ptr<IntsRef> term,
                                    bool isValidUnicode)
  {
    if (!isValidUnicode) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return term->toString();
    } else if (inputMode == 0) {
      // utf8
      return toBytesRef(term)->utf8ToString() + L" " + term;
    } else {
      // utf32
      return UnicodeUtil::newString(term->ints, term->offset, term->length) +
             L" " + term;
    }
  }

private:
  static std::shared_ptr<BytesRef> toBytesRef(std::shared_ptr<IntsRef> ir)
  {
    std::shared_ptr<BytesRef> br = std::make_shared<BytesRef>(ir->length);
    for (int i = 0; i < ir->length; i++) {
      int x = ir->ints[ir->offset + i];
      assert(x >= 0 && x <= 255);
      br->bytes[i] = static_cast<char>(x);
    }
    br->length = ir->length;
    return br;
  }

public:
  static std::wstring getRandomString(std::shared_ptr<Random> random)
  {
    const std::wstring term;
    if (random->nextBoolean()) {
      term = TestUtil::randomRealisticUnicodeString(random);
    } else {
      // we want to mix in limited-alphabet symbols so
      // we get more sharing of the nodes given how few
      // terms we are testing...
      term = simpleRandomString(random);
    }
    return term;
  }

  static std::wstring simpleRandomString(std::shared_ptr<Random> r)
  {
    constexpr int end = r->nextInt(10);
    if (end == 0) {
      // allow 0 length
      return L"";
    }
    const std::deque<wchar_t> buffer = std::deque<wchar_t>(end);
    for (int i = 0; i < end; i++) {
      buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, 97, 102));
    }
    return std::wstring(buffer, 0, end);
  }

  static std::shared_ptr<IntsRef> toIntsRef(const std::wstring &s,
                                            int inputMode)
  {
    return toIntsRef(s, inputMode, std::make_shared<IntsRefBuilder>());
  }

  static std::shared_ptr<IntsRef> toIntsRef(const std::wstring &s,
                                            int inputMode,
                                            std::shared_ptr<IntsRefBuilder> ir)
  {
    if (inputMode == 0) {
      // utf8
      return toIntsRef(std::make_shared<BytesRef>(s), ir);
    } else {
      // utf32
      return toIntsRefUTF32(s, ir);
    }
  }

  static std::shared_ptr<IntsRef>
  toIntsRefUTF32(const std::wstring &s, std::shared_ptr<IntsRefBuilder> ir)
  {
    constexpr int charLength = s.length();
    int charIdx = 0;
    int intIdx = 0;
    ir->clear();
    while (charIdx < charLength) {
      ir->grow(intIdx + 1);
      constexpr int utf32 = s.codePointAt(charIdx);
      ir->append(utf32);
      charIdx += Character::charCount(utf32);
      intIdx++;
    }
    return ir->get();
  }

  static std::shared_ptr<IntsRef> toIntsRef(std::shared_ptr<BytesRef> br,
                                            std::shared_ptr<IntsRefBuilder> ir)
  {
    ir->grow(br->length);
    ir->clear();
    for (int i = 0; i < br->length; i++) {
      ir->append(br->bytes[br->offset + i] & 0xFF);
    }
    return ir->get();
  }

  /** Holds one input/output pair. */
public:
  template <typename T>
  class InputOutput : public std::enable_shared_from_this<InputOutput>,
                      public Comparable<InputOutput<T>>
  {
    GET_CLASS_NAME(InputOutput)
  public:
    const std::shared_ptr<IntsRef> input;
    const T output;

    InputOutput(std::shared_ptr<IntsRef> input, T output)
        : input(input), output(output)
    {
    }

    int compareTo(std::shared_ptr<InputOutput<T>> other) override
    {
      if (std::dynamic_pointer_cast<InputOutput>(other) != nullptr) {
        return input->compareTo((other)->input);
      } else {
        // C++ TODO: This exception's constructor requires an argument:
        // ORIGINAL LINE: throw new IllegalArgumentException();
        throw std::invalid_argument();
      }
    }
  };

public:
  virtual void doTest(bool testPruning) 
  {
    // no pruning
    doTest(0, 0, true);

    if (testPruning) {
      // simple pruning
      doTest(TestUtil::nextInt(random, 1, 1 + pairs.size()), 0, true);

      // leafy pruning
      doTest(0, TestUtil::nextInt(random, 1, 1 + pairs.size()), true);
    }
  }

  // runs the term, returning the output, or null if term
  // isn't accepted.  if prefixLength is non-null it must be
  // length 1 int array; prefixLength[0] is set to the length
  // of the term prefix that matches
private:
  T run(std::shared_ptr<FST<T>> fst, std::shared_ptr<IntsRef> term,
        std::deque<int> &prefixLength) 
  {
    assert(prefixLength.empty() || prefixLength.size() == 1);
    std::shared_ptr<FST::Arc<T>> *const arc =
        fst->getFirstArc(std::make_shared<FST::Arc<T>>());
    constexpr T NO_OUTPUT = fst->outputs->getNoOutput();
    T output = NO_OUTPUT;
    std::shared_ptr<FST::BytesReader> *const fstReader = fst->getBytesReader();

    for (int i = 0; i <= term->length; i++) {
      constexpr int label;
      if (i == term->length) {
        label = FST::END_LABEL;
      } else {
        label = term->ints[term->offset + i];
      }
      // System.out.println("   loop i=" + i + " label=" + label + " output=" +
      // fst.outputs.outputToString(output) + " curArc: target=" + arc.target +
      // " isFinal?=" + arc.isFinal());
      if (fst->findTargetArc(label, arc, arc, fstReader) == nullptr) {
        // System.out.println("    not found");
        if (prefixLength.size() > 0) {
          prefixLength[0] = i;
          return output;
        } else {
          return nullptr;
        }
      }
      output = fst->outputs->add(output, arc->output);
    }

    if (prefixLength.size() > 0) {
      prefixLength[0] = term->length;
    }

    return output;
  }

  T randomAcceptedWord(std::shared_ptr<FST<T>> fst,
                       std::shared_ptr<IntsRefBuilder> in_) 
  {
    std::shared_ptr<FST::Arc<T>> arc =
        fst->getFirstArc(std::make_shared<FST::Arc<T>>());

    const std::deque<FST::Arc<T>> arcs = std::deque<FST::Arc<T>>();
    in_->clear();
    constexpr T NO_OUTPUT = fst->outputs->getNoOutput();
    T output = NO_OUTPUT;
    std::shared_ptr<FST::BytesReader> *const fstReader = fst->getBytesReader();

    while (true) {
      // read all arcs:
      fst->readFirstTargetArc(arc, arc, fstReader);
      arcs.push_back((std::make_shared<FST::Arc<T>>())->copyFrom(arc));
      while (!arc->isLast()) {
        fst->readNextArc(arc, fstReader);
        arcs.push_back((std::make_shared<FST::Arc<T>>())->copyFrom(arc));
      }

      // pick one
      arc = arcs[random->nextInt(arcs.size())];
      arcs.clear();

      // accumulate output
      output = fst->outputs->add(output, arc->output);

      // append label
      if (arc->label == FST::END_LABEL) {
        break;
      }

      in_->append(arc->label);
    }

    return output;
  }

public:
  virtual std::shared_ptr<FST<T>>
  doTest(int prune1, int prune2,
         bool allowRandomSuffixSharing) 
  {
    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"\nTEST: prune1=" << prune1 << L" prune2=" << prune2
                 << std::endl;
    }

    std::shared_ptr<Builder<T>> *const builder = std::make_shared<Builder<T>>(
        inputMode == 0 ? FST::INPUT_TYPE::BYTE1 : FST::INPUT_TYPE::BYTE4,
        prune1, prune2, prune1 == 0 && prune2 == 0,
        allowRandomSuffixSharing ? random->nextBoolean() : true,
        allowRandomSuffixSharing ? TestUtil::nextInt(random, 1, 10)
                                 : std::numeric_limits<int>::max(),
        outputs, true, 15);

    for (auto pair : pairs) {
      if (static_cast<std::deque>(pair->output) != nullptr) {
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: @SuppressWarnings("unchecked") java.util.List<long>
        // longValues = (java.util.List<long>) pair.output;
        std::deque<int64_t> longValues =
            static_cast<std::deque<int64_t>>(pair->output);
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: @SuppressWarnings("unchecked") final Builder<Object>
        // builderObject = (Builder<Object>) builder;
        std::shared_ptr<Builder<std::any>> *const builderObject =
            std::static_pointer_cast<Builder<std::any>>(builder);
        for (std::shared_ptr<> : : optional<int64_t> value : longValues) {
          builderObject->add(pair->input, value);
        }
      } else {
        builder->add(pair->input, pair->output);
      }
    }
    std::shared_ptr<FST<T>> fst = builder->finish();

    if (random->nextBoolean() && fst != nullptr) {
      std::shared_ptr<IOContext> context = LuceneTestCase::newIOContext(random);
      std::shared_ptr<IndexOutput> out = dir->createOutput(L"fst.bin", context);
      fst->save(out);
      delete out;
      std::shared_ptr<IndexInput> in_ = dir->openInput(L"fst.bin", context);
      try {
        fst = std::make_shared<FST<T>>(in_, outputs);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        delete in_;
        dir->deleteFile(L"fst.bin");
      }
    }

    if (LuceneTestCase::VERBOSE && pairs.size() <= 20 && fst != nullptr) {
      std::wcout << L"Printing FST as dot file to stdout:" << std::endl;
      std::shared_ptr<Writer> *const w = std::make_shared<OutputStreamWriter>(
          System::out, Charset::defaultCharset());
      Util::toDot(fst, w, false, false);
      w->flush();
      std::wcout << L"END dot file" << std::endl;
    }

    if (LuceneTestCase::VERBOSE) {
      if (fst == nullptr) {
        std::wcout << L"  fst has 0 nodes (fully pruned)" << std::endl;
      } else {
        std::wcout << L"  fst has " << builder->getNodeCount() << L" nodes and "
                   << builder->getArcCount() << L" arcs" << std::endl;
      }
    }

    if (prune1 == 0 && prune2 == 0) {
      verifyUnPruned(inputMode, fst);
    } else {
      verifyPruned(inputMode, fst, prune1, prune2);
    }

    nodeCount = builder->getNodeCount();
    arcCount = builder->getArcCount();

    return fst;
  }

protected:
  virtual bool outputsEqual(T a, T b) { return a.equals(b); }

  // FST is complete
private:
  void verifyUnPruned(int inputMode,
                      std::shared_ptr<FST<T>> fst) 
  {

    std::shared_ptr<FST<int64_t>> *const fstLong;
    std::shared_ptr<Set<int64_t>> *const validOutputs;
    int64_t minLong = std::numeric_limits<int64_t>::max();
    int64_t maxLong = std::numeric_limits<int64_t>::min();

    if (doReverseLookup) {
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("unchecked") FST<long> fstLong0 =
      // (FST<long>) fst;
      std::shared_ptr<FST<int64_t>> fstLong0 =
          std::static_pointer_cast<FST<int64_t>>(fst);
      fstLong = fstLong0;
      validOutputs = std::unordered_set<>();
      for (auto pair : pairs) {
        std::optional<int64_t> output =
            static_cast<std::optional<int64_t>>(pair->output);
        maxLong = std::max(maxLong, output);
        minLong = std::min(minLong, output);
        validOutputs->add(output);
      }
    } else {
      fstLong.reset();
      validOutputs.reset();
    }

    if (pairs.empty()) {
      assertNull(fst);
      return;
    }

    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: now verify " << pairs.size() << L" terms"
                 << std::endl;
      for (auto pair : pairs) {
        assertNotNull(pair);
        assertNotNull(pair->input);
        assertNotNull(pair->output);
        std::wcout << L"  " << inputToString(inputMode, pair->input) << L": "
                   << outputs->outputToString(pair->output) << std::endl;
      }
    }

    assertNotNull(fst);

    // visit valid pairs in order -- make sure all words
    // are accepted, and FSTEnum's next() steps through
    // them correctly
    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: check valid terms/next()" << std::endl;
    }
    {
      std::shared_ptr<IntsRefFSTEnum<T>> fstEnum =
          std::make_shared<IntsRefFSTEnum<T>>(fst);
      for (auto pair : pairs) {
        std::shared_ptr<IntsRef> term = pair->input;
        if (LuceneTestCase::VERBOSE) {
          std::wcout << L"TEST: check term=" << inputToString(inputMode, term)
                     << L" output="
                     << fst->outputs->outputToString(pair->output) << std::endl;
        }
        T output = run(fst, term, nullptr);
        assertNotNull(L"term " + inputToString(inputMode, term) +
                          L" is not accepted",
                      output);
        assertTrue(outputsEqual(pair->output, output));

        // verify enum's next
        std::shared_ptr<IntsRefFSTEnum::InputOutput<T>> t = fstEnum->next();
        assertNotNull(t);
        assertEquals(L"expected input=" + inputToString(inputMode, term) +
                         L" but fstEnum returned " +
                         inputToString(inputMode, t->input),
                     term, t->input);
        assertTrue(outputsEqual(pair->output, t->output));
      }
      assertNull(fstEnum->next());
    }

    const std::unordered_map<std::shared_ptr<IntsRef>, T> termsMap =
        std::unordered_map<std::shared_ptr<IntsRef>, T>();
    for (auto pair : pairs) {
      termsMap.emplace(pair->input, pair->output);
    }

    if (doReverseLookup && maxLong > minLong) {
      // Do random lookups so we test null (output doesn't
      // exist) case:
      assertNull(Util::getByOutput(fstLong, minLong - 7));
      assertNull(Util::getByOutput(fstLong, maxLong + 7));

      constexpr int num = LuceneTestCase::atLeast(random, 100);
      for (int iter = 0; iter < num; iter++) {
        std::optional<int64_t> v =
            TestUtil::nextLong(random, minLong, maxLong);
        std::shared_ptr<IntsRef> input = Util::getByOutput(fstLong, v);
        assertTrue(validOutputs->contains(v) || input == nullptr);
      }
    }

    // find random matching word and make sure it's valid
    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: verify random accepted terms" << std::endl;
    }
    std::shared_ptr<IntsRefBuilder> *const scratch =
        std::make_shared<IntsRefBuilder>();
    int num = LuceneTestCase::atLeast(random, 500);
    for (int iter = 0; iter < num; iter++) {
      T output = randomAcceptedWord(fst, scratch);
      assertTrue(L"accepted word " + inputToString(inputMode, scratch->get()) +
                     L" is not valid",
                 termsMap.find(scratch->get()) != termsMap.end());
      assertTrue(outputsEqual(termsMap[scratch->get()], output));

      if (doReverseLookup) {
        // System.out.println("lookup output=" + output + " outs=" +
        // fst.outputs);
        std::shared_ptr<IntsRef> input = Util::getByOutput(
            fstLong, static_cast<std::optional<int64_t>>(output));
        assertNotNull(input);
        // System.out.println("  got " + Util.toBytesRef(input, new
        // BytesRef()).utf8ToString());
        TestUtil::assertEquals(scratch->get(), input);
      }
    }

    // test IntsRefFSTEnum.seek:
    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: verify seek" << std::endl;
    }
    std::shared_ptr<IntsRefFSTEnum<T>> fstEnum =
        std::make_shared<IntsRefFSTEnum<T>>(fst);
    num = LuceneTestCase::atLeast(random, 100);
    for (int iter = 0; iter < num; iter++) {
      if (LuceneTestCase::VERBOSE) {
        std::wcout << L"  iter=" << iter << std::endl;
      }
      if (random->nextBoolean()) {
        // seek to term that doesn't exist:
        while (true) {
          std::shared_ptr<IntsRef> *const term =
              toIntsRef(getRandomString(random), inputMode);
          int pos = Collections::binarySearch(
              pairs, std::make_shared<InputOutput<T>>(term, nullptr));
          if (pos < 0) {
            pos = -(pos + 1);
            // ok doesn't exist
            // System.out.println("  seek " + inputToString(inputMode, term));
            std::shared_ptr<IntsRefFSTEnum::InputOutput<T>> *const seekResult;
            if (random->nextInt(3) == 0) {
              if (LuceneTestCase::VERBOSE) {
                std::wcout << L"  do non-exist seekExact term="
                           << inputToString(inputMode, term) << std::endl;
              }
              seekResult = fstEnum->seekExact(term);
              pos = -1;
            } else if (random->nextBoolean()) {
              if (LuceneTestCase::VERBOSE) {
                std::wcout << L"  do non-exist seekFloor term="
                           << inputToString(inputMode, term) << std::endl;
              }
              seekResult = fstEnum->seekFloor(term);
              pos--;
            } else {
              if (LuceneTestCase::VERBOSE) {
                std::wcout << L"  do non-exist seekCeil term="
                           << inputToString(inputMode, term) << std::endl;
              }
              seekResult = fstEnum->seekCeil(term);
            }

            if (pos != -1 && pos < pairs.size()) {
              // System.out.println("    got " +
              // inputToString(inputMode,seekResult.input) + " output=" +
              // fst.outputs.outputToString(seekResult.output));
              assertNotNull(L"got null but expected term=" +
                                inputToString(inputMode, pairs[pos]->input),
                            seekResult);
              if (LuceneTestCase::VERBOSE) {
                std::wcout << L"    got "
                           << inputToString(inputMode, seekResult->input)
                           << std::endl;
              }
              assertEquals(L"expected " +
                               inputToString(inputMode, pairs[pos]->input) +
                               L" but got " +
                               inputToString(inputMode, seekResult->input),
                           pairs[pos]->input, seekResult->input);
              assertTrue(outputsEqual(pairs[pos]->output, seekResult->output));
            } else {
              // seeked before start or beyond end
              // System.out.println("seek=" + seekTerm);
              assertNull(
                  L"expected null but got " +
                      (seekResult == nullptr
                           ? L"null"
                           : inputToString(inputMode, seekResult->input)),
                  seekResult);
              if (LuceneTestCase::VERBOSE) {
                std::wcout << L"    got null" << std::endl;
              }
            }

            break;
          }
        }
      } else {
        // seek to term that does exist:
        std::shared_ptr<InputOutput<T>> pair =
            pairs[random->nextInt(pairs.size())];
        std::shared_ptr<IntsRefFSTEnum::InputOutput<T>> *const seekResult;
        if (random->nextInt(3) == 2) {
          if (LuceneTestCase::VERBOSE) {
            std::wcout << L"  do exists seekExact term="
                       << inputToString(inputMode, pair->input) << std::endl;
          }
          seekResult = fstEnum->seekExact(pair->input);
        } else if (random->nextBoolean()) {
          if (LuceneTestCase::VERBOSE) {
            std::wcout << L"  do exists seekFloor "
                       << inputToString(inputMode, pair->input) << std::endl;
          }
          seekResult = fstEnum->seekFloor(pair->input);
        } else {
          if (LuceneTestCase::VERBOSE) {
            std::wcout << L"  do exists seekCeil "
                       << inputToString(inputMode, pair->input) << std::endl;
          }
          seekResult = fstEnum->seekCeil(pair->input);
        }
        assertNotNull(seekResult);
        assertEquals(L"got " + inputToString(inputMode, seekResult->input) +
                         L" but expected " +
                         inputToString(inputMode, pair->input),
                     pair->input, seekResult->input);
        assertTrue(outputsEqual(pair->output, seekResult->output));
      }
    }

    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: mixed next/seek" << std::endl;
    }

    // test mixed next/seek
    num = LuceneTestCase::atLeast(random, 100);
    for (int iter = 0; iter < num; iter++) {
      if (LuceneTestCase::VERBOSE) {
        std::wcout << L"TEST: iter " << iter << std::endl;
      }
      // reset:
      fstEnum = std::make_shared<IntsRefFSTEnum<T>>(fst);
      int upto = -1;
      while (true) {
        bool isDone = false;
        if (upto == pairs.size() - 1 || random->nextBoolean()) {
          // next
          upto++;
          if (LuceneTestCase::VERBOSE) {
            std::wcout << L"  do next" << std::endl;
          }
          isDone = fstEnum->next() == nullptr;
        } else if (upto != -1 && upto < 0.75 * pairs.size() &&
                   random->nextBoolean()) {
          int attempt = 0;
          for (; attempt < 10; attempt++) {
            std::shared_ptr<IntsRef> term =
                toIntsRef(getRandomString(random), inputMode);
            if (termsMap.find(term) == termsMap.end() &&
                term->compareTo(pairs[upto]->input) > 0) {
              int pos = Collections::binarySearch(
                  pairs, std::make_shared<InputOutput<T>>(term, nullptr));
              assert(pos < 0);
              upto = -(pos + 1);

              if (random->nextBoolean()) {
                upto--;
                assertTrue(upto != -1);
                if (LuceneTestCase::VERBOSE) {
                  std::wcout << L"  do non-exist seekFloor("
                             << inputToString(inputMode, term) << L")"
                             << std::endl;
                }
                isDone = fstEnum->seekFloor(term) == nullptr;
              } else {
                if (LuceneTestCase::VERBOSE) {
                  std::wcout << L"  do non-exist seekCeil("
                             << inputToString(inputMode, term) << L")"
                             << std::endl;
                }
                isDone = fstEnum->seekCeil(term) == nullptr;
              }

              break;
            }
          }
          if (attempt == 10) {
            continue;
          }

        } else {
          constexpr int inc = random->nextInt(pairs.size() - upto - 1);
          upto += inc;
          if (upto == -1) {
            upto = 0;
          }

          if (random->nextBoolean()) {
            if (LuceneTestCase::VERBOSE) {
              std::wcout << L"  do seekCeil("
                         << inputToString(inputMode, pairs[upto]->input) << L")"
                         << std::endl;
            }
            isDone = fstEnum->seekCeil(pairs[upto]->input) == nullptr;
          } else {
            if (LuceneTestCase::VERBOSE) {
              std::wcout << L"  do seekFloor("
                         << inputToString(inputMode, pairs[upto]->input) << L")"
                         << std::endl;
            }
            isDone = fstEnum->seekFloor(pairs[upto]->input) == nullptr;
          }
        }
        if (LuceneTestCase::VERBOSE) {
          if (!isDone) {
            std::wcout << L"    got "
                       << inputToString(inputMode, fstEnum->current()->input)
                       << std::endl;
          } else {
            std::wcout << L"    got null" << std::endl;
          }
        }

        if (upto == pairs.size()) {
          assertTrue(isDone);
          break;
        } else {
          assertFalse(isDone);
          TestUtil::assertEquals(pairs[upto]->input, fstEnum->current()->input);
          assertTrue(
              outputsEqual(pairs[upto]->output, fstEnum->current()->output));

          /*
            if (upto < pairs.size()-1) {
            int tryCount = 0;
            while(tryCount < 10) {
            final IntsRef t = toIntsRef(getRandomString(), inputMode);
            if (pairs.get(upto).input.compareTo(t) < 0) {
            final bool expected = t.compareTo(pairs.get(upto+1).input) < 0;
            if (LuceneTestCase.VERBOSE) {
            System.out.println("TEST: call beforeNext(" +
            inputToString(inputMode, t) + "); current=" +
            inputToString(inputMode, pairs.get(upto).input) + " next=" +
            inputToString(inputMode, pairs.get(upto+1).input) + " expected=" +
            expected);
            }
            assertEquals(expected, fstEnum.beforeNext(t));
            break;
            }
            tryCount++;
            }
            }
          */
        }
      }
    }
  }

private:
  template <typename T>
  class CountMinOutput : public std::enable_shared_from_this<CountMinOutput>
  {
    GET_CLASS_NAME(CountMinOutput)
  public:
    int count = 0;
    T output;
    T finalOutput;
    bool isLeaf = true;
    bool isFinal = false;
  };

  // FST is pruned
private:
  void verifyPruned(int inputMode, std::shared_ptr<FST<T>> fst, int prune1,
                    int prune2) 
  {

    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: now verify pruned " << pairs.size()
                 << L" terms; outputs=" << outputs << std::endl;
      for (auto pair : pairs) {
        std::wcout << L"  " << inputToString(inputMode, pair->input) << L": "
                   << outputs->outputToString(pair->output) << std::endl;
      }
    }

    // To validate the FST, we brute-force compute all prefixes
    // in the terms, matched to their "common" outputs, prune that
    // set according to the prune thresholds, then assert the FST
    // matches that same set.

    // NOTE: Crazy RAM intensive!!

    // System.out.println("TEST: tally prefixes");

    // build all prefixes
    const std::unordered_map<std::shared_ptr<IntsRef>, CountMinOutput<T>>
        prefixes =
            std::unordered_map<std::shared_ptr<IntsRef>, CountMinOutput<T>>();
    std::shared_ptr<IntsRefBuilder> *const scratch =
        std::make_shared<IntsRefBuilder>();
    for (auto pair : pairs) {
      scratch->copyInts(pair->input);
      for (int idx = 0; idx <= pair->input->length; idx++) {
        scratch->setLength(idx);
        std::shared_ptr<CountMinOutput<T>> cmo = prefixes[scratch->get()];
        if (cmo == nullptr) {
          cmo = std::make_shared<CountMinOutput<T>>();
          cmo->count = 1;
          cmo->output = pair->output;
          prefixes.emplace(scratch->toIntsRef(), cmo);
        } else {
          cmo->count++;
          T output1 = cmo->output;
          if (output1.equals(outputs->getNoOutput())) {
            output1 = outputs->getNoOutput();
          }
          T output2 = pair->output;
          if (output2.equals(outputs->getNoOutput())) {
            output2 = outputs->getNoOutput();
          }
          cmo->output = outputs->common(output1, output2);
        }
        if (idx == pair->input->length) {
          cmo->isFinal = true;
          cmo->finalOutput = cmo->output;
        }
      }
    }

    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: now prune" << std::endl;
    }

    // prune 'em
    constexpr std::unordered_map<std::shared_ptr<IntsRef>,
                                 CountMinOutput<T>>::const_iterator it =
        prefixes.begin();
    while (it != prefixes.end()) {
      std::unordered_map::Entry<std::shared_ptr<IntsRef>, CountMinOutput<T>>
          ent = *it;
      std::shared_ptr<IntsRef> *const prefix = ent.getKey();
      std::shared_ptr<CountMinOutput<T>> *const cmo = ent.getValue();
      if (LuceneTestCase::VERBOSE) {
        std::wcout << L"  term prefix="
                   << inputToString(inputMode, prefix, false) << L" count="
                   << cmo->count << L" isLeaf=" << cmo->isLeaf << L" output="
                   << outputs->outputToString(cmo->output) << L" isFinal="
                   << cmo->isFinal << std::endl;
      }
      constexpr bool keep;
      if (prune1 > 0) {
        keep = cmo->count >= prune1;
      } else {
        assert(prune2 > 0);
        if (prune2 > 1 && cmo->count >= prune2) {
          keep = true;
        } else if (prefix->length > 0) {
          // consult our parent
          scratch->setLength(prefix->length - 1);
          System::arraycopy(prefix->ints, prefix->offset, scratch->ints(), 0,
                            scratch->length());
          std::shared_ptr<CountMinOutput<T>> *const cmo2 =
              prefixes[scratch->get()];
          // System.out.println("    parent count = " + (cmo2 == null ? -1 :
          // cmo2.count));
          keep = cmo2 != nullptr &&
                 ((prune2 > 1 && cmo2->count >= prune2) ||
                  (prune2 == 1 && (cmo2->count >= 2 || prefix->length <= 1)));
        } else if (cmo->count >= prune2) {
          keep = true;
        } else {
          keep = false;
        }
      }

      if (!keep) {
        it.remove();
        // System.out.println("    remove");
      } else {
        // clear isLeaf for all ancestors
        // System.out.println("    keep");
        scratch->copyInts(prefix);
        scratch->setLength(scratch->length() - 1);
        while (scratch->length() >= 0) {
          std::shared_ptr<CountMinOutput<T>> *const cmo2 =
              prefixes[scratch->get()];
          if (cmo2 != nullptr) {
            // System.out.println("    clear isLeaf " + inputToString(inputMode,
            // scratch));
            cmo2->isLeaf = false;
          }
          scratch->setLength(scratch->length() - 1);
        }
      }
      it++;
    }

    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: after prune" << std::endl;
      for (auto ent : prefixes) {
        std::wcout << L"  " << inputToString(inputMode, ent.first, false)
                   << L": isLeaf=" << ent.second::isLeaf << L" isFinal="
                   << ent.second::isFinal << std::endl;
        if (ent.second::isFinal) {
          std::wcout << L"    finalOutput="
                     << outputs->outputToString(ent.second::finalOutput)
                     << std::endl;
        }
      }
    }

    if (prefixes.size() <= 1) {
      assertNull(fst);
      return;
    }

    assertNotNull(fst);

    // make sure FST only enums valid prefixes
    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: check pruned enum" << std::endl;
    }
    std::shared_ptr<IntsRefFSTEnum<T>> fstEnum =
        std::make_shared<IntsRefFSTEnum<T>>(fst);
    std::shared_ptr<IntsRefFSTEnum::InputOutput<T>> current;
    while ((current = fstEnum->next()) != nullptr) {
      if (LuceneTestCase::VERBOSE) {
        std::wcout << L"  fstEnum.next prefix="
                   << inputToString(inputMode, current->input, false)
                   << L" output=" << outputs->outputToString(current->output)
                   << std::endl;
      }
      std::shared_ptr<CountMinOutput<T>> *const cmo = prefixes[current->input];
      assertNotNull(cmo);
      assertTrue(cmo->isLeaf || cmo->isFinal);
      // if (cmo.isFinal && !cmo.isLeaf) {
      if (cmo->isFinal) {
        TestUtil::assertEquals(cmo->finalOutput, current->output);
      } else {
        TestUtil::assertEquals(cmo->output, current->output);
      }
    }

    // make sure all non-pruned prefixes are present in the FST
    if (LuceneTestCase::VERBOSE) {
      std::wcout << L"TEST: verify all prefixes" << std::endl;
    }
    const std::deque<int> stopNode = std::deque<int>(1);
    for (auto ent : prefixes) {
      if (ent.first->length > 0) {
        std::shared_ptr<CountMinOutput<T>> *const cmo = ent.second;
        constexpr T output = run(fst, ent.first, stopNode);
        if (LuceneTestCase::VERBOSE) {
          std::wcout << L"TEST: verify prefix="
                     << inputToString(inputMode, ent.first, false)
                     << L" output=" << outputs->outputToString(cmo->output)
                     << std::endl;
        }
        // if (cmo.isFinal && !cmo.isLeaf) {
        if (cmo->isFinal) {
          TestUtil::assertEquals(cmo->finalOutput, output);
        } else {
          TestUtil::assertEquals(cmo->output, output);
        }
        TestUtil::assertEquals(ent.first->length, stopNode[0]);
      }
    }
  }
};

} // namespace org::apache::lucene::util::fst
