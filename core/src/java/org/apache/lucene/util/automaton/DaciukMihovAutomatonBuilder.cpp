using namespace std;

#include "DaciukMihovAutomatonBuilder.h"

namespace org::apache::lucene::util::automaton
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

// C++ TODO: No base class can be determined:
DaciukMihovAutomatonBuilder::DaciukMihovAutomatonBuilder() {}

std::deque<int> const DaciukMihovAutomatonBuilder::State::NO_LABELS =
    std::deque<int>(0);
std::deque<std::shared_ptr<State>> const
    DaciukMihovAutomatonBuilder::State::NO_STATES =
        std::deque<std::shared_ptr<State>>(0);

shared_ptr<State> DaciukMihovAutomatonBuilder::State::getState(int label)
{
  constexpr int index = Arrays::binarySearch(labels, label);
  return index >= 0 ? states[index] : nullptr;
}

bool DaciukMihovAutomatonBuilder::State::equals(any obj)
{
  shared_ptr<State> *const other = any_cast<std::shared_ptr<State>>(obj);
  return is_final == other->is_final &&
         Arrays::equals(this->labels, other->labels) &&
         referenceEquals(this->states, other->states);
}

int DaciukMihovAutomatonBuilder::State::hashCode()
{
  int hash = is_final ? 1 : 0;

  hash ^= hash * 31 + this->labels.size();
  for (auto c : this->labels) {
    hash ^= hash * 31 + c;
  }

  /*
   * Compare the right-language of this state using reference-identity of
   * outgoing states. This is possible because states are interned (stored
   * in registry) and traversed in post-order, so any outgoing transitions
   * are already interned.
   */
  for (auto s : this->states) {
    hash ^= System::identityHashCode(s);
  }

  return hash;
}

bool DaciukMihovAutomatonBuilder::State::hasChildren()
{
  return labels.size() > 0;
}

shared_ptr<State> DaciukMihovAutomatonBuilder::State::newState(int label)
{
  assert((Arrays::binarySearch(labels, label) < 0,
          L"State already has transition labeled: "));
  +label;

  labels = Arrays::copyOf(labels, labels.size() + 1);
  states = Arrays::copyOf(states, states.size() + 1);

  labels[labels.size() - 1] = label;
  return states[states.size() - 1] = make_shared<State>();
}

shared_ptr<State> DaciukMihovAutomatonBuilder::State::lastChild()
{
  assert((hasChildren(), L"No outgoing transitions."));
  return states[states.size() - 1];
}

shared_ptr<State> DaciukMihovAutomatonBuilder::State::lastChild(int label)
{
  constexpr int index = labels.size() - 1;
  shared_ptr<State> s = nullptr;
  if (index >= 0 && labels[index] == label) {
    s = states[index];
  }
  assert(s == getState(label));
  return s;
}

void DaciukMihovAutomatonBuilder::State::replaceLastChild(
    shared_ptr<State> state)
{
  assert((hasChildren(), L"No outgoing transitions."));
  states[states.size() - 1] = state;
}

bool DaciukMihovAutomatonBuilder::State::referenceEquals(std::deque<any> &a1,
                                                         std::deque<any> &a2)
{
  if (a1.size() != a2.size()) {
    return false;
  }

  for (int i = 0; i < a1.size(); i++) {
    if (a1[i] != a2[i]) {
      return false;
    }
  }

  return true;
}

const shared_ptr<
    Comparator<std::shared_ptr<org::apache::lucene::util::CharsRef>>>
    DaciukMihovAutomatonBuilder::comparator =
        org::apache::lucene::util::CharsRef::getUTF16SortedAsUTF8Comparator();

void DaciukMihovAutomatonBuilder::add(shared_ptr<CharsRef> current)
{
  assert((stateRegistry.size() > 0, L"Automaton already built."));
  assert((previous == nullptr || comparator->compare(previous, current) <= 0,
          L"Input must be in sorted UTF-8 order: "));
  +previous + L" >= " + current;
  assert(setPrevious(current));

  // Descend in the automaton (find matching prefix).
  int pos = 0, max = current->length();
  shared_ptr<State> next, state = root;
  while (pos < max && (next = state->lastChild(
                           Character::codePointAt(current, pos))) != nullptr) {
    state = next;
    // todo, optimize me
    pos += Character::charCount(Character::codePointAt(current, pos));
  }

  if (state->hasChildren()) {
    replaceOrRegister(state);
  }

  addSuffix(state, current, pos);
}

shared_ptr<State> DaciukMihovAutomatonBuilder::complete()
{
  if (this->stateRegistry.empty()) {
    throw make_shared<IllegalStateException>();
  }

  if (root->hasChildren()) {
    replaceOrRegister(root);
  }

  stateRegistry.clear();
  return root;
}

int DaciukMihovAutomatonBuilder::convert(
    shared_ptr<Automaton::Builder> a, shared_ptr<State> s,
    shared_ptr<IdentityHashMap<std::shared_ptr<State>, int>> visited)
{

  optional<int> converted = visited->get(s);
  if (converted) {
    return converted;
  }

  converted = a->createState();
  a->setAccept(converted, s->is_final);

  visited->put(s, converted);
  int i = 0;
  std::deque<int> labels = s->labels;
  for (auto target : s->states) {
    a->addTransition(converted, convert(a, target, visited), labels[i++]);
  }

  return converted;
}

shared_ptr<Automaton> DaciukMihovAutomatonBuilder::build(
    shared_ptr<deque<std::shared_ptr<BytesRef>>> input)
{
  shared_ptr<DaciukMihovAutomatonBuilder> *const builder =
      make_shared<DaciukMihovAutomatonBuilder>();

  std::deque<wchar_t> chars(0);
  shared_ptr<CharsRef> ref = make_shared<CharsRef>();
  for (auto b : input) {
    chars = ArrayUtil::grow(chars, b->length);
    constexpr int len = UnicodeUtil::UTF8toUTF16(b, chars);
    ref->chars = chars;
    ref->length_ = len;
    builder->add(ref);
  }

  shared_ptr<Automaton::Builder> a = make_shared<Automaton::Builder>();
  convert(a, builder->complete(),
          make_shared<IdentityHashMap<std::shared_ptr<State>, int>>());

  return a->finish();
}

bool DaciukMihovAutomatonBuilder::setPrevious(shared_ptr<CharsRef> current)
{
  // don't need to copy, once we fix
  // https://issues.apache.org/jira/browse/LUCENE-3277 still, called only from
  // assert
  previous = CharsRef::deepCopyOf(current);
  return true;
}

void DaciukMihovAutomatonBuilder::replaceOrRegister(shared_ptr<State> state)
{
  shared_ptr<State> *const child = state->lastChild();

  if (child->hasChildren()) {
    replaceOrRegister(child);
  }

  shared_ptr<State> *const registered = stateRegistry[child];
  if (registered != nullptr) {
    state->replaceLastChild(registered);
  } else {
    stateRegistry.emplace(child, child);
  }
}

void DaciukMihovAutomatonBuilder::addSuffix(shared_ptr<State> state,
                                            shared_ptr<std::wstring> current,
                                            int fromIndex)
{
  constexpr int len = current->length();
  while (fromIndex < len) {
    int cp = Character::codePointAt(current, fromIndex);
    state = state->newState(cp);
    fromIndex += Character::charCount(cp);
  }
  state->is_final = true;
}
} // namespace org::apache::lucene::util::automaton