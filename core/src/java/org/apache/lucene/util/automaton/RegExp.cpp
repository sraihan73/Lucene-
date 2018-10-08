using namespace std;

#include "RegExp.h"

namespace org::apache::lucene::util::automaton
{

RegExp::RegExp() : originalString(L"") {}

RegExp::RegExp(const wstring &s)  {}

RegExp::RegExp(const wstring &s, int syntax_flags) 
    : originalString(s)
{
  flags = syntax_flags;
  shared_ptr<RegExp> e;
  if (s.length() == 0) {
    e = makeString(L"");
  } else {
    e = parseUnionExp();
    if (pos < originalString.length()) {
      throw invalid_argument(L"end-of-string expected at position " +
                             to_wstring(pos));
    }
  }
  kind = e->kind;
  exp1 = e->exp1;
  exp2 = e->exp2;
  this->s = e->s;
  c = e->c;
  min = e->min;
  max = e->max;
  digits = e->digits;
  from = e->from;
  to = e->to;
}

shared_ptr<Automaton> RegExp::toAutomaton()
{
  return toAutomaton(nullptr, nullptr,
                     Operations::DEFAULT_MAX_DETERMINIZED_STATES);
}

shared_ptr<Automaton> RegExp::toAutomaton(int maxDeterminizedStates) throw(
    invalid_argument, TooComplexToDeterminizeException)
{
  return toAutomaton(nullptr, nullptr, maxDeterminizedStates);
}

shared_ptr<Automaton> RegExp::toAutomaton(
    shared_ptr<AutomatonProvider> automaton_provider,
    int maxDeterminizedStates) throw(invalid_argument,
                                     TooComplexToDeterminizeException)
{
  return toAutomaton(nullptr, automaton_provider, maxDeterminizedStates);
}

shared_ptr<Automaton> RegExp::toAutomaton(
    unordered_map<wstring, std::shared_ptr<Automaton>> &automata,
    int maxDeterminizedStates) throw(invalid_argument,
                                     TooComplexToDeterminizeException)
{
  return toAutomaton(automata, nullptr, maxDeterminizedStates);
}

shared_ptr<Automaton> RegExp::toAutomaton(
    unordered_map<wstring, std::shared_ptr<Automaton>> &automata,
    shared_ptr<AutomatonProvider> automaton_provider,
    int maxDeterminizedStates) throw(invalid_argument,
                                     TooComplexToDeterminizeException)
{
  try {
    return toAutomatonInternal(automata, automaton_provider,
                               maxDeterminizedStates);
  } catch (const TooComplexToDeterminizeException &e) {
    throw make_shared<TooComplexToDeterminizeException>(shared_from_this(), e);
  }
}

shared_ptr<Automaton> RegExp::toAutomatonInternal(
    unordered_map<wstring, std::shared_ptr<Automaton>> &automata,
    shared_ptr<AutomatonProvider> automaton_provider,
    int maxDeterminizedStates) 
{
  deque<std::shared_ptr<Automaton>> deque;
  shared_ptr<Automaton> a = nullptr;
  switch (kind) {
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_UNION:
    deque = deque<>();
    findLeaves(exp1, Kind::REGEXP_UNION, deque, automata, automaton_provider,
               maxDeterminizedStates);
    findLeaves(exp2, Kind::REGEXP_UNION, deque, automata, automaton_provider,
               maxDeterminizedStates);
    a = Operations::union_(deque);
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CONCATENATION:
    deque = deque<>();
    findLeaves(exp1, Kind::REGEXP_CONCATENATION, deque, automata,
               automaton_provider, maxDeterminizedStates);
    findLeaves(exp2, Kind::REGEXP_CONCATENATION, deque, automata,
               automaton_provider, maxDeterminizedStates);
    a = Operations::concatenate(deque);
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERSECTION:
    a = Operations::intersection(
        exp1->toAutomatonInternal(automata, automaton_provider,
                                  maxDeterminizedStates),
        exp2->toAutomatonInternal(automata, automaton_provider,
                                  maxDeterminizedStates));
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_OPTIONAL:
    a = Operations::optional(exp1->toAutomatonInternal(
        automata, automaton_provider, maxDeterminizedStates));
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT:
    a = Operations::repeat(exp1->toAutomatonInternal(
        automata, automaton_provider, maxDeterminizedStates));
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MIN: {
    a = exp1->toAutomatonInternal(automata, automaton_provider,
                                  maxDeterminizedStates);
    int minNumStates = (a->getNumStates() - 1) * min;
    if (minNumStates > maxDeterminizedStates) {
      throw make_shared<TooComplexToDeterminizeException>(a, minNumStates);
    }
    a = Operations::repeat(a, min);
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  }
  case org::apache::lucene::util::automaton::RegExp::Kind::
      REGEXP_REPEAT_MINMAX: {
    a = exp1->toAutomatonInternal(automata, automaton_provider,
                                  maxDeterminizedStates);
    int minMaxNumStates = (a->getNumStates() - 1) * max;
    if (minMaxNumStates > maxDeterminizedStates) {
      throw make_shared<TooComplexToDeterminizeException>(a, minMaxNumStates);
    }
    a = Operations::repeat(a, min, max);
    break;
  }
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_COMPLEMENT:
    a = Operations::complement(exp1->toAutomatonInternal(automata,
                                                         automaton_provider,
                                                         maxDeterminizedStates),
                               maxDeterminizedStates);
    a = MinimizationOperations::minimize(a, maxDeterminizedStates);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CHAR:
    a = Automata::makeChar(c);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CHAR_RANGE:
    a = Automata::makeCharRange(from, to);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_ANYCHAR:
    a = Automata::makeAnyChar();
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_EMPTY:
    a = Automata::makeEmpty();
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_STRING:
    a = Automata::makeString(s);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_ANYSTRING:
    a = Automata::makeAnyString();
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_AUTOMATON: {
    shared_ptr<Automaton> aa = nullptr;
    if (automata.size() > 0) {
      aa = automata[s];
    }
    if (aa == nullptr && automaton_provider != nullptr) {
      try {
        aa = automaton_provider->getAutomaton(s);
      } catch (const IOException &e) {
        throw invalid_argument(e);
      }
    }
    if (aa == nullptr) {
      throw invalid_argument(L"'" + s + L"' not found");
    }
    a = aa;
    break;
  }
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERVAL:
    a = Automata::makeDecimalInterval(min, max, digits);
    break;
  }
  return a;
}

void RegExp::findLeaves(
    shared_ptr<RegExp> exp, Kind kind, deque<std::shared_ptr<Automaton>> &deque,
    unordered_map<wstring, std::shared_ptr<Automaton>> &automata,
    shared_ptr<AutomatonProvider> automaton_provider, int maxDeterminizedStates)
{
  if (exp->kind == kind) {
    findLeaves(exp->exp1, kind, deque, automata, automaton_provider,
               maxDeterminizedStates);
    findLeaves(exp->exp2, kind, deque, automata, automaton_provider,
               maxDeterminizedStates);
  } else {
    deque.push_back(exp->toAutomatonInternal(automata, automaton_provider,
                                            maxDeterminizedStates));
  }
}

wstring RegExp::getOriginalString() { return originalString; }

wstring RegExp::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  toStringBuilder(b);
  return b->toString();
}

void RegExp::toStringBuilder(shared_ptr<StringBuilder> b)
{
  switch (kind) {
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_UNION:
    b->append(L"(");
    exp1->toStringBuilder(b);
    b->append(L"|");
    exp2->toStringBuilder(b);
    b->append(L")");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CONCATENATION:
    exp1->toStringBuilder(b);
    exp2->toStringBuilder(b);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERSECTION:
    b->append(L"(");
    exp1->toStringBuilder(b);
    b->append(L"&");
    exp2->toStringBuilder(b);
    b->append(L")");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_OPTIONAL:
    b->append(L"(");
    exp1->toStringBuilder(b);
    b->append(L")?");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT:
    b->append(L"(");
    exp1->toStringBuilder(b);
    b->append(L")*");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MIN:
    b->append(L"(");
    exp1->toStringBuilder(b);
    b->append(L"){")->append(min)->append(L",}");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MINMAX:
    b->append(L"(");
    exp1->toStringBuilder(b);
    b->append(L"){")->append(min)->append(L",")->append(max)->append(L"}");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_COMPLEMENT:
    b->append(L"~(");
    exp1->toStringBuilder(b);
    b->append(L")");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CHAR:
    b->append(L"\\").appendCodePoint(c);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CHAR_RANGE:
    b->append(L"[\\")
        .appendCodePoint(from)
        ->append(L"-\\")
        .appendCodePoint(to)
        ->append(L"]");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_ANYCHAR:
    b->append(L".");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_EMPTY:
    b->append(L"#");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_STRING:
    b->append(L"\"")->append(s)->append(L"\"");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_ANYSTRING:
    b->append(L"@");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_AUTOMATON:
    b->append(L"<")->append(s)->append(L">");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERVAL: {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s1 = Integer::toString(min);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s2 = Integer::toString(max);
    b->append(L"<");
    if (digits > 0) {
      for (int i = s1.length(); i < digits; i++) {
        b->append(L'0');
      }
    }
    b->append(s1)->append(L"-");
    if (digits > 0) {
      for (int i = s2.length(); i < digits; i++) {
        b->append(L'0');
      }
    }
    b->append(s2)->append(L">");
    break;
  }
  }
}

wstring RegExp::toStringTree()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  toStringTree(b, L"");
  return b->toString();
}

void RegExp::toStringTree(shared_ptr<StringBuilder> b, const wstring &indent)
{
  switch (kind) {
  // binary
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_UNION:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CONCATENATION:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERSECTION:
    b->append(indent);
    b->append(kind);
    b->append(L'\n');
    exp1->toStringTree(b, indent + L"  ");
    exp2->toStringTree(b, indent + L"  ");
    break;
  // unary
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_OPTIONAL:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_COMPLEMENT:
    b->append(indent);
    b->append(kind);
    b->append(L'\n');
    exp1->toStringTree(b, indent + L"  ");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MIN:
    b->append(indent);
    b->append(kind);
    b->append(L" min=");
    b->append(min);
    b->append(L'\n');
    exp1->toStringTree(b, indent + L"  ");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MINMAX:
    b->append(indent);
    b->append(kind);
    b->append(L" min=");
    b->append(min);
    b->append(L" max=");
    b->append(max);
    b->append(L'\n');
    exp1->toStringTree(b, indent + L"  ");
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CHAR:
    b->append(indent);
    b->append(kind);
    b->append(L" char=");
    b->appendCodePoint(c);
    b->append(L'\n');
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CHAR_RANGE:
    b->append(indent);
    b->append(kind);
    b->append(L" from=");
    b->appendCodePoint(from);
    b->append(L" to=");
    b->appendCodePoint(to);
    b->append(L'\n');
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_ANYCHAR:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_EMPTY:
    b->append(indent);
    b->append(kind);
    b->append(L'\n');
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_STRING:
    b->append(indent);
    b->append(kind);
    b->append(L" string=");
    b->append(s);
    b->append(L'\n');
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_ANYSTRING:
    b->append(indent);
    b->append(kind);
    b->append(L'\n');
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_AUTOMATON:
    b->append(indent);
    b->append(kind);
    b->append(L'\n');
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERVAL: {
    b->append(indent);
    b->append(kind);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s1 = Integer::toString(min);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s2 = Integer::toString(max);
    b->append(L"<");
    if (digits > 0) {
      for (int i = s1.length(); i < digits; i++) {
        b->append(L'0');
      }
    }
    b->append(s1)->append(L"-");
    if (digits > 0) {
      for (int i = s2.length(); i < digits; i++) {
        b->append(L'0');
      }
    }
    b->append(s2)->append(L">");
    b->append(L'\n');
    break;
  }
  }
}

shared_ptr<Set<wstring>> RegExp::getIdentifiers()
{
  unordered_set<wstring> set = unordered_set<wstring>();
  getIdentifiers(set);
  return set;
}

void RegExp::getIdentifiers(shared_ptr<Set<wstring>> set)
{
  switch (kind) {
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_UNION:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_CONCATENATION:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_INTERSECTION:
    exp1->getIdentifiers(set);
    exp2->getIdentifiers(set);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_OPTIONAL:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MIN:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_REPEAT_MINMAX:
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_COMPLEMENT:
    exp1->getIdentifiers(set);
    break;
  case org::apache::lucene::util::automaton::RegExp::Kind::REGEXP_AUTOMATON:
    set->add(s);
    break;
  default:
  }
}

shared_ptr<RegExp> RegExp::makeUnion(shared_ptr<RegExp> exp1,
                                     shared_ptr<RegExp> exp2)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_UNION;
  r->exp1 = exp1;
  r->exp2 = exp2;
  return r;
}

shared_ptr<RegExp> RegExp::makeConcatenation(shared_ptr<RegExp> exp1,
                                             shared_ptr<RegExp> exp2)
{
  if ((exp1->kind == Kind::REGEXP_CHAR || exp1->kind == Kind::REGEXP_STRING) &&
      (exp2->kind == Kind::REGEXP_CHAR || exp2->kind == Kind::REGEXP_STRING)) {
    return makeString(exp1, exp2);
  }
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_CONCATENATION;
  if (exp1->kind == Kind::REGEXP_CONCATENATION &&
      (exp1->exp2->kind == Kind::REGEXP_CHAR ||
       exp1->exp2->kind == Kind::REGEXP_STRING) &&
      (exp2->kind == Kind::REGEXP_CHAR || exp2->kind == Kind::REGEXP_STRING)) {
    r->exp1 = exp1->exp1;
    r->exp2 = makeString(exp1->exp2, exp2);
  } else if ((exp1->kind == Kind::REGEXP_CHAR ||
              exp1->kind == Kind::REGEXP_STRING) &&
             exp2->kind == Kind::REGEXP_CONCATENATION &&
             (exp2->exp1->kind == Kind::REGEXP_CHAR ||
              exp2->exp1->kind == Kind::REGEXP_STRING)) {
    r->exp1 = makeString(exp1, exp2->exp1);
    r->exp2 = exp2->exp2;
  } else {
    r->exp1 = exp1;
    r->exp2 = exp2;
  }
  return r;
}

shared_ptr<RegExp> RegExp::makeString(shared_ptr<RegExp> exp1,
                                      shared_ptr<RegExp> exp2)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  if (exp1->kind == Kind::REGEXP_STRING) {
    b->append(exp1->s);
  } else {
    b->appendCodePoint(exp1->c);
  }
  if (exp2->kind == Kind::REGEXP_STRING) {
    b->append(exp2->s);
  } else {
    b->appendCodePoint(exp2->c);
  }
  return makeString(b->toString());
}

shared_ptr<RegExp> RegExp::makeIntersection(shared_ptr<RegExp> exp1,
                                            shared_ptr<RegExp> exp2)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_INTERSECTION;
  r->exp1 = exp1;
  r->exp2 = exp2;
  return r;
}

shared_ptr<RegExp> RegExp::makeOptional(shared_ptr<RegExp> exp)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_OPTIONAL;
  r->exp1 = exp;
  return r;
}

shared_ptr<RegExp> RegExp::makeRepeat(shared_ptr<RegExp> exp)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_REPEAT;
  r->exp1 = exp;
  return r;
}

shared_ptr<RegExp> RegExp::makeRepeat(shared_ptr<RegExp> exp, int min)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_REPEAT_MIN;
  r->exp1 = exp;
  r->min = min;
  return r;
}

shared_ptr<RegExp> RegExp::makeRepeat(shared_ptr<RegExp> exp, int min, int max)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_REPEAT_MINMAX;
  r->exp1 = exp;
  r->min = min;
  r->max = max;
  return r;
}

shared_ptr<RegExp> RegExp::makeComplement(shared_ptr<RegExp> exp)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_COMPLEMENT;
  r->exp1 = exp;
  return r;
}

shared_ptr<RegExp> RegExp::makeChar(int c)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_CHAR;
  r->c = c;
  return r;
}

shared_ptr<RegExp> RegExp::makeCharRange(int from, int to)
{
  if (from > to) {
    throw invalid_argument(L"invalid range: from (" + to_wstring(from) +
                           L") cannot be > to (" + to_wstring(to) + L")");
  }
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_CHAR_RANGE;
  r->from = from;
  r->to = to;
  return r;
}

shared_ptr<RegExp> RegExp::makeAnyChar()
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_ANYCHAR;
  return r;
}

shared_ptr<RegExp> RegExp::makeEmpty()
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_EMPTY;
  return r;
}

shared_ptr<RegExp> RegExp::makeString(const wstring &s)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_STRING;
  r->s = s;
  return r;
}

shared_ptr<RegExp> RegExp::makeAnyString()
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_ANYSTRING;
  return r;
}

shared_ptr<RegExp> RegExp::makeAutomaton(const wstring &s)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_AUTOMATON;
  r->s = s;
  return r;
}

shared_ptr<RegExp> RegExp::makeInterval(int min, int max, int digits)
{
  shared_ptr<RegExp> r = make_shared<RegExp>();
  r->kind = Kind::REGEXP_INTERVAL;
  r->min = min;
  r->max = max;
  r->digits = digits;
  return r;
}

bool RegExp::peek(const wstring &s)
{
  return more() && s.find(originalString.codePointAt(pos)) != wstring::npos;
}

bool RegExp::match(int c)
{
  if (pos >= originalString.length()) {
    return false;
  }
  if (originalString.codePointAt(pos) == c) {
    pos += Character::charCount(c);
    return true;
  }
  return false;
}

bool RegExp::more() { return pos < originalString.length(); }

int RegExp::next() 
{
  if (!more()) {
    throw invalid_argument(L"unexpected end-of-string");
  }
  int ch = originalString.codePointAt(pos);
  pos += Character::charCount(ch);
  return ch;
}

bool RegExp::check(int flag) { return (flags & flag) != 0; }

shared_ptr<RegExp> RegExp::parseUnionExp() 
{
  shared_ptr<RegExp> e = parseInterExp();
  if (match(L'|')) {
    e = makeUnion(e, parseUnionExp());
  }
  return e;
}

shared_ptr<RegExp> RegExp::parseInterExp() 
{
  shared_ptr<RegExp> e = parseConcatExp();
  if (check(INTERSECTION) && match(L'&')) {
    e = makeIntersection(e, parseInterExp());
  }
  return e;
}

shared_ptr<RegExp> RegExp::parseConcatExp() 
{
  shared_ptr<RegExp> e = parseRepeatExp();
  if (more() && !peek(L")|") && (!check(INTERSECTION) || !peek(L"&"))) {
    e = makeConcatenation(e, parseConcatExp());
  }
  return e;
}

shared_ptr<RegExp> RegExp::parseRepeatExp() 
{
  shared_ptr<RegExp> e = parseComplExp();
  while (peek(L"?*+{")) {
    if (match(L'?')) {
      e = makeOptional(e);
    } else if (match(L'*')) {
      e = makeRepeat(e);
    } else if (match(L'+')) {
      e = makeRepeat(e, 1);
    } else if (match(L'{')) {
      int start = pos;
      while (peek(L"0123456789")) {
        next();
      }
      if (start == pos) {
        throw invalid_argument(L"integer expected at position " +
                               to_wstring(pos));
      }
      int n = stoi(originalString.substr(start, pos - start));
      int m = -1;
      if (match(L',')) {
        start = pos;
        while (peek(L"0123456789")) {
          next();
        }
        if (start != pos) {
          m = stoi(originalString.substr(start, pos - start));
        }
      } else {
        m = n;
      }
      if (!match(L'}')) {
        throw invalid_argument(L"expected '}' at position " + to_wstring(pos));
      }
      if (m == -1) {
        e = makeRepeat(e, n);
      } else {
        e = makeRepeat(e, n, m);
      }
    }
  }
  return e;
}

shared_ptr<RegExp> RegExp::parseComplExp() 
{
  if (check(COMPLEMENT) && match(L'~')) {
    return makeComplement(parseComplExp());
  } else {
    return parseCharClassExp();
  }
}

shared_ptr<RegExp> RegExp::parseCharClassExp() 
{
  if (match(L'[')) {
    bool negate = false;
    if (match(L'^')) {
      negate = true;
    }
    shared_ptr<RegExp> e = parseCharClasses();
    if (negate) {
      e = makeIntersection(makeAnyChar(), makeComplement(e));
    }
    if (!match(L']')) {
      throw invalid_argument(L"expected ']' at position " + to_wstring(pos));
    }
    return e;
  } else {
    return parseSimpleExp();
  }
}

shared_ptr<RegExp> RegExp::parseCharClasses() 
{
  shared_ptr<RegExp> e = parseCharClass();
  while (more() && !peek(L"]")) {
    e = makeUnion(e, parseCharClass());
  }
  return e;
}

shared_ptr<RegExp> RegExp::parseCharClass() 
{
  int c = parseCharExp();
  if (match(L'-')) {
    return makeCharRange(c, parseCharExp());
  } else {
    return makeChar(c);
  }
}

shared_ptr<RegExp> RegExp::parseSimpleExp() 
{
  if (match(L'.')) {
    return makeAnyChar();
  } else if (check(EMPTY) && match(L'#')) {
    return makeEmpty();
  } else if (check(ANYSTRING) && match(L'@')) {
    return makeAnyString();
  } else if (match(L'"')) {
    int start = pos;
    while (more() && !peek(L"\"")) {
      next();
    }
    if (!match(L'"')) {
      throw invalid_argument(L"expected '\"' at position " + to_wstring(pos));
    }
    return makeString(originalString.substr(start, (pos - 1) - start));
  } else if (match(L'(')) {
    if (match(L')')) {
      return makeString(L"");
    }
    shared_ptr<RegExp> e = parseUnionExp();
    if (!match(L')')) {
      throw invalid_argument(L"expected ')' at position " + to_wstring(pos));
    }
    return e;
  } else if ((check(AUTOMATON) || check(INTERVAL)) && match(L'<')) {
    int start = pos;
    while (more() && !peek(L">")) {
      next();
    }
    if (!match(L'>')) {
      throw invalid_argument(L"expected '>' at position " + to_wstring(pos));
    }
    wstring s = originalString.substr(start, (pos - 1) - start);
    int i = (int)s.find(L'-');
    if (i == -1) {
      if (!check(AUTOMATON)) {
        throw invalid_argument(L"interval syntax error at position " +
                               to_wstring(pos - 1));
      }
      return makeAutomaton(s);
    } else {
      if (!check(INTERVAL)) {
        throw invalid_argument(L"illegal identifier at position " +
                               to_wstring(pos - 1));
      }
      try {
        if (i == 0 || i == s.length() - 1 || i != (int)s.rfind(L'-')) {
          throw make_shared<NumberFormatException>();
        }
        wstring smin = s.substr(0, i);
        wstring smax = s.substr(i + 1, s.length() - (i + 1));
        int imin = stoi(smin);
        int imax = stoi(smax);
        int digits;
        if (smin.length() == smax.length()) {
          digits = smin.length();
        } else {
          digits = 0;
        }
        if (imin > imax) {
          int t = imin;
          imin = imax;
          imax = t;
        }
        return makeInterval(imin, imax, digits);
      } catch (const NumberFormatException &e) {
        throw invalid_argument(L"interval syntax error at position " +
                               to_wstring(pos - 1));
      }
    }
  } else {
    return makeChar(parseCharExp());
  }
}

int RegExp::parseCharExp() 
{
  match(L'\\');
  return next();
}
} // namespace org::apache::lucene::util::automaton