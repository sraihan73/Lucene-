#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
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
namespace org::apache::lucene::util::fst
{

/** Can next() and advance() through the terms in an FST
 *
 * @lucene.experimental
 */

template <typename T>
class FSTEnum : public std::enable_shared_from_this<FSTEnum>
{
  GET_CLASS_NAME(FSTEnum)
protected:
  const std::shared_ptr<FST<T>> fst;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) protected
  // FST.Arc<T>[] arcs = new FST.Arc[10];
  std::deque<FST::Arc<T>> arcs = std::deque<std::shared_ptr<FST::Arc>>(10);
  // outputs are cumulative
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) protected T[]
  // output = (T[]) new Object[10];
  std::deque<T> output =
      static_cast<std::deque<T>>(std::deque<std::any>(10));

  const T NO_OUTPUT;
  const std::shared_ptr<FST::BytesReader> fstReader;
  const std::shared_ptr<FST::Arc<T>> scratchArc =
      std::make_shared<FST::Arc<T>>();

  int upto = 0;
  int targetLength = 0;

  /** doFloor controls the behavior of advance: if it's true
   *  doFloor is true, advance positions to the biggest
   *  term before target.  */
  FSTEnum(std::shared_ptr<FST<T>> fst)
      : fst(fst), NO_OUTPUT(fst->outputs.getNoOutput()),
        fstReader(fst->getBytesReader())
  {
    fst->getFirstArc(getArc(0));
    output[0] = NO_OUTPUT;
  }

  virtual int getTargetLabel() = 0;
  virtual int getCurrentLabel() = 0;

  virtual void setCurrentLabel(int label) = 0;
  virtual void grow() = 0;

  /** Rewinds enum state to match the shared prefix between
   *  current term and target term */
  void rewindPrefix() 
  {
    if (upto == 0) {
      // System.out.println("  init");
      upto = 1;
      fst->readFirstTargetArc(getArc(0), getArc(1), fstReader);
      return;
    }
    // System.out.println("  rewind upto=" + upto + " vs targetLength=" +
    // targetLength);

    constexpr int currentLimit = upto;
    upto = 1;
    while (upto < currentLimit && upto <= targetLength + 1) {
      constexpr int cmp = getCurrentLabel() - getTargetLabel();
      if (cmp < 0) {
        // seek forward
        // System.out.println("    seek fwd");
        break;
      } else if (cmp > 0) {
        // seek backwards -- reset this arc to the first arc
        std::shared_ptr<FST::Arc<T>> *const arc = getArc(upto);
        fst->readFirstTargetArc(getArc(upto - 1), arc, fstReader);
        // System.out.println("    seek first arc");
        break;
      }
      upto++;
    }
    // System.out.println("  fall through upto=" + upto);
  }

  virtual void doNext() 
  {
    // System.out.println("FE: next upto=" + upto);
    if (upto == 0) {
      // System.out.println("  init");
      upto = 1;
      fst->readFirstTargetArc(getArc(0), getArc(1), fstReader);
    } else {
      // pop
      // System.out.println("  check pop curArc target=" + arcs[upto].target + "
      // label=" + arcs[upto].label + " isLast?=" + arcs[upto].isLast());
      while (arcs[upto]->isLast()) {
        upto--;
        if (upto == 0) {
          // System.out.println("  eof");
          return;
        }
      }
      fst->readNextArc(arcs[upto], fstReader);
    }

    pushFirst();
  }

  // TODO: should we return a status here (SEEK_FOUND / SEEK_NOT_FOUND /
  // SEEK_END)?  saves the eq check above?

  /** Seeks to smallest term that's &gt;= target. */
  virtual void doSeekCeil() 
  {

    // System.out.println("    advance len=" + target.length + " curlen=" +
    // current.length);

    // TODO: possibly caller could/should provide common
    // prefix length?  ie this work may be redundant if
    // caller is in fact intersecting against its own
    // automaton

    // System.out.println("FE.seekCeil upto=" + upto);

    // Save time by starting at the end of the shared prefix
    // b/w our current term & the target:
    rewindPrefix();
    // System.out.println("  after rewind upto=" + upto);

    std::shared_ptr<FST::Arc<T>> arc = getArc(upto);
    int targetLabel = getTargetLabel();
    // System.out.println("  init targetLabel=" + targetLabel);

    // Now scan forward, matching the new suffix of the target
    while (true) {

      // System.out.println("  cycle upto=" + upto + " arc.label=" + arc.label +
      // " (" + (char) arc.label + ") vs targetLabel=" + targetLabel);

      if (arc->bytesPerArc != 0 && arc->label != -1) {

        // Arcs are fixed array -- use binary search to find
        // the target.

        std::shared_ptr<FST::BytesReader> *const in_ = fst->getBytesReader();
        int low = arc->arcIdx;
        int high = arc->numArcs - 1;
        int mid = 0;
        // System.out.println("do arc array low=" + low + " high=" + high + "
        // targetLabel=" + targetLabel);
        bool found = false;
        while (low <= high) {
          mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
          in_->setPosition(arc->posArcsStart);
          in_->skipBytes(arc->bytesPerArc * mid + 1);
          constexpr int midLabel = fst->readLabel(in_);
          constexpr int cmp = midLabel - targetLabel;
          // System.out.println("  cycle low=" + low + " high=" + high + " mid="
          // + mid + " midLabel=" + midLabel + " cmp=" + cmp);
          if (cmp < 0) {
            low = mid + 1;
          } else if (cmp > 0) {
            high = mid - 1;
          } else {
            found = true;
            break;
          }
        }

        // NOTE: this code is dup'd w/ the code below (in
        // the outer else clause):
        if (found) {
          // Match
          arc->arcIdx = mid - 1;
          fst->readNextRealArc(arc, in_);
          assert(arc->arcIdx == mid);
          assert((arc->label == targetLabel,
                  L"arc.label=" + std::to_wstring(arc->label) +
                      L" vs targetLabel=" + std::to_wstring(targetLabel) +
                      L" mid=" + std::to_wstring(mid)));
          output[upto] = fst->outputs->add(output[upto - 1], arc->output);
          if (targetLabel == FST::END_LABEL) {
            return;
          }
          setCurrentLabel(arc->label);
          incr();
          arc = fst->readFirstTargetArc(arc, getArc(upto), fstReader);
          targetLabel = getTargetLabel();
          continue;
        } else if (low == arc->numArcs) {
          // Dead end
          arc->arcIdx = arc->numArcs - 2;
          fst->readNextRealArc(arc, in_);
          assert(arc->isLast());
          // Dead end (target is after the last arc);
          // rollback to last fork then push
          upto--;
          while (true) {
            if (upto == 0) {
              return;
            }
            std::shared_ptr<FST::Arc<T>> *const prevArc = getArc(upto);
            // System.out.println("  rollback upto=" + upto + " arc.label=" +
            // prevArc.label + " isLast?=" + prevArc.isLast());
            if (!prevArc->isLast()) {
              fst->readNextArc(prevArc, fstReader);
              pushFirst();
              return;
            }
            upto--;
          }
        } else {
          arc->arcIdx = (low > high ? low : high) - 1;
          fst->readNextRealArc(arc, in_);
          assert(arc->label > targetLabel);
          pushFirst();
          return;
        }
      } else {
        // Arcs are not array'd -- must do linear scan:
        if (arc->label == targetLabel) {
          // recurse
          output[upto] = fst->outputs->add(output[upto - 1], arc->output);
          if (targetLabel == FST::END_LABEL) {
            return;
          }
          setCurrentLabel(arc->label);
          incr();
          arc = fst->readFirstTargetArc(arc, getArc(upto), fstReader);
          targetLabel = getTargetLabel();
        } else if (arc->label > targetLabel) {
          pushFirst();
          return;
        } else if (arc->isLast()) {
          // Dead end (target is after the last arc);
          // rollback to last fork then push
          upto--;
          while (true) {
            if (upto == 0) {
              return;
            }
            std::shared_ptr<FST::Arc<T>> *const prevArc = getArc(upto);
            // System.out.println("  rollback upto=" + upto + " arc.label=" +
            // prevArc.label + " isLast?=" + prevArc.isLast());
            if (!prevArc->isLast()) {
              fst->readNextArc(prevArc, fstReader);
              pushFirst();
              return;
            }
            upto--;
          }
        } else {
          // keep scanning
          // System.out.println("    next scan");
          fst->readNextArc(arc, fstReader);
        }
      }
    }
  }

  // TODO: should we return a status here (SEEK_FOUND / SEEK_NOT_FOUND /
  // SEEK_END)?  saves the eq check above?
  /** Seeks to largest term that's &lt;= target. */
  virtual void doSeekFloor() 
  {

    // TODO: possibly caller could/should provide common
    // prefix length?  ie this work may be redundant if
    // caller is in fact intersecting against its own
    // automaton
    // System.out.println("FE: seek floor upto=" + upto);

    // Save CPU by starting at the end of the shared prefix
    // b/w our current term & the target:
    rewindPrefix();

    // System.out.println("FE: after rewind upto=" + upto);

    std::shared_ptr<FST::Arc<T>> arc = getArc(upto);
    int targetLabel = getTargetLabel();

    // System.out.println("FE: init targetLabel=" + targetLabel);

    // Now scan forward, matching the new suffix of the target
    while (true) {
      // System.out.println("  cycle upto=" + upto + " arc.label=" + arc.label +
      // " (" + (char) arc.label + ") targetLabel=" + targetLabel + " isLast?=" +
      // arc.isLast() + " bba=" + arc.bytesPerArc);

      if (arc->bytesPerArc != 0 && arc->label != FST::END_LABEL) {
        // Arcs are fixed array -- use binary search to find
        // the target.

        std::shared_ptr<FST::BytesReader> *const in_ = fst->getBytesReader();
        int low = arc->arcIdx;
        int high = arc->numArcs - 1;
        int mid = 0;
        // System.out.println("do arc array low=" + low + " high=" + high + "
        // targetLabel=" + targetLabel);
        bool found = false;
        while (low <= high) {
          mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
          in_->setPosition(arc->posArcsStart);
          in_->skipBytes(arc->bytesPerArc * mid + 1);
          constexpr int midLabel = fst->readLabel(in_);
          constexpr int cmp = midLabel - targetLabel;
          // System.out.println("  cycle low=" + low + " high=" + high + " mid="
          // + mid + " midLabel=" + midLabel + " cmp=" + cmp);
          if (cmp < 0) {
            low = mid + 1;
          } else if (cmp > 0) {
            high = mid - 1;
          } else {
            found = true;
            break;
          }
        }

        // NOTE: this code is dup'd w/ the code below (in
        // the outer else clause):
        if (found) {
          // Match -- recurse
          // System.out.println("  match!  arcIdx=" + mid);
          arc->arcIdx = mid - 1;
          fst->readNextRealArc(arc, in_);
          assert(arc->arcIdx == mid);
          assert((arc->label == targetLabel,
                  L"arc.label=" + std::to_wstring(arc->label) +
                      L" vs targetLabel=" + std::to_wstring(targetLabel) +
                      L" mid=" + std::to_wstring(mid)));
          output[upto] = fst->outputs->add(output[upto - 1], arc->output);
          if (targetLabel == FST::END_LABEL) {
            return;
          }
          setCurrentLabel(arc->label);
          incr();
          arc = fst->readFirstTargetArc(arc, getArc(upto), fstReader);
          targetLabel = getTargetLabel();
          continue;
        } else if (high == -1) {
          // System.out.println("  before first");
          // Very first arc is after our target
          // TODO: if each arc could somehow read the arc just
          // before, we can save this re-scan.  The ceil case
          // doesn't need this because it reads the next arc
          // instead:
          while (true) {
            // First, walk backwards until we find a first arc
            // that's before our target label:
            fst->readFirstTargetArc(getArc(upto - 1), arc, fstReader);
            if (arc->label < targetLabel) {
              // Then, scan forwards to the arc just before
              // the targetLabel:
              while (!arc->isLast() &&
                     fst->readNextArcLabel(arc, in_) < targetLabel) {
                fst->readNextArc(arc, fstReader);
              }
              pushLast();
              return;
            }
            upto--;
            if (upto == 0) {
              return;
            }
            targetLabel = getTargetLabel();
            arc = getArc(upto);
          }
        } else {
          // There is a floor arc:
          arc->arcIdx = (low > high ? high : low) - 1;
          // System.out.println(" hasFloor arcIdx=" + (arc.arcIdx+1));
          fst->readNextRealArc(arc, in_);
          assert(
              (arc->isLast() || fst->readNextArcLabel(arc, in_) > targetLabel));
          assert((arc->label < targetLabel,
                  L"arc.label=" + std::to_wstring(arc->label) +
                      L" vs targetLabel=" + std::to_wstring(targetLabel)));
          pushLast();
          return;
        }
      } else {

        if (arc->label == targetLabel) {
          // Match -- recurse
          output[upto] = fst->outputs->add(output[upto - 1], arc->output);
          if (targetLabel == FST::END_LABEL) {
            return;
          }
          setCurrentLabel(arc->label);
          incr();
          arc = fst->readFirstTargetArc(arc, getArc(upto), fstReader);
          targetLabel = getTargetLabel();
        } else if (arc->label > targetLabel) {
          // TODO: if each arc could somehow read the arc just
          // before, we can save this re-scan.  The ceil case
          // doesn't need this because it reads the next arc
          // instead:
          while (true) {
            // First, walk backwards until we find a first arc
            // that's before our target label:
            fst->readFirstTargetArc(getArc(upto - 1), arc, fstReader);
            if (arc->label < targetLabel) {
              // Then, scan forwards to the arc just before
              // the targetLabel:
              while (!arc->isLast() &&
                     fst->readNextArcLabel(arc, fstReader) < targetLabel) {
                fst->readNextArc(arc, fstReader);
              }
              pushLast();
              return;
            }
            upto--;
            if (upto == 0) {
              return;
            }
            targetLabel = getTargetLabel();
            arc = getArc(upto);
          }
        } else if (!arc->isLast()) {
          // System.out.println("  check next label=" +
          // fst.readNextArcLabel(arc) + " (" + (char) fst.readNextArcLabel(arc)
          // + ")");
          if (fst->readNextArcLabel(arc, fstReader) > targetLabel) {
            pushLast();
            return;
          } else {
            // keep scanning
            fst->readNextArc(arc, fstReader);
          }
        } else {
          pushLast();
          return;
        }
      }
    }
  }

  /** Seeks to exactly target term. */
  virtual bool doSeekExact() 
  {

    // TODO: possibly caller could/should provide common
    // prefix length?  ie this work may be redundant if
    // caller is in fact intersecting against its own
    // automaton

    // System.out.println("FE: seek exact upto=" + upto);

    // Save time by starting at the end of the shared prefix
    // b/w our current term & the target:
    rewindPrefix();

    // System.out.println("FE: after rewind upto=" + upto);
    std::shared_ptr<FST::Arc<T>> arc = getArc(upto - 1);
    int targetLabel = getTargetLabel();

    std::shared_ptr<FST::BytesReader> *const fstReader = fst->getBytesReader();

    while (true) {
      // System.out.println("  cycle target=" + (targetLabel == -1 ? "-1" :
      // (char) targetLabel));
      std::shared_ptr<FST::Arc<T>> *const nextArc =
          fst->findTargetArc(targetLabel, arc, getArc(upto), fstReader);
      if (nextArc == nullptr) {
        // short circuit
        // upto--;
        // upto = 0;
        fst->readFirstTargetArc(arc, getArc(upto), fstReader);
        // System.out.println("  no match upto=" + upto);
        return false;
      }
      // Match -- recurse:
      output[upto] = fst->outputs->add(output[upto - 1], nextArc->output);
      if (targetLabel == FST::END_LABEL) {
        // System.out.println("  return found; upto=" + upto + " output=" +
        // output[upto] + " nextArc=" + nextArc.isLast());
        return true;
      }
      setCurrentLabel(targetLabel);
      incr();
      targetLabel = getTargetLabel();
      arc = nextArc;
    }
  }

private:
  void incr()
  {
    upto++;
    grow();
    if (arcs.size() <= upto) {
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
      // FST.Arc<T>[] newArcs = new
      // FST.Arc[org.apache.lucene.util.ArrayUtil.oversize(1+upto,
      // org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF)];
      std::deque<FST::Arc<T>> newArcs =
          std::deque<std::shared_ptr<FST::Arc>>(ArrayUtil::oversize(
              1 + upto, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
      System::arraycopy(arcs, 0, newArcs, 0, arcs.size());
      arcs = newArcs;
    }
    if (output.size() <= upto) {
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final T[]
      // newOutput = (T[]) new
      // Object[org.apache.lucene.util.ArrayUtil.oversize(1+upto,
      // org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF)];
      const std::deque<T> newOutput =
          static_cast<std::deque<T>>(std::deque<std::any>(ArrayUtil::oversize(
              1 + upto, RamUsageEstimator::NUM_BYTES_OBJECT_REF)));
      System::arraycopy(output, 0, newOutput, 0, output.size());
      output = newOutput;
    }
  }

  // Appends current arc, and then recurses from its target,
  // appending first arc all the way to the final node
  void pushFirst() 
  {

    std::shared_ptr<FST::Arc<T>> arc = arcs[upto];
    assert(arc != nullptr);

    while (true) {
      output[upto] = fst->outputs->add(output[upto - 1], arc->output);
      if (arc->label == FST::END_LABEL) {
        // Final node
        break;
      }
      // System.out.println("  pushFirst label=" + (char) arc.label + " upto=" +
      // upto + " output=" + fst.outputs.outputToString(output[upto]));
      setCurrentLabel(arc->label);
      incr();

      std::shared_ptr<FST::Arc<T>> *const nextArc = getArc(upto);
      fst->readFirstTargetArc(arc, nextArc, fstReader);
      arc = nextArc;
    }
  }

  // Recurses from current arc, appending last arc all the
  // way to the first final node
  void pushLast() 
  {

    std::shared_ptr<FST::Arc<T>> arc = arcs[upto];
    assert(arc != nullptr);

    while (true) {
      setCurrentLabel(arc->label);
      output[upto] = fst->outputs->add(output[upto - 1], arc->output);
      if (arc->label == FST::END_LABEL) {
        // Final node
        break;
      }
      incr();

      arc = fst->readLastTargetArc(arc, getArc(upto), fstReader);
    }
  }

  std::shared_ptr<FST::Arc<T>> getArc(int idx)
  {
    if (arcs[idx] == nullptr) {
      arcs[idx] = std::make_shared<FST::Arc<T>>();
    }
    return arcs[idx];
  }
};

} // namespace org::apache::lucene::util::fst
