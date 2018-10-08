#pragma once
#include "stringhelper.h"
#include <memory>
#include <set>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class Result;
}
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;

/** Static helper methods.
 *
 * @lucene.experimental */
class Util final : public std::enable_shared_from_this<Util>
{
  GET_CLASS_NAME(Util)
private:
  Util();

  /** Looks up the output for this input, or null if the
   *  input is not accepted. */
public:
  template <typename T>
  static T get(std::shared_ptr<FST<T>> fst,
               std::shared_ptr<IntsRef> input) ;

  // TODO: maybe a CharsRef version for BYTE2

  /** Looks up the output for this input, or null if the
   *  input is not accepted */
  template <typename T>
  static T get(std::shared_ptr<FST<T>> fst,
               std::shared_ptr<BytesRef> input) ;

  /** Reverse lookup (lookup by output instead of by input),
   *  in the special case when your FSTs outputs are
   *  strictly ascending.  This locates the input/output
   *  pair where the output is equal to the target, and will
   *  return null if that output does not exist.
   *
   *  <p>NOTE: this only works with {@code FST<Long>}, only
   *  works when the outputs are ascending in order with
   *  the inputs.
   *  For example, simple ordinals (0, 1,
   *  2, ...), or file offets (when appending to a file)
   *  fit this. */
  static std::shared_ptr<IntsRef>
  getByOutput(std::shared_ptr<FST<int64_t>> fst,
              int64_t targetOutput) ;

  /**
   * Expert: like {@link Util#getByOutput(FST, long)} except reusing
   * BytesReader, initial and scratch Arc, and result.
   */
  static std::shared_ptr<IntsRef>
  getByOutput(std::shared_ptr<FST<int64_t>> fst, int64_t targetOutput,
              std::shared_ptr<BytesReader> in_,
              std::shared_ptr<Arc<int64_t>> arc,
              std::shared_ptr<Arc<int64_t>> scratchArc,
              std::shared_ptr<IntsRefBuilder> result) ;

  /** Represents a path in TopNSearcher.
   *
   *  @lucene.experimental
   */
public:
  template <typename T>
  class FSTPath : public std::enable_shared_from_this<FSTPath>
  {
    GET_CLASS_NAME(FSTPath)
    /** Holds the last arc appended to this path */
  public:
    std::shared_ptr<FST::Arc<T>> arc;
    /** Holds cost plus any usage-specific output: */
    T output;
    const std::shared_ptr<IntsRefBuilder> input;
    const float boost;
    const std::shared_ptr<std::wstring> context;

    // Custom int payload for consumers; the NRT suggester uses this to record
    // if this path has already enumerated a surface form
    int payload = 0;

    /** Sole constructor */
    FSTPath(T output, std::shared_ptr<FST::Arc<T>> arc,
            std::shared_ptr<IntsRefBuilder> input)
        : FSTPath(output, arc, input, 0, nullptr, -1)
    {
    }

    FSTPath(T output, std::shared_ptr<FST::Arc<T>> arc,
            std::shared_ptr<IntsRefBuilder> input, float boost,
            std::shared_ptr<std::wstring> context, int payload)
        : input(input), boost(boost), context(context)
    {
      this->arc = (std::make_shared<FST::Arc<T>>())->copyFrom(arc);
      this->output = output;
      this->payload = payload;
    }

    virtual std::shared_ptr<FSTPath<T>>
    newPath(T output, std::shared_ptr<IntsRefBuilder> input)
    {
      return std::make_shared<FSTPath<T>>(output, this->arc, input, this->boost,
                                          this->context, this->payload);
    }

    std::wstring toString() override
    {
      return L"input=" + input->get() + L" output=" + output + L" context=" +
             context + L" boost=" + std::to_wstring(boost) + L" payload=" +
             std::to_wstring(payload);
    }
  };

  /** Compares first by the provided comparator, and then
   *  tie breaks by path.input. */
private:
  template <typename T>
  class TieBreakByInputComparator
      : public std::enable_shared_from_this<TieBreakByInputComparator>,
        public Comparator<FSTPath<T>>
  {
    GET_CLASS_NAME(TieBreakByInputComparator)
  private:
    const std::shared_ptr<Comparator<T>> comparator;

  public:
    TieBreakByInputComparator(std::shared_ptr<Comparator<T>> comparator)
        : comparator(comparator)
    {
    }

    int compare(std::shared_ptr<FSTPath<T>> a,
                std::shared_ptr<FSTPath<T>> b) override
    {
      int cmp = comparator->compare(a->output, b->output);
      if (cmp == 0) {
        return a->input->get()->compareTo(b->input->get());
      } else {
        return cmp;
      }
    }
  };

  /** Utility class to find top N shortest paths from start
   *  point(s). */
public:
  template <typename T>
  class TopNSearcher : public std::enable_shared_from_this<TopNSearcher>
  {
    GET_CLASS_NAME(TopNSearcher)

  private:
    const std::shared_ptr<FST<T>> fst;
    const std::shared_ptr<BytesReader> bytesReader;
    const int topN;
    const int maxQueueDepth;

    const std::shared_ptr<FST::Arc<T>> scratchArc =
        std::make_shared<FST::Arc<T>>();

    const std::shared_ptr<Comparator<T>> comparator;
    const std::shared_ptr<Comparator<FSTPath<T>>> pathComparator;

  public:
    std::set<FSTPath<T>> queue = nullptr;

    /**
     * Creates an unbounded TopNSearcher
     * @param fst the {@link org.apache.lucene.util.fst.FST} to search on
     * @param topN the number of top scoring entries to retrieve
     * @param maxQueueDepth the maximum size of the queue of possible top
     * entries
     * @param comparator the comparator to select the top N
     */
    TopNSearcher(std::shared_ptr<FST<T>> fst, int topN, int maxQueueDepth,
                 std::shared_ptr<Comparator<T>> comparator)
        : TopNSearcher(fst, topN, maxQueueDepth, comparator,
                       new TieBreakByInputComparator<>(comparator))
    {
    }

    TopNSearcher(std::shared_ptr<FST<T>> fst, int topN, int maxQueueDepth,
                 std::shared_ptr<Comparator<T>> comparator,
                 std::shared_ptr<Comparator<FSTPath<T>>> pathComparator)
        : fst(fst), bytesReader(fst->getBytesReader()), topN(topN),
          maxQueueDepth(maxQueueDepth), comparator(comparator),
          pathComparator(pathComparator)
    {
      queue = std::set<FSTPath<T>>(pathComparator);
    }

    // If back plus this arc is competitive then add to queue:
  protected:
    virtual void addIfCompetitive(std::shared_ptr<FSTPath<T>> path)
    {

      assert(queue != nullptr);

      T output = fst->outputs->add(path->output, path->arc->output);

      if (queue.size() == maxQueueDepth) {
        std::shared_ptr<FSTPath<T>> bottom = queue.last();
        int comp = pathComparator->compare(path, bottom);
        if (comp > 0) {
          // Doesn't compete
          return;
        } else if (comp == 0) {
          // Tie break by alpha sort on the input:
          path->input->append(path->arc->label);
          constexpr int cmp =
              bottom->input->get()->compareTo(path->input->get());
          path->input->setLength(path->input->length() - 1);

          // We should never see dups:
          assert(cmp != 0);

          if (cmp < 0) {
            // Doesn't compete
            return;
          }
        }
        // Competes
      } else {
        // Queue isn't full yet, so any path we hit competes:
      }

      // copy over the current input to the new input
      // and add the arc.label to the end
      std::shared_ptr<IntsRefBuilder> newInput =
          std::make_shared<IntsRefBuilder>();
      newInput->copyInts(path->input->get());
      newInput->append(path->arc->label);

      std::shared_ptr<FSTPath<T>> newPath = path->newPath(output, newInput);
      if (acceptPartialPath(newPath)) {
        queue.insert(newPath);
        if (queue.size() == maxQueueDepth + 1) {
          queue.pollLast();
        }
      }
    }

  public:
    virtual void
    addStartPaths(std::shared_ptr<FST::Arc<T>> node, T startOutput,
                  bool allowEmptyString,
                  std::shared_ptr<IntsRefBuilder> input) 
    {
      addStartPaths(node, startOutput, allowEmptyString, input, 0, nullptr, -1);
    }

    /** Adds all leaving arcs, including 'finished' arc, if
     *  the node is final, from this node into the queue.  */
    virtual void addStartPaths(std::shared_ptr<FST::Arc<T>> node, T startOutput,
                               bool allowEmptyString,
                               std::shared_ptr<IntsRefBuilder> input,
                               float boost,
                               std::shared_ptr<std::wstring> context,
                               int payload) 
    {

      // De-dup NO_OUTPUT since it must be a singleton:
      if (startOutput.equals(fst->outputs->getNoOutput())) {
        startOutput = fst->outputs->getNoOutput();
      }

      std::shared_ptr<FSTPath<T>> path = std::make_shared<FSTPath<T>>(
          startOutput, node, input, boost, context, payload);
      fst->readFirstTargetArc(node, path->arc, bytesReader);

      // Bootstrap: find the min starting arc
      while (true) {
        if (allowEmptyString || path->arc->label != FST::END_LABEL) {
          addIfCompetitive(path);
        }
        if (path->arc->isLast()) {
          break;
        }
        fst->readNextArc(path->arc, bytesReader);
      }
    }

    virtual std::shared_ptr<TopResults<T>> search() 
    {

      const std::deque<Result<T>> results = std::deque<Result<T>>();

      std::shared_ptr<BytesReader> *const fstReader = fst->getBytesReader();
      constexpr T NO_OUTPUT = fst->outputs->getNoOutput();

      // TODO: we could enable FST to sorting arcs by weight
      // as it freezes... can easily do this on first pass
      // (w/o requiring rewrite)

      // TODO: maybe we should make an FST.INPUT_TYPE.BYTE0.5!?
      // (nibbles)
      int rejectCount = 0;

      // For each top N path:
      while (results.size() < topN) {

        std::shared_ptr<FSTPath<T>> path;

        if (queue == nullptr) {
          // Ran out of paths
          break;
        }

        // Remove top path since we are now going to
        // pursue it:
        path = queue.pollFirst();

        if (path == nullptr) {
          // There were less than topN paths available:
          break;
        }
        // System.out.println("pop path=" + path + " arc=" + path.arc.output);

        if (acceptPartialPath(path) == false) {
          continue;
        }

        if (path->arc->label == FST::END_LABEL) {
          // Empty string!
          path->input->setLength(path->input->length() - 1);
          results.push_back(
              std::make_shared<Result<>>(path->input->get(), path->output));
          continue;
        }

        if (results.size() == topN - 1 && maxQueueDepth == topN) {
          // Last path -- don't bother w/ queue anymore:
          queue = nullptr;
        }

        // We take path and find its "0 output completion",
        // ie, just keep traversing the first arc with
        // NO_OUTPUT that we can find, since this must lead
        // to the minimum path that completes from
        // path.arc.

        // For each input letter:
        while (true) {

          fst->readFirstTargetArc(path->arc, path->arc, fstReader);

          // For each arc leaving this node:
          bool foundZero = false;
          while (true) {
            // tricky: instead of comparing output == 0, we must
            // express it via the comparator compare(output, 0) == 0
            if (comparator->compare(NO_OUTPUT, path->arc->output) == 0) {
              if (queue == nullptr) {
                foundZero = true;
                break;
              } else if (!foundZero) {
                scratchArc->copyFrom(path->arc);
                foundZero = true;
              } else {
                addIfCompetitive(path);
              }
            } else if (queue != nullptr) {
              addIfCompetitive(path);
            }
            if (path->arc->isLast()) {
              break;
            }
            fst->readNextArc(path->arc, fstReader);
          }

          assert(foundZero);

          if (queue != nullptr) {
            // TODO: maybe we can save this copyFrom if we
            // are more clever above... eg on finding the
            // first NO_OUTPUT arc we'd switch to using
            // scratchArc
            path->arc->copyFrom(scratchArc);
          }

          if (path->arc->label == FST::END_LABEL) {
            // Add final output:
            path->output = fst->outputs->add(path->output, path->arc->output);
            if (acceptResult(path)) {
              results.push_back(
                  std::make_shared<Result<>>(path->input->get(), path->output));
            } else {
              rejectCount++;
            }
            break;
          } else {
            path->input->append(path->arc->label);
            path->output = fst->outputs->add(path->output, path->arc->output);
            if (acceptPartialPath(path) == false) {
              break;
            }
          }
        }
      }
      return std::make_shared<TopResults<T>>(
          rejectCount + topN <= maxQueueDepth, results);
    }

  protected:
    virtual bool acceptResult(std::shared_ptr<FSTPath<T>> path)
    {
      return acceptResult(path->input->get(), path->output);
    }

    /** Override this to prevent considering a path before it's complete */
    virtual bool acceptPartialPath(std::shared_ptr<FSTPath<T>> path)
    {
      return true;
    }

    virtual bool acceptResult(std::shared_ptr<IntsRef> input, T output)
    {
      return true;
    }
  };

  /** Holds a single input (IntsRef) + output, returned by
   *  {@link #shortestPaths shortestPaths()}. */
public:
  template <typename T>
  class Result final : public std::enable_shared_from_this<Result>
  {
    GET_CLASS_NAME(Result)
  public:
    const std::shared_ptr<IntsRef> input;
    const T output;
    Result(std::shared_ptr<IntsRef> input, T output)
        : input(input), output(output)
    {
    }
  };

  /**
   * Holds the results for a top N search using {@link TopNSearcher}
   */
public:
  template <typename T>
  class TopResults final : public std::enable_shared_from_this<TopResults>,
                           public std::deque<Result<T>>
  {
    GET_CLASS_NAME(TopResults)

    /**
     * <code>true</code> iff this is a complete result ie. if
     * the specified queue size was large enough to find the complete deque of
     * results. This might be <code>false</code> if the {@link TopNSearcher}
     * rejected too many results.
     */
  public:
    const bool isComplete;
    /**
     * The top results
     */
    const std::deque<Result<T>> topN;

    TopResults(bool isComplete, std::deque<Result<T>> &topN)
        : isComplete(isComplete), topN(topN)
    {
    }

    std::shared_ptr<Iterator<Result<T>>> iterator() override
    {
      return topN.begin();
    }
  };

  /** Starting from node, find the top N min cost
   *  completions to a final node. */
public:
  template <typename T>
  static std::shared_ptr<TopResults<T>>
  shortestPaths(std::shared_ptr<FST<T>> fst,
                std::shared_ptr<FST::Arc<T>> fromNode, T startOutput,
                std::shared_ptr<Comparator<T>> comparator, int topN,
                bool allowEmptyString) ;

  /**
   * Dumps an {@link FST} to a GraphViz's <code>dot</code> language description
   * for visualization. Example of use:
   *
   * <pre class="prettyprint">
   * PrintWriter pw = new PrintWriter(&quot;out.dot&quot;);
   * Util.toDot(fst, pw, true, true);
   * pw.close();
   * </pre>
   *
   * and then, from command line:
   *
   * <pre>
   * dot -Tpng -o out.png out.dot
   * </pre>
   *
   * <p>
   * Note: larger FSTs (a few thousand nodes) won't even
   * render, don't bother.
   *
   * @param sameRank
   *          If <code>true</code>, the resulting <code>dot</code> file will try
   *          to order states in layers of breadth-first traversal. This may
   *          mess up arcs, but makes the output FST's structure a bit clearer.
   *
   * @param labelStates
   *          If <code>true</code> states will have labels equal to their
   * offsets in their binary format. Expands the graph considerably.
   *
   * @see <a href="http://www.graphviz.org/">graphviz project</a>
   */
  template <typename T>
  static void toDot(std::shared_ptr<FST<T>> fst, std::shared_ptr<Writer> out,
                    bool sameRank, bool labelStates) ;

  /**
   * Emit a single state in the <code>dot</code> language.
   */
private:
  static void emitDotState(std::shared_ptr<Writer> out,
                           const std::wstring &name, const std::wstring &shape,
                           const std::wstring &color,
                           const std::wstring &label) ;

  /**
   * Ensures an arc's label is indeed printable (dot uses US-ASCII).
   */
  static std::wstring printableLabel(int label);

  /** Just maps each UTF16 unit (char) to the ints in an
   *  IntsRef. */
public:
  static std::shared_ptr<IntsRef>
  toUTF16(std::shared_ptr<std::wstring> s,
          std::shared_ptr<IntsRefBuilder> scratch);

  /** Decodes the Unicode codepoints from the provided
   *  std::wstring and places them in the provided scratch
   *  IntsRef, which must not be null, returning it. */
  static std::shared_ptr<IntsRef>
  toUTF32(std::shared_ptr<std::wstring> s,
          std::shared_ptr<IntsRefBuilder> scratch);

  /** Decodes the Unicode codepoints from the provided
   *  char[] and places them in the provided scratch
   *  IntsRef, which must not be null, returning it. */
  static std::shared_ptr<IntsRef>
  toUTF32(std::deque<wchar_t> &s, int offset, int length,
          std::shared_ptr<IntsRefBuilder> scratch);

  /** Just takes unsigned byte values from the BytesRef and
   *  converts into an IntsRef. */
  static std::shared_ptr<IntsRef>
  toIntsRef(std::shared_ptr<BytesRef> input,
            std::shared_ptr<IntsRefBuilder> scratch);

  /** Just converts IntsRef to BytesRef; you must ensure the
   *  int values fit into a byte. */
  static std::shared_ptr<BytesRef>
  toBytesRef(std::shared_ptr<IntsRef> input,
             std::shared_ptr<BytesRefBuilder> scratch);

  // Uncomment for debugging:

  /*
  public static <T> void dotToFile(FST<T> fst, std::wstring filePath) throws
  IOException { Writer w = new OutputStreamWriter(new
  FileOutputStream(filePath)); toDot(fst, w, true, true); w.close();
  }
  */

  /**
   * Reads the first arc greater or equal that the given label into the provided
   * arc in place and returns it iff found, otherwise return <code>null</code>.
   *
   * @param label the label to ceil on
   * @param fst the fst to operate on
   * @param follow the arc to follow reading the label from
   * @param arc the arc to read into in place
   * @param in the fst's {@link BytesReader}
   */
  template <typename T>
  static std::shared_ptr<Arc<T>>
  readCeilArc(int label, std::shared_ptr<FST<T>> fst,
              std::shared_ptr<Arc<T>> follow, std::shared_ptr<Arc<T>> arc,
              std::shared_ptr<BytesReader> in_) ;
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
