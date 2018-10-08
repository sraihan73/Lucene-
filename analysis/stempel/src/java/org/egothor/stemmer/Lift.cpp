using namespace std;

#include "Lift.h"
#include "Cell.h"
#include "Row.h"
#include "Trie.h"

namespace org::egothor::stemmer
{

Lift::Lift(bool changeSkip) { this->changeSkip = changeSkip; }

shared_ptr<Trie> Lift::optimize(shared_ptr<Trie> orig)
{
  deque<std::shared_ptr<std::wstring>> &cmds = orig->cmds;
  deque<std::shared_ptr<Row>> rows = deque<std::shared_ptr<Row>>();
  deque<std::shared_ptr<Row>> &orows = orig->rows;
  std::deque<int> remap(orows.size());

  for (int j = orows.size() - 1; j >= 0; j--) {
    liftUp(orows[j], orows);
  }

  Arrays::fill(remap, -1);
  rows = removeGaps(orig->root, orows, deque<std::shared_ptr<Row>>(), remap);

  return make_shared<Trie>(orig->forward, remap[orig->root], cmds, rows);
}

void Lift::liftUp(shared_ptr<Row> in_, deque<std::shared_ptr<Row>> &nodes)
{
  Iterator<std::shared_ptr<Cell>> i = in_->cells.values().begin();
  for (; i->hasNext();) {
    shared_ptr<Cell> c = i->next();
    if (c->ref >= 0) {
      shared_ptr<Row> to = nodes[c->ref];
      int sum = to->uniformCmd(changeSkip);
      if (sum >= 0) {
        if (sum == c->cmd) {
          if (changeSkip) {
            if (c->skip != to->uniformSkip + 1) {
              continue;
            }
            c->skip = to->uniformSkip + 1;
          } else {
            c->skip = 0;
          }
          c->cnt += to->uniformCnt;
          c->ref = -1;
        } else if (c->cmd < 0) {
          c->cnt = to->uniformCnt;
          c->cmd = sum;
          c->ref = -1;
          if (changeSkip) {
            c->skip = to->uniformSkip + 1;
          } else {
            c->skip = 0;
          }
        }
      }
    }
  }
}
} // namespace org::egothor::stemmer