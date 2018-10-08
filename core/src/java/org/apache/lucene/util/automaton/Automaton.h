#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/Transition.h"

#include  "core/src/java/org/apache/lucene/util/Sorter.h"

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
namespace org::apache::lucene::util::automaton
{

// import java.io.IOException;
// import java.io.PrintWriter;

using Accountable = org::apache::lucene::util::Accountable;

// TODO
//   - could use packed int arrays instead
//   - could encode dest w/ delta from to?

/** Represents an automaton and all its states and transitions.  States
 *  are integers and must be created using {@link #createState}.  Mark a
 *  state as an accept state using {@link #setAccept}.  Add transitions
 *  using {@link #addTransition}.  Each state must have all of its
 *  transitions added at once; if this is too restrictive then use
 *  {@link Automaton.Builder} instead.  State 0 is always the
 *  initial state.  Once a state is finished, either
 *  because you've starting adding transitions to another state or you
 *  call {@link #finishState}, then that states transitions are sorted
 *  (first by min, then max, then dest) and reduced (transitions with
 *  adjacent labels going to the same dest are combined).
 *
 * @lucene.experimental */

class Automaton : public std::enable_shared_from_this<Automaton>,
                  public Accountable
{
  GET_CLASS_NAME(Automaton)

  /** Where we next write to the int[] states; this increments by 2 for
   *  each added state because we pack a pointer to the transitions
   *  array and a count of how many transitions leave the state.  */
private:
  int nextState = 0;

  /** Where we next write to in int[] transitions; this
   *  increments by 3 for each added transition because we
   *  pack min, max, dest in sequence. */
  int nextTransition = 0;

  /** Current state we are adding transitions to; the caller
   *  must add all transitions for this state before moving
   *  onto another state. */
  int curState = -1;

  /** Index in the transitions array, where this states
   *  leaving transitions are stored, or -1 if this state
   *  has not added any transitions yet, followed by number
   *  of transitions. */
  std::deque<int> states;

  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<BitSet> isAccept_;

  /** Holds toState, min, max for each transition. */
  std::deque<int> transitions;

  /** True if no state has two transitions leaving with the same label. */
  bool deterministic = true;

  /** Sole constructor; creates an automaton with no states. */
public:
  Automaton();

  /**
   * Constructor which creates an automaton with enough space for the given
   * number of states and transitions.
   *
   * @param numStates
   *           Number of states.
   * @param numTransitions
   *           Number of transitions.
   */
  Automaton(int numStates, int numTransitions);

  /** Create a new state. */
  virtual int createState();

  /** Set or clear this state as an accept state. */
  virtual void setAccept(int state, bool accept);

  /** Sugar to get all transitions for all states.  This is
   *  object-heavy; it's better to iterate state by state instead. */
  virtual std::deque<std::deque<std::shared_ptr<Transition>>>
  getSortedTransitions();

  /** Returns accept states.  If the bit is set then that state is an accept
   * state. */
  virtual std::shared_ptr<BitSet> getAcceptStates();

  /** Returns true if this state is an accept state. */
  virtual bool isAccept(int state);

  /** Add a new transition with min = max = label. */
  virtual void addTransition(int source, int dest, int label);

  /** Add a new transition with the specified source, dest, min, max. */
  virtual void addTransition(int source, int dest, int min, int max);

  /** Add a [virtual] epsilon transition between source and dest.
   *  Dest state must already have all transitions added because this
   *  method simply copies those same transitions over to source. */
  virtual void addEpsilon(int source, int dest);

  /** Copies over all states/transitions from other.  The states numbers
   *  are sequentially assigned (appended). */
  virtual void copy(std::shared_ptr<Automaton> other);

  /** Freezes the last state, sorting and reducing the transitions. */
private:
  void finishCurrentState();

  /** Returns true if this automaton is deterministic (for ever state
   *  there is only one transition for each label). */
public:
  virtual bool isDeterministic();

  /** Finishes the current state; call this once you are done adding
   *  transitions for a state.  This is automatically called if you
   *  start adding transitions to a new source state, but for the last
   *  state you add you need to this method yourself. */
  virtual void finishState();

  // TODO: add finish() to shrink wrap the arrays?

  /** How many states this automaton has. */
  virtual int getNumStates();

  /** How many transitions this automaton has. */
  virtual int getNumTransitions();

  /** How many transitions this state has. */
  virtual int getNumTransitions(int state);

private:
  void growStates();

  void growTransitions();

  /** Sorts transitions by dest, ascending, then min label ascending, then max
   * label ascending */
  const std::shared_ptr<Sorter> destMinMaxSorter =
      std::make_shared<InPlaceMergeSorterAnonymousInnerClass>();

private:
  class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
  public:
    InPlaceMergeSorterAnonymousInnerClass();

  private:
    void swapOne(int i, int j);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

  /** Sorts transitions by min label, ascending, then max label ascending, then
   * dest ascending */
private:
  const std::shared_ptr<Sorter> minMaxDestSorter =
      std::make_shared<InPlaceMergeSorterAnonymousInnerClass2>();

private:
  class InPlaceMergeSorterAnonymousInnerClass2 : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass2)
  public:
    InPlaceMergeSorterAnonymousInnerClass2();

  private:
    void swapOne(int i, int j);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass2>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

  /** Initialize the provided Transition to iterate through all transitions
   *  leaving the specified state.  You must call {@link #getNextTransition} to
   *  get each transition.  Returns the number of transitions
   *  leaving this state. */
public:
  virtual int initTransition(int state, std::shared_ptr<Transition> t);

  /** Iterate to the next transition after the provided one */
  virtual void getNextTransition(std::shared_ptr<Transition> t);

private:
  bool transitionSorted(std::shared_ptr<Transition> t);

  /** Fill the provided {@link Transition} with the index'th
   *  transition leaving the specified state. */
public:
  virtual void getTransition(int state, int index,
                             std::shared_ptr<Transition> t);

  static void appendCharString(int c, std::shared_ptr<StringBuilder> b);

  /*
  public void writeDot(std::wstring fileName) {
    if (fileName.indexOf('/') == -1) {
      fileName = "/l/la/lucene/core/" + fileName + ".dot";
    }
    try {
      PrintWriter pw = new PrintWriter(fileName);
      pw.println(toDot());
      pw.close();
    } catch (IOException ioe) {
      throw new RuntimeException(ioe);
    }
  }
  */

  /** Returns the dot (graphviz) representation of this automaton.
   *  This is extremely useful for visualizing the automaton. */
  virtual std::wstring toDot();

  /**
   * Returns sorted array of all interval start points.
   */
  virtual std::deque<int> getStartPoints();

  /**
   * Performs lookup in transitions, assuming determinism.
   *
   * @param state starting state
   * @param label codepoint to look up
   * @return destination state, -1 if no matching outgoing transition
   */
  virtual int step(int state, int label);

  /** Records new states and transitions and then {@link
   *  #finish} creates the {@link Automaton}.  Use this
   *  when you cannot create the Automaton directly because
   *  it's too restrictive to have to add all transitions
   *  leaving each state at once. */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    int nextState = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<BitSet> isAccept_;
    std::deque<int> transitions;
    int nextTransition = 0;

    /** Default constructor, pre-allocating for 16 states and transitions. */
  public:
    Builder();

    /**
     * Constructor which creates a builder with enough space for the given
     * number of states and transitions.
     *
     * @param numStates
     *           Number of states.
     * @param numTransitions
     *           Number of transitions.
     */
    Builder(int numStates, int numTransitions);

    /** Add a new transition with min = max = label. */
    virtual void addTransition(int source, int dest, int label);

    /** Add a new transition with the specified source, dest, min, max. */
    virtual void addTransition(int source, int dest, int min, int max);

    /** Add a [virtual] epsilon transition between source and dest.
     *  Dest state must already have all transitions added because this
     *  method simply copies those same transitions over to source. */
    virtual void addEpsilon(int source, int dest);

    /** Sorts transitions first then min label ascending, then
     *  max label ascending, then dest ascending */
  private:
    const std::shared_ptr<Sorter> sorter =
        std::make_shared<InPlaceMergeSorterAnonymousInnerClass3>();

  private:
    class InPlaceMergeSorterAnonymousInnerClass3 : public InPlaceMergeSorter
    {
      GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass3)
    public:
      InPlaceMergeSorterAnonymousInnerClass3();

    private:
      void swapOne(int i, int j);

    protected:
      void swap(int i, int j) override;

      int compare(int i, int j) override;

    protected:
      std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass3> shared_from_this()
      {
        return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass3>(
            org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
      }
    };

    /** Compiles all added states and transitions into a new {@code Automaton}
     *  and returns it. */
  public:
    virtual std::shared_ptr<Automaton> finish();

    /** Create a new state. */
    virtual int createState();

    /** Set or clear this state as an accept state. */
    virtual void setAccept(int state, bool accept);

    /** Returns true if this state is an accept state. */
    virtual bool isAccept(int state);

    /** How many states this automaton has. */
    virtual int getNumStates();

    /** Copies over all states/transitions from other. */
    virtual void copy(std::shared_ptr<Automaton> other);

    /** Copies over all states from other. */
    virtual void copyStates(std::shared_ptr<Automaton> other);
  };

public:
  int64_t ramBytesUsed() override;
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
