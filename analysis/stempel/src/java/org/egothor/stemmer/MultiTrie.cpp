using namespace std;

#include "MultiTrie.h"
#include "Reduce.h"

namespace org::egothor::stemmer
{

MultiTrie::MultiTrie(shared_ptr<DataInput> is)  : Trie(false)
{
  forward = is->readBoolean();
  BY = is->readInt();
  for (int i = is->readInt(); i > 0; i--) {
    tries.push_back(make_shared<Trie>(is));
  }
}

MultiTrie::MultiTrie(bool forward) : Trie(forward) {}

shared_ptr<std::wstring> MultiTrie::getFully(shared_ptr<std::wstring> key)
{
  shared_ptr<StringBuilder> result =
      make_shared<StringBuilder>(tries.size() * 2);
  for (int i = 0; i < tries.size(); i++) {
    shared_ptr<std::wstring> r = tries[i]->getFully(key);
    if (r == nullptr || (r->length() == 1 && r->charAt(0) == EOM)) {
      return result;
    }
    result->append(r);
  }
  return result;
}

shared_ptr<std::wstring> MultiTrie::getLastOnPath(shared_ptr<std::wstring> key)
{
  shared_ptr<StringBuilder> result =
      make_shared<StringBuilder>(tries.size() * 2);
  for (int i = 0; i < tries.size(); i++) {
    shared_ptr<std::wstring> r = tries[i]->getLastOnPath(key);
    if (r == nullptr || (r->length() == 1 && r->charAt(0) == EOM)) {
      return result;
    }
    result->append(r);
  }
  return result;
}

void MultiTrie::store(shared_ptr<DataOutput> os) 
{
  os->writeBoolean(forward);
  os->writeInt(BY);
  os->writeInt(tries.size());
  for (auto trie : tries) {
    trie->store(os);
  }
}

void MultiTrie::add(shared_ptr<std::wstring> key, shared_ptr<std::wstring> cmd)
{
  if (cmd->length() == 0) {
    return;
  }
  int levels = cmd->length() / BY;
  while (levels >= tries.size()) {
    tries.push_back(make_shared<Trie>(forward));
  }
  for (int i = 0; i < levels; i++) {
    tries[i]->add(key, cmd->substr(BY * i, BY));
  }
  tries[levels]->add(key, EOM_NODE);
}

shared_ptr<Trie> MultiTrie::reduce(shared_ptr<Reduce> by)
{
  deque<std::shared_ptr<Trie>> h = deque<std::shared_ptr<Trie>>();
  for (auto trie : tries) {
    h.push_back(trie->reduce(by));
  }

  shared_ptr<MultiTrie> m = make_shared<MultiTrie>(forward);
  m->tries = h;
  return m;
}

void MultiTrie::printInfo(shared_ptr<PrintStream> out,
                          shared_ptr<std::wstring> prefix)
{
  int c = 0;
  for (auto trie : tries) {
    trie->printInfo(out, prefix + L"[" + to_wstring(++c) + L"] ");
  }
}
} // namespace org::egothor::stemmer