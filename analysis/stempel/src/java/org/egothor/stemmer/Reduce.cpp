using namespace std;

#include "Reduce.h"
#include "Cell.h"
#include "Trie.h"

namespace org::egothor::stemmer
{

Reduce::Reduce() {}

shared_ptr<Trie> Reduce::optimize(shared_ptr<Trie> orig)
{
  deque<std::shared_ptr<std::wstring>> &cmds = orig->cmds;
  deque<std::shared_ptr<Row>> rows = deque<std::shared_ptr<Row>>();
  deque<std::shared_ptr<Row>> &orows = orig->rows;
  std::deque<int> remap(orows.size());

  Arrays::fill(remap, -1);
  rows = removeGaps(orig->root, rows, deque<std::shared_ptr<Row>>(), remap);

  return make_shared<Trie>(orig->forward, remap[orig->root], cmds, rows);
}

deque<std::shared_ptr<Row>>
Reduce::removeGaps(int ind, deque<std::shared_ptr<Row>> &old,
                   deque<std::shared_ptr<Row>> &to, std::deque<int> &remap)
{
  remap[ind] = to.size();

  shared_ptr<Row> now = old[ind];
  to.push_back(now);
  Iterator<std::shared_ptr<Cell>> i = now->cells.values().begin();
  for (; i->hasNext();) {
    shared_ptr<Cell> c = i->next();
    if (c->ref >= 0 && remap[c->ref] < 0) {
      removeGaps(c->ref, old, to, remap);
    }
  }
  to[remap[ind]] = make_shared<Remap>(shared_from_this(), now, remap);
  return to;
}

Reduce::Remap::Remap(shared_ptr<Reduce> outerInstance, shared_ptr<Row> old,
                     std::deque<int> &remap)
    : Row(), outerInstance(outerInstance)
{
  Iterator<wchar_t> i = old->cells.keySet().begin();
  for (; i->hasNext();) {
    optional<wchar_t> ch = i->next();
    shared_ptr<Cell> c = old->at(ch);
    shared_ptr<Cell> nc;
    if (c->ref >= 0) {
      nc = make_shared<Cell>(c);
      nc->ref = remap[nc->ref];
    } else {
      nc = make_shared<Cell>(c);
    }
    cells.emplace(ch, nc);
  }
}
} // namespace org::egothor::stemmer