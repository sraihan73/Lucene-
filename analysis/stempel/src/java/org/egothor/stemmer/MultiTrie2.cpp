using namespace std;

#include "MultiTrie2.h"
#include "Reduce.h"
#include "Trie.h"

namespace org::egothor::stemmer
{

MultiTrie2::MultiTrie2(shared_ptr<DataInput> is) 
    : MultiTrie(is)
{
}

MultiTrie2::MultiTrie2(bool forward) : MultiTrie(forward) {}

shared_ptr<std::wstring> MultiTrie2::getFully(shared_ptr<std::wstring> key)
{
  shared_ptr<StringBuilder> result =
      make_shared<StringBuilder>(tries.size() * 2);
  try {
    shared_ptr<std::wstring> lastkey = key;
    std::deque<std::shared_ptr<std::wstring>> p(tries.size());
    wchar_t lastch = L' ';
    for (int i = 0; i < tries.size(); i++) {
      shared_ptr<std::wstring> r = tries[i]->getFully(lastkey);
      if (r == nullptr || (r->length() == 1 && r->charAt(0) == EOM)) {
        return result;
      }
      if (cannotFollow(lastch, r->charAt(0))) {
        return result;
      } else {
        lastch = r->charAt(r->length() - 2);
      }
      // key=key.substring(lengthPP(r));
      p[i] = r;
      if (p[i]->charAt(0) == L'-') {
        if (i > 0) {
          key = skip(key, lengthPP(p[i - 1]));
        }
        key = skip(key, lengthPP(p[i]));
      }
      // key = skip(key, lengthPP(r));
      result->append(r);
      if (key->length() != 0) {
        lastkey = key;
      }
    }
  } catch (const out_of_range &x) {
  }
  return result;
}

shared_ptr<std::wstring> MultiTrie2::getLastOnPath(shared_ptr<std::wstring> key)
{
  shared_ptr<StringBuilder> result =
      make_shared<StringBuilder>(tries.size() * 2);
  try {
    shared_ptr<std::wstring> lastkey = key;
    std::deque<std::shared_ptr<std::wstring>> p(tries.size());
    wchar_t lastch = L' ';
    for (int i = 0; i < tries.size(); i++) {
      shared_ptr<std::wstring> r = tries[i]->getLastOnPath(lastkey);
      if (r == nullptr || (r->length() == 1 && r->charAt(0) == EOM)) {
        return result;
      }
      // System.err.println("LP:"+key+" last:"+lastch+" new:"+r);
      if (cannotFollow(lastch, r->charAt(0))) {
        return result;
      } else {
        lastch = r->charAt(r->length() - 2);
      }
      // key=key.substring(lengthPP(r));
      p[i] = r;
      if (p[i]->charAt(0) == L'-') {
        if (i > 0) {
          key = skip(key, lengthPP(p[i - 1]));
        }
        key = skip(key, lengthPP(p[i]));
      }
      // key = skip(key, lengthPP(r));
      result->append(r);
      if (key->length() != 0) {
        lastkey = key;
      }
    }
  } catch (const out_of_range &x) {
  }
  return result;
}

void MultiTrie2::store(shared_ptr<DataOutput> os) 
{
  MultiTrie::store(os);
}

void MultiTrie2::add(shared_ptr<std::wstring> key, shared_ptr<std::wstring> cmd)
{
  if (cmd->length() == 0) {
    return;
  }
  // System.err.println( cmd );
  std::deque<std::shared_ptr<std::wstring>> p = decompose(cmd);
  int levels = p.size();
  // System.err.println("levels "+key+" cmd "+cmd+"|"+levels);
  while (levels >= tries.size()) {
    tries.push_back(make_shared<Trie>(forward));
  }
  shared_ptr<std::wstring> lastkey = key;
  for (int i = 0; i < levels; i++) {
    if (key->length() > 0) {
      tries[i]->add(key, p[i]);
      lastkey = key;
    } else {
      tries[i]->add(lastkey, p[i]);
    }
    // System.err.println("-"+key+" "+p[i]+"|"+key.length());
    /*
     * key=key.substring(lengthPP(p[i]));
     */
    if (p[i]->length() > 0 && p[i]->charAt(0) == L'-') {
      if (i > 0) {
        key = skip(key, lengthPP(p[i - 1]));
      }
      key = skip(key, lengthPP(p[i]));
    }
    // System.err.println("--->"+key);
  }
  if (key->length() > 0) {
    tries[levels]->add(key, EOM_NODE);
  } else {
    tries[levels]->add(lastkey, EOM_NODE);
  }
}

std::deque<std::shared_ptr<std::wstring>>
MultiTrie2::decompose(shared_ptr<std::wstring> cmd)
{
  int parts = 0;

  for (int i = 0; 0 <= i && i < cmd->length();) {
    int next = dashEven(cmd, i);
    if (i == next) {
      parts++;
      i = next + 2;
    } else {
      parts++;
      i = next;
    }
  }

  std::deque<std::shared_ptr<std::wstring>> part(parts);
  int x = 0;

  for (int i = 0; 0 <= i && i < cmd->length();) {
    int next = dashEven(cmd, i);
    if (i == next) {
      part[x++] = cmd->substr(i, 2);
      i = next + 2;
    } else {
      part[x++] = (next < 0) ? cmd->substr(i, cmd->length() - i)
                             : cmd->substr(i, next - i);
      i = next;
    }
  }
  return part;
}

shared_ptr<Trie> MultiTrie2::reduce(shared_ptr<Reduce> by)
{
  deque<std::shared_ptr<Trie>> h = deque<std::shared_ptr<Trie>>();
  for (auto trie : tries) {
    h.push_back(trie->reduce(by));
  }

  shared_ptr<MultiTrie2> m = make_shared<MultiTrie2>(forward);
  m->tries = h;
  return m;
}

bool MultiTrie2::cannotFollow(wchar_t after, wchar_t goes)
{
  switch (after) {
  case L'-':
  case L'D':
    return after == goes;
  }
  return false;
}

shared_ptr<std::wstring> MultiTrie2::skip(shared_ptr<std::wstring> in_,
                                          int count)
{
  if (forward) {
    return in_->substr(count, in_->length() - count);
  } else {
    return in_->substr(0, in_->length() - count);
  }
}

int MultiTrie2::dashEven(shared_ptr<std::wstring> in_, int from)
{
  while (from < in_->length()) {
    if (in_->charAt(from) == L'-') {
      return from;
    } else {
      from += 2;
    }
  }
  return -1;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private int
// lengthPP(std::wstring cmd)
int MultiTrie2::lengthPP(shared_ptr<std::wstring> cmd)
{
  int len = 0;
  for (int i = 0; i < cmd->length(); i++) {
    switch (cmd->charAt(i++)) {
    case L'-':
    case L'D':
      len += cmd->charAt(i) - L'a' + 1;
      break;
    case L'R':
      len++; // intentional fallthrough
    case L'I':
      break;
    }
  }
  return len;
}
} // namespace org::egothor::stemmer