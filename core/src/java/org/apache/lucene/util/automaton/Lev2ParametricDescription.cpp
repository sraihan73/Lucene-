using namespace std;

#include "Lev2ParametricDescription.h"

namespace org::apache::lucene::util::automaton
{
using ParametricDescription = org::apache::lucene::util::automaton::
    LevenshteinAutomata::ParametricDescription;

int Lev2ParametricDescription::transition(int absState, int position,
                                          int deque)
{
  // null absState should never be passed in
  assert(absState != -1);

  // decode absState -> state, offset
  int state = absState / (w + 1);
  int offset = absState % (w + 1);
  assert(offset >= 0);

  if (position == w) {
    if (state < 3) {
      constexpr int loc = deque * 3 + state;
      offset += unpack(offsetIncrs0, loc, 1);
      state = unpack(toStates0, loc, 2) - 1;
    }
  } else if (position == w - 1) {
    if (state < 5) {
      constexpr int loc = deque * 5 + state;
      offset += unpack(offsetIncrs1, loc, 1);
      state = unpack(toStates1, loc, 3) - 1;
    }
  } else if (position == w - 2) {
    if (state < 11) {
      constexpr int loc = deque * 11 + state;
      offset += unpack(offsetIncrs2, loc, 2);
      state = unpack(toStates2, loc, 4) - 1;
    }
  } else if (position == w - 3) {
    if (state < 21) {
      constexpr int loc = deque * 21 + state;
      offset += unpack(offsetIncrs3, loc, 2);
      state = unpack(toStates3, loc, 5) - 1;
    }
  } else if (position == w - 4) {
    if (state < 30) {
      constexpr int loc = deque * 30 + state;
      offset += unpack(offsetIncrs4, loc, 3);
      state = unpack(toStates4, loc, 5) - 1;
    }
  } else {
    if (state < 30) {
      constexpr int loc = deque * 30 + state;
      offset += unpack(offsetIncrs5, loc, 3);
      state = unpack(toStates5, loc, 5) - 1;
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

std::deque<int64_t> const Lev2ParametricDescription::toStates0 =
    std::deque<int64_t>{0x23LL};
std::deque<int64_t> const Lev2ParametricDescription::offsetIncrs0 =
    std::deque<int64_t>{0x0LL};
std::deque<int64_t> const Lev2ParametricDescription::toStates1 =
    std::deque<int64_t>{0x13688b44LL};
std::deque<int64_t> const Lev2ParametricDescription::offsetIncrs1 =
    std::deque<int64_t>{0x3e0LL};
std::deque<int64_t> const Lev2ParametricDescription::toStates2 =
    std::deque<int64_t>{0x26a09a0a0520a504LL, 0x2323523321a260a2LL,
                           0x354235543213LL};
std::deque<int64_t> const Lev2ParametricDescription::offsetIncrs2 =
    std::deque<int64_t>{0x5555520280000800LL, 0x555555LL};
std::deque<int64_t> const Lev2ParametricDescription::toStates3 =
    std::deque<int64_t>{0x380e014a051404LL,   0xe28245009451140LL,
                           0x8a26880098a6268cLL, 0x180a288ca0246213LL,
                           0x494053284a1080e1LL, 0x510265a89c311940LL,
                           0x4218c41188a6509cLL, 0x6340c4211c4710dLL,
                           0xa168398471882a12LL, 0x104c841c683a0425LL,
                           0x3294472904351483LL, 0xe6290620a84a20d0LL,
                           0x1441a0ea2896a4a0LL, 0x32LL};
std::deque<int64_t> const Lev2ParametricDescription::offsetIncrs3 =
    std::deque<int64_t>{0x33300230c0000800LL, 0x220ca080a00fc330LL,
                           0x555555f832823380LL, 0x5555555555555555LL,
                           0x5555555555555555LL, 0x5555LL};
std::deque<int64_t> const Lev2ParametricDescription::toStates4 =
    std::deque<int64_t>{
        0x380e014a051404LL,   0xaa015452940LL,      0x55014501000000LL,
        0x1843ddc771085c07LL, 0x7141200040108405LL, 0x52b44004c5313460LL,
        0x401080200063115cLL, 0x85314c4d181c5048LL, 0x1440190a3e5c7828LL,
        0x28a232809100a21LL,  0xa028ca2a84203846LL, 0xca0240010800108aLL,
        0xc7b4205c1580a508LL, 0x1021090251846b6LL,  0x4cb513862328090LL,
        0x210863128ca2b8a2LL, 0x4e188ca024402940LL, 0xa6b6c7c520532d4LL,
        0x8c41101451150219LL, 0xa0c4211c4710d421LL, 0x2108421094e15063LL,
        0x8f13c43708631044LL, 0x18274d908c611631LL, 0x1cc238c411098263LL,
        0x450e3a1d0212d0b4LL, 0x31050242048108c6LL, 0xfa318b42d07308eLL,
        0xa8865182356907c6LL, 0x1ca410d4520c4140LL, 0x2954e13883a0ca51LL,
        0x3714831044229442LL, 0x93946116b58f2c84LL, 0xc41109a5631a574dLL,
        0x1d4512d4941cc520LL, 0x52848294c643883aLL, 0xb525073148310502LL,
        0xa5356939460f7358LL, 0x409ca651LL};
std::deque<int64_t> const Lev2ParametricDescription::offsetIncrs4 =
    std::deque<int64_t>{
        0x20c0600000010000LL, 0x2000040000000001LL, 0x209204a40209LL,
        0x301b6c0618018618LL, 0x207206186000186cLL, 0x1200061b8e06dc0LL,
        0x480492080612010LL,  0xa20204a040048000LL, 0x1061a0000129124LL,
        0x1848349b680612LL,   0xd26da0204a041868LL, 0x2492492492496128LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x24924924LL};
std::deque<int64_t> const Lev2ParametricDescription::toStates5 =
    std::deque<int64_t>{
        0x380e014a051404LL,   0xaa015452940LL,      0x8052814501000000LL,
        0xb80a515450000e03LL, 0x5140410842108426LL, 0x71dc421701c01540LL,
        0x100421014610f7LL,   0x85c0700550145010LL, 0x94a271843ddc7710LL,
        0x1346071412108a22LL, 0x3115c52b44004c53LL, 0xc504840108020006LL,
        0x54d1001314c4d181LL, 0x9081204239c4a71LL,  0x14c5313460714124LL,
        0x51006428f971e0a2LL, 0x4d181c5048402884LL, 0xa3e5c782885314cLL,
        0x2809409482a8a239LL, 0x2a84203846028a23LL, 0x10800108aa028caLL,
        0xe1180a288ca0240LL,  0x98c6b80e3294a108LL, 0x2942328091098c10LL,
        0x11adb1ed08170560LL, 0xa024004084240946LL, 0x7b4205c1580a508cLL,
        0xa8c2968c71846b6cLL, 0x4cb5138623280910LL, 0x10863128ca2b8a20LL,
        0xe188ca0244029402LL, 0x4e3294e288132d44LL, 0x809409ad1218c39cLL,
        0xf14814cb51386232LL, 0x514454086429adb1LL, 0x32d44e188ca02440LL,
        0x8c390a6b6c7c5205LL, 0xd4218c41409cd2aaLL, 0x5063a0c4211c4710LL,
        0x10442108421094e1LL, 0x31084711c4350863LL, 0xbdef7bddf05918f2LL,
        0xc4f10dc218c41ef7LL, 0x9d3642318458c63LL,  0x70863104426098c6LL,
        0x8c6116318f13c43LL,  0x41ef75dd6b5de4d9LL, 0xd0212d0b41cc238cLL,
        0x2048108c6450e3a1LL, 0x42d07308e3105024LL, 0xdb591938f274084bLL,
        0xc238c41f77deefbbLL, 0x1f183e8c62d0b41cLL, 0x502a2194608d5a4LL,
        0xa318b42d07308e31LL, 0xed675db56907c60fLL, 0xa410d4520c41f773LL,
        0x54e13883a0ca511cLL, 0x1483104422944229LL, 0x20f2329447290435LL,
        0x1ef6f7ef6f7df05cLL, 0xad63cb210dc520c4LL, 0x58c695d364e51845LL,
        0xc843714831044269LL, 0xe4d93946116b58f2LL, 0x520c41ef717d6b17LL,
        0x83a1d4512d4941ccLL, 0x50252848294c6438LL, 0x144b525073148310LL,
        0xefaf7b591c20f275LL, 0x941cc520c41f777bLL, 0xd5a4e5183dcd62d4LL,
        0x4831050272994694LL, 0x460f7358b5250731LL, 0xf779bd6717b56939LL};
std::deque<int64_t> const Lev2ParametricDescription::offsetIncrs5 =
    std::deque<int64_t>{
        0x20c0600000010000LL, 0x40000000001LL,      0xb6db6d4830180LL,
        0x4812900824800010LL, 0x2092000040000082LL, 0x618000b659254a40LL,
        0x86c301b6c0618018LL, 0xdb01860061860001LL, 0x81861800075baed6LL,
        0x186e381b70081cLL,   0xe56dc02072061860LL, 0x61201001200075b8LL,
        0x480000480492080LL,  0x52b5248201848040LL, 0x880812810012000bLL,
        0x4004800004a4492LL,  0xb529124a20204aLL,   0x49b68061201061a0LL,
        0x8480418680018483LL, 0x1a000752ad26da01LL, 0x4a349b6808128106LL,
        0xa0204a0418680018LL, 0x492492497528d26dLL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL};

Lev2ParametricDescription::Lev2ParametricDescription(int w)
    : org::apache::lucene::util::automaton::LevenshteinAutomata::
          ParametricDescription(w, 2, new int[]{0,  2,  1,  0,  1,  -1, 0,  0,
                                                -1, 0,  -1, -1, -1, -1, -1, -2,
                                                -1, -1, -2, -1, -2, -2, -2, -2,
                                                -2, -2, -2, -2, -2, -2})
{
}
} // namespace org::apache::lucene::util::automaton