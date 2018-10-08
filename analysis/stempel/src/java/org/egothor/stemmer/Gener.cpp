using namespace std;

#include "Gener.h"
#include "Cell.h"
#include "Row.h"
#include "Trie.h"

namespace org::egothor::stemmer
{

Gener::Gener() {}

shared_ptr<Trie> Gener::optimize(shared_ptr<Trie> orig)
{
  deque<std::shared_ptr<std::wstring>> &cmds = orig->cmds;
  deque<std::shared_ptr<Row>> rows = deque<std::shared_ptr<Row>>();
  deque<std::shared_ptr<Row>> &orows = orig->rows;
  std::deque<int> remap(orows.size());

  Arrays::fill(remap, 1);
  for (int j = orows.size() - 1; j >= 0; j--) {
    if (eat(orows[j], remap)) {
      remap[j] = 0;
    }
  }

  Arrays::fill(remap, -1);
  rows = removeGaps(orig->root, orows, deque<std::shared_ptr<Row>>(), remap);

  return make_shared<Trie>(orig->forward, remap[orig->root], cmds, rows);
}

bool Gener::eat(shared_ptr<Row> in_, std::deque<int> &remap)
{
  int sum = 0;
  for (shared_ptr<Iterator<std::shared_ptr<Cell>>> i =
           in_->cells.values().begin();
       i->hasNext();) {
    shared_ptr<Cell> c = i->next();
    sum += c->cnt;
    if (c->ref >= 0) {
      if (remap[c->ref] == 0) {
        c->ref = -1;
      }
    }
  }
  int frame = sum / 10;
  bool live = false;
  for (shared_ptr<Iterator<std::shared_ptr<Cell>>> i =
           in_->cells.values().begin();
       i->hasNext();) {
    shared_ptr<Cell> c = i->next();
    if (c->cnt < frame && c->cmd >= 0) {
      c->cnt = 0;
      c->cmd = -1;
    }
    if (c->cmd >= 0 || c->ref >= 0) {
      live |= true;
    }
  }
  return !live;
}
} // namespace org::egothor::stemmer