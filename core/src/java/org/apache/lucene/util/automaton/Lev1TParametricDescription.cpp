using namespace std;

#include "Lev1TParametricDescription.h"

namespace org::apache::lucene::util::automaton
{
using ParametricDescription = org::apache::lucene::util::automaton::
    LevenshteinAutomata::ParametricDescription;

int Lev1TParametricDescription::transition(int absState, int position,
                                           int deque)
{
  // null absState should never be passed in
  assert(absState != -1);

  // decode absState -> state, offset
  int state = absState / (w + 1);
  int offset = absState % (w + 1);
  assert(offset >= 0);

  if (position == w) {
    if (state < 2) {
      constexpr int loc = deque * 2 + state;
      offset += unpack(offsetIncrs0, loc, 1);
      state = unpack(toStates0, loc, 2) - 1;
    }
  } else if (position == w - 1) {
    if (state < 3) {
      constexpr int loc = deque * 3 + state;
      offset += unpack(offsetIncrs1, loc, 1);
      state = unpack(toStates1, loc, 2) - 1;
    }
  } else if (position == w - 2) {
    if (state < 6) {
      constexpr int loc = deque * 6 + state;
      offset += unpack(offsetIncrs2, loc, 2);
      state = unpack(toStates2, loc, 3) - 1;
    }
  } else {
    if (state < 6) {
      constexpr int loc = deque * 6 + state;
      offset += unpack(offsetIncrs3, loc, 2);
      state = unpack(toStates3, loc, 3) - 1;
    }
  }

  if (state == -1) {
    // null state
    return -1;
  } else {
    // translate back to abs
    return state * (w + 1) + offset;
  }
}

std::deque<int64_t> const Lev1TParametricDescription::toStates0 =
    std::deque<int64_t>{0x2LL};
std::deque<int64_t> const Lev1TParametricDescription::offsetIncrs0 =
    std::deque<int64_t>{0x0LL};
std::deque<int64_t> const Lev1TParametricDescription::toStates1 =
    std::deque<int64_t>{0xa43LL};
std::deque<int64_t> const Lev1TParametricDescription::offsetIncrs1 =
    std::deque<int64_t>{0x38LL};
std::deque<int64_t> const Lev1TParametricDescription::toStates2 =
    std::deque<int64_t>{0x3453491482140003LL, 0x6dLL};
std::deque<int64_t> const Lev1TParametricDescription::offsetIncrs2 =
    std::deque<int64_t>{0x555555a20000LL};
std::deque<int64_t> const Lev1TParametricDescription::toStates3 =
    std::deque<int64_t>{0x21520854900c0003LL, 0x5b4d19a24534916dLL,
                           0xda34LL};
std::deque<int64_t> const Lev1TParametricDescription::offsetIncrs3 =
    std::deque<int64_t>{0x5555ae0a20fc0000LL, 0x55555555LL};

Lev1TParametricDescription::Lev1TParametricDescription(int w)
    : org::apache::lucene::util::automaton::LevenshteinAutomata::
          ParametricDescription(w, 1, new int[]{0, 1, 0, -1, -1, -1})
{
}
} // namespace org::apache::lucene::util::automaton