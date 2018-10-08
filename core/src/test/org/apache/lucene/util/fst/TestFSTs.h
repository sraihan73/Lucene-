#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <limits>
#include <map_obj>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class BytesRefFSTEnum;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class PairOutputs;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"

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

using TermsEnum = org::apache::lucene::index::TermsEnum;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::fst::Util::Result;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.fst.FSTTester.getRandomString;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.fst.FSTTester.simpleRandomString;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.fst.FSTTester.toIntsRef;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" }) @Slow
// public class TestFSTs extends org.apache.lucene.util.LuceneTestCase
class TestFSTs : public LuceneTestCase
{

private:
  std::shared_ptr<MockDirectoryWrapper> dir;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testBasicFSA() ;

  // given set of terms, test the different outputs for them
private:
  void doTest(int inputMode,
              std::deque<std::shared_ptr<IntsRef>> &terms) ;

public:
  virtual void testRandomWords() ;

  virtual std::wstring inputModeToString(int mode);

private:
  void testRandomWords(int maxNumWords, int numIter) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testBigSet() throws java.io.IOException
  virtual void testBigSet() ;

  // Build FST for all unique terms in the test line docs
  // file, up until a doc limit
  virtual void testRealTerms() ;

private:
  template <typename T1>
  void assertSame(std::shared_ptr<TermsEnum> termsEnum,
                  std::shared_ptr<BytesRefFSTEnum<T1>> fstEnum,
                  bool storeOrd) ;

private:
  template <typename T>
  class VisitTerms : public std::enable_shared_from_this<VisitTerms>
  {
    GET_CLASS_NAME(VisitTerms)
  private:
    const std::shared_ptr<Path> dirOut;
    const std::shared_ptr<Path> wordsFileIn;
    int inputMode = 0;
    const std::shared_ptr<Outputs<T>> outputs;
    const std::shared_ptr<Automaton::Builder<T>> builder;

  public:
    VisitTerms(std::shared_ptr<Path> dirOut, std::shared_ptr<Path> wordsFileIn,
               int inputMode, int prune, std::shared_ptr<Outputs<T>> outputs,
               bool noArcArrays)
        : dirOut(dirOut), wordsFileIn(wordsFileIn), outputs(outputs),
          builder(std::make_shared<Automaton::Builder<>>(
              inputMode == 0 ? FST::INPUT_TYPE::BYTE1 : FST::INPUT_TYPE::BYTE4,
              0, prune, prune == 0, true, std::numeric_limits<int>::max(),
              outputs, !noArcArrays, 15))
    {
      this->inputMode = inputMode;
    }

  protected:
    virtual T getOutput(std::shared_ptr<IntsRef> input, int ord) = 0;

  public:
    virtual void run(int limit, bool verify,
                     bool verifyByOutput) 
    {

      std::shared_ptr<BufferedReader> is =
          Files::newBufferedReader(wordsFileIn, StandardCharsets::UTF_8);
      try {
        std::shared_ptr<IntsRefBuilder> *const intsRef =
            std::make_shared<IntsRefBuilder>();
        int64_t tStart = System::currentTimeMillis();
        int ord = 0;
        while (true) {
          std::wstring w = is->readLine();
          if (w == L"") {
            break;
          }
          toIntsRef(w, inputMode, intsRef);
          builder->add(intsRef->get(), getOutput(intsRef->get(), ord));

          ord++;
          if (ord % 500000 == 0) {
            std::wcout << std::wstring::format(
                              Locale::ROOT, L"%6.2fs: %9d...",
                              ((System::currentTimeMillis() - tStart) / 1000.0),
                              ord)
                       << std::endl;
          }
          if (ord >= limit) {
            break;
          }
        }

        int64_t tMid = System::currentTimeMillis();
        std::wcout << ((tMid - tStart) / 1000.0) << L" sec to add all terms"
                   << std::endl;

        assert(builder->getTermCount() == ord);
        std::shared_ptr<FST<T>> fst = builder->finish();
        int64_t tEnd = System::currentTimeMillis();
        std::wcout << ((tEnd - tMid) / 1000.0) << L" sec to finish/pack"
                   << std::endl;
        if (fst == nullptr) {
          std::wcout << L"FST was fully pruned!" << std::endl;
          exit(0);
        }

        if (dirOut == nullptr) {
          return;
        }

        std::wcout << ord << L" terms; " << builder->getNodeCount()
                   << L" nodes; " << builder->getArcCount()
                   << L" arcs; tot size " << fst->ramBytesUsed() << std::endl;
        if (builder->getNodeCount() < 100) {
          std::shared_ptr<Writer> w = Files::newBufferedWriter(
              Paths->get(L"out.dot"), StandardCharsets::UTF_8);
          Util::toDot(fst, w, false, false);
          w->close();
          std::wcout << L"Wrote FST to out.dot" << std::endl;
        }

        std::shared_ptr<Directory> dir = FSDirectory::open(dirOut);
        std::shared_ptr<IndexOutput> out =
            dir->createOutput(L"fst.bin", IOContext::DEFAULT);
        fst->save(out);
        delete out;
        std::wcout << L"Saved FST to fst.bin." << std::endl;

        if (!verify) {
          return;
        }

        /*
        IndexInput in = dir.openInput("fst.bin", IOContext.DEFAULT);
        fst = new FST<T>(in, outputs);
        in.close();
        */

        std::wcout << L"\nNow verify..." << std::endl;

        while (true) {
          for (int iter = 0; iter < 2; iter++) {
            is->close();
            is = Files::newBufferedReader(wordsFileIn, StandardCharsets::UTF_8);

            ord = 0;
            tStart = System::currentTimeMillis();
            while (true) {
              std::wstring w = is->readLine();
              if (w == L"") {
                break;
              }
              toIntsRef(w, inputMode, intsRef);
              if (iter == 0) {
                T expected = getOutput(intsRef->get(), ord);
                T actual = Util::get(fst, intsRef->get());
                if (actual == nullptr) {
                  throw std::runtime_error("unexpected null output on input=" +
                                           w);
                }
                if (!actual.equals(expected)) {
                  throw std::runtime_error(
                      "wrong output (got " + outputs->outputToString(actual) +
                      L" but expected " + outputs->outputToString(expected) +
                      L") on input=" + w);
                }
              } else {
                // Get by output
                const std::optional<int64_t> output =
                    static_cast<std::optional<int64_t>>(
                        getOutput(intsRef->get(), ord));
                // C++ TODO: Most Java annotations will not have direct C++
                // equivalents: ORIGINAL LINE: @SuppressWarnings("unchecked")
                // final org.apache.lucene.util.IntsRef actual =
                // Util.getByOutput((FST<long>) fst, output.longValue());
                std::shared_ptr<IntsRef> *const actual = Util::getByOutput(
                    std::static_pointer_cast<FST<int64_t>>(fst),
                    output.value());
                if (actual == nullptr) {
                  throw std::runtime_error(
                      "unexpected null input from output=" + output);
                }
                if (!actual->equals(intsRef)) {
                  throw std::runtime_error("wrong input (got " + actual +
                                           L" but expected " + intsRef +
                                           L" from output=" + output);
                }
              }

              ord++;
              if (ord % 500000 == 0) {
                std::wcout << ((System::currentTimeMillis() - tStart) / 1000.0)
                           << L"s: " << ord << L"..." << std::endl;
              }
              if (ord >= limit) {
                break;
              }
            }

            double totSec = ((System::currentTimeMillis() - tStart) / 1000.0);
            std::wcout << L"Verify " << (iter == 1 ? L"(by output) " : L"")
                       << L"took " << totSec << L" sec + ("
                       << static_cast<int>((totSec * 1000000000 / ord))
                       << L" nsec per lookup)" << std::endl;

            if (!verifyByOutput) {
              break;
            }
          }

          // NOTE: comment out to profile lookup...
          break;
        }

      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        is->close();
      }
    }
  };

  // TODO: try experiment: reverse terms before
  // compressing -- how much smaller?

  // TODO: can FST be used to index all internal substrings,
  // mapping to term?

  // java -cp
  // ../build/codecs/classes/java:../test-framework/lib/randomizedtesting-runner-*.jar:../build/core/classes/test:../build/core/classes/test-framework:../build/core/classes/java:../build/test-framework/classes/java:../test-framework/lib/junit-4.10.jar
  // org.apache.lucene.util.fst.TestFSTs /xold/tmp/allTerms3.txt out
  static void main(std::deque<std::wstring> &args) ;

private:
  class VisitTermsAnonymousInnerClass
      : public VisitTerms<PairOutputs::Pair<int64_t, int64_t>>
  {
    GET_CLASS_NAME(VisitTermsAnonymousInnerClass)
  private:
    std::shared_ptr<
        org::apache::lucene::util::fst::PairOutputs<int64_t, int64_t>>
        outputs;

  public:
    VisitTermsAnonymousInnerClass(
        std::shared_ptr<Path> dirOut, std::shared_ptr<Path> wordsFileIn,
        int inputMode, int prune, bool noArcArrays,
        std::shared_ptr<
            org::apache::lucene::util::fst::PairOutputs<int64_t, int64_t>>
            outputs);

    std::shared_ptr<Random> rand;
    std::shared_ptr<PairOutputs::Pair<int64_t, int64_t>>
    getOutput(std::shared_ptr<IntsRef> input, int ord) override;

  protected:
    std::shared_ptr<VisitTermsAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<VisitTermsAnonymousInnerClass>(
          VisitTerms<PairOutputs.Pair<long, long>>::shared_from_this());
    }
  };

private:
  class VisitTermsAnonymousInnerClass2 : public VisitTerms<int64_t>
  {
    GET_CLASS_NAME(VisitTermsAnonymousInnerClass2)
  public:
    VisitTermsAnonymousInnerClass2(std::shared_ptr<Path> dirOut,
                                   std::shared_ptr<Path> wordsFileIn,
                                   int inputMode, int prune, bool noArcArrays);

    std::optional<int64_t> getOutput(std::shared_ptr<IntsRef> input,
                                       int ord) override;

  protected:
    std::shared_ptr<VisitTermsAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<VisitTermsAnonymousInnerClass2>(
          VisitTerms<long>::shared_from_this());
    }
  };

private:
  class VisitTermsAnonymousInnerClass3 : public VisitTerms<int64_t>
  {
    GET_CLASS_NAME(VisitTermsAnonymousInnerClass3)
  public:
    VisitTermsAnonymousInnerClass3(std::shared_ptr<Path> dirOut,
                                   std::shared_ptr<Path> wordsFileIn,
                                   int inputMode, int prune, bool noArcArrays);

    std::shared_ptr<Random> rand;
    std::optional<int64_t> getOutput(std::shared_ptr<IntsRef> input,
                                       int ord) override;

  protected:
    std::shared_ptr<VisitTermsAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<VisitTermsAnonymousInnerClass3>(
          VisitTerms<long>::shared_from_this());
    }
  };

private:
  class VisitTermsAnonymousInnerClass4 : public VisitTerms<std::any>
  {
    GET_CLASS_NAME(VisitTermsAnonymousInnerClass4)
  private:
    std::any NO_OUTPUT;

  public:
    VisitTermsAnonymousInnerClass4(std::shared_ptr<Path> dirOut,
                                   std::shared_ptr<Path> wordsFileIn,
                                   int inputMode, int prune, bool noArcArrays,
                                   std::any NO_OUTPUT);

    std::any getOutput(std::shared_ptr<IntsRef> input, int ord) override;

  protected:
    std::shared_ptr<VisitTermsAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<VisitTermsAnonymousInnerClass4>(
          VisitTerms<Object>::shared_from_this());
    }
  };

public:
  virtual void testSingleString() ;

  virtual void testDuplicateFSAString() ;

  /*
  public void testTrivial() throws Exception {

    // Get outputs -- passing true means FST will share
    // (delta code) the outputs.  This should result in
    // smaller FST if the outputs grow monotonically.  But
    // if numbers are "random", false should give smaller
    // final size:
    final NoOutputs outputs = NoOutputs.getSingleton();

    std::wstring[] strings = new std::wstring[] {"station", "commotion", "elation",
  "elastic", "plastic", "stop", "ftop", "ftation", "stat"};

    final Builder<Object> builder = new Builder<Object>(FST.INPUT_TYPE.BYTE1,
                                                        0, 0,
                                                        true,
                                                        true,
                                                        Integer.MAX_VALUE,
                                                        outputs,
                                                        null,
                                                        true);
    Arrays.sort(strings);
    final IntsRef scratch = new IntsRef();
    for(std::wstring s : strings) {
      builder.add(Util.toIntsRef(new BytesRef(s), scratch),
  outputs.getNoOutput());
    }
    final FST<Object> fst = builder.finish();
    System.out.println("DOT before rewrite");
    Writer w = new OutputStreamWriter(new
  FileOutputStream("/mnt/scratch/before.dot")); Util.toDot(fst, w, false,
  false); w.close();

    final FST<Object> rewrite = new FST<Object>(fst, 1, 100);

    System.out.println("DOT after rewrite");
    w = new OutputStreamWriter(new FileOutputStream("/mnt/scratch/after.dot"));
    Util.toDot(rewrite, w, false, false);
    w.close();
  }
  */

  virtual void testSimple() ;

  virtual void testPrimaryKeys() ;

  virtual void testRandomTermLookup() ;

  /**
   * Test state expansion (array format) on close-to-root states. Creates
   * synthetic input that has one expanded state on each level.
   *
   * @see <a
   * href="https://issues.apache.org/jira/browse/LUCENE-2933">LUCENE-2933</a>
   */
  virtual void testExpandedCloseToRoot() ;

  virtual void testFinalOutputOnEndState() ;

  virtual void testInternalFinalState() ;

  // Make sure raw FST can differentiate between final vs
  // non-final end nodes
  virtual void testNonFinalStopNode() ;

private:
  void checkStopNodes(
      std::shared_ptr<FST<int64_t>> fst,
      std::shared_ptr<PositiveIntOutputs> outputs) ;

public:
  static const std::shared_ptr<Comparator<int64_t>> minLongComparator;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<int64_t>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::optional<int64_t> &left,
                std::optional<int64_t> &right) override;
  };

public:
  virtual void testShortestPaths() ;

  virtual void testRejectNoLimits() ;

private:
  class TopNSearcherAnonymousInnerClass : public Util::TopNSearcher<int64_t>
  {
    GET_CLASS_NAME(TopNSearcherAnonymousInnerClass)
  private:
    std::shared_ptr<TestFSTs> outerInstance;

    std::shared_ptr<AtomicInteger> rejectCount;

  public:
    TopNSearcherAnonymousInnerClass(
        std::shared_ptr<TestFSTs> outerInstance,
        std::shared_ptr<UnknownType> minLongComparator,
        std::shared_ptr<AtomicInteger> rejectCount);

  protected:
    bool acceptResult(std::shared_ptr<IntsRef> input,
                      std::optional<int64_t> &output) override;

  protected:
    std::shared_ptr<TopNSearcherAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TopNSearcherAnonymousInnerClass>(
          Util.TopNSearcher<long>::shared_from_this());
    }
  };

private:
  class TopNSearcherAnonymousInnerClass2 : public Util::TopNSearcher<int64_t>
  {
    GET_CLASS_NAME(TopNSearcherAnonymousInnerClass2)
  private:
    std::shared_ptr<TestFSTs> outerInstance;

    std::shared_ptr<AtomicInteger> rejectCount;

  public:
    TopNSearcherAnonymousInnerClass2(
        std::shared_ptr<TestFSTs> outerInstance,
        std::shared_ptr<UnknownType> minLongComparator,
        std::shared_ptr<AtomicInteger> rejectCount);

  protected:
    bool acceptResult(std::shared_ptr<IntsRef> input,
                      std::optional<int64_t> &output) override;

  protected:
    std::shared_ptr<TopNSearcherAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<TopNSearcherAnonymousInnerClass2>(
          Util.TopNSearcher<long>::shared_from_this());
    }
  };

  // compares just the weight side of the pair
public:
  static const std::shared_ptr<Comparator<Pair<int64_t, int64_t>>>
      minPairWeightComparator;

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<Pair<int64_t, int64_t>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  public:
    ComparatorAnonymousInnerClass2();

    int compare(std::shared_ptr<Pair<int64_t, int64_t>> left,
                std::shared_ptr<Pair<int64_t, int64_t>> right) override;
  };

  /** like testShortestPaths, but uses pairoutputs so we have both a weight and
   * an output */
public:
  virtual void testShortestPathsWFST() ;

  virtual void testShortestPathsRandom() ;

private:
  template <typename T>
  class TieBreakByInputComparator
      : public std::enable_shared_from_this<TieBreakByInputComparator>,
        public Comparator<Result<T>>
  {
    GET_CLASS_NAME(TieBreakByInputComparator)
  private:
    const std::shared_ptr<Comparator<T>> comparator;

  public:
    TieBreakByInputComparator(std::shared_ptr<Comparator<T>> comparator)
        : comparator(comparator)
    {
    }

    int compare(std::shared_ptr<Result<T>> a,
                std::shared_ptr<Result<T>> b) override
    {
      int cmp = comparator->compare(a->output, b->output);
      if (cmp == 0) {
        return a->input->compareTo(b->input);
      } else {
        return cmp;
      }
    }
  };

  // used by slowcompletor
public:
  class TwoLongs : public std::enable_shared_from_this<TwoLongs>
  {
    GET_CLASS_NAME(TwoLongs)
  public:
    int64_t a = 0;
    int64_t b = 0;

    TwoLongs(int64_t a, int64_t b);
  };

  /** like testShortestPathsRandom, but uses pairoutputs so we have both a
   * weight and an output */
public:
  virtual void testShortestPathsWFSTRandom() ;

  virtual void testLargeOutputsOnArrayArcs() ;

  virtual void testIllegallyModifyRootArc() ;

protected:
  std::shared_ptr<TestFSTs> shared_from_this()
  {
    return std::static_pointer_cast<TestFSTs>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
