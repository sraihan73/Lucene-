using namespace std;

#include "Lev2TParametricDescription.h"

namespace org::apache::lucene::util::automaton
{
using ParametricDescription = org::apache::lucene::util::automaton::
    LevenshteinAutomata::ParametricDescription;

int Lev2TParametricDescription::transition(int absState, int position,
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
    if (state < 13) {
      constexpr int loc = deque * 13 + state;
      offset += unpack(offsetIncrs2, loc, 2);
      state = unpack(toStates2, loc, 4) - 1;
    }
  } else if (position == w - 3) {
    if (state < 28) {
      constexpr int loc = deque * 28 + state;
      offset += unpack(offsetIncrs3, loc, 2);
      state = unpack(toStates3, loc, 5) - 1;
    }
  } else if (position == w - 4) {
    if (state < 45) {
      constexpr int loc = deque * 45 + state;
      offset += unpack(offsetIncrs4, loc, 3);
      state = unpack(toStates4, loc, 6) - 1;
    }
  } else {
    if (state < 45) {
      constexpr int loc = deque * 45 + state;
      offset += unpack(offsetIncrs5, loc, 3);
      state = unpack(toStates5, loc, 6) - 1;
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

std::deque<int64_t> const Lev2TParametricDescription::toStates0 =
    std::deque<int64_t>{0x23LL};
std::deque<int64_t> const Lev2TParametricDescription::offsetIncrs0 =
    std::deque<int64_t>{0x0LL};
std::deque<int64_t> const Lev2TParametricDescription::toStates1 =
    std::deque<int64_t>{0x13688b44LL};
std::deque<int64_t> const Lev2TParametricDescription::offsetIncrs1 =
    std::deque<int64_t>{0x3e0LL};
std::deque<int64_t> const Lev2TParametricDescription::toStates2 =
    std::deque<int64_t>{0x60dbb0b05200b504LL, 0x5233217627062227LL,
                           0x2355543214323235LL, 0x4354LL};
std::deque<int64_t> const Lev2TParametricDescription::offsetIncrs2 =
    std::deque<int64_t>{0x555080a800002000LL, 0x5555555555LL};
std::deque<int64_t> const Lev2TParametricDescription::toStates3 =
    std::deque<int64_t>{
        0xe701c02940059404LL, 0xa010162000a50000LL, 0xb02c8c40a1416288LL,
        0xa821032310858c0LL,  0x314423980d28b201LL, 0x5281e528847788e0LL,
        0xa23980d308c2280eLL, 0x1e3294b1a962278cLL, 0x8c41309e2288e528LL,
        0x11444409021aca21LL, 0x11a4624886b1086bLL, 0x2a6258941d6240c4LL,
        0x5024a50b489074adLL, 0x14821aca520c411aLL, 0x5888b5890b594a44LL,
        0x941d6520c411a465LL, 0x8b589075ad6a62d4LL, 0x1a5055a4LL};
std::deque<int64_t> const Lev2TParametricDescription::offsetIncrs3 =
    std::deque<int64_t>{0x30c30200002000LL,   0x2a0030f3c3fc333cLL,
                           0x233a00328282a820LL, 0x5555555532b283a8LL,
                           0x5555555555555555LL, 0x5555555555555555LL,
                           0x5555555555555555LL};
std::deque<int64_t> const Lev2TParametricDescription::toStates4 =
    std::deque<int64_t>{0x3801450002c5004LL,  0xc500014b00000e38LL,
                           0x51451401402LL,      0x0LL,
                           0x518000b14010000LL,  0x9f1c20828e20230LL,
                           0x219f0df0830a70c2LL, 0x8200008208208200LL,
                           0x805050160800800LL,  0x3082098602602643LL,
                           0x4564014250508064LL, 0x850051420000831LL,
                           0x4140582085002082LL, 0x456180980990c201LL,
                           0x8316d0c50a01051LL,  0x21451420050df0e0LL,
                           0xd14214014508214LL,  0x3c21c01850821c60LL,
                           0x1cb1403cb142087LL,  0x800821451851822cLL,
                           0x20020820800020LL,   0xd006182087180345LL,
                           0xcb0a81cb24976b09LL, 0x8b1a60e624709d1LL,
                           0x249082082249089LL,  0xc31421c600d2c024LL,
                           0x3c31451515454423LL, 0x31853c22c21cb140LL,
                           0x4514500b2c208214LL, 0x8718034508b0051LL,
                           0xb2cb45515108f0c5LL, 0xe824715d1cb0a810LL,
                           0x1422cb14908b0e60LL, 0x30812c22c02cb145LL,
                           0x842022020cb1420cLL, 0x5c20ce0820ce0850LL,
                           0x208208208b0d70c2LL, 0x4208508214214208LL,
                           0x920834050830c20LL,  0xc6134dc613653592LL,
                           0xd309341c6dc4db4dLL, 0x6424d90854d34d34LL,
                           0x92072c22030814c2LL, 0x4220724b24a30930LL,
                           0x2470d72025c920e2LL, 0x92c92d70975c9082LL,
                           0xcb0880c204924e08LL, 0x45739728c24c2481LL,
                           0xc6da4db5da6174daLL, 0x4b5d35d75d30971dLL,
                           0x1030815c93825ce2LL, 0x51442051020cb145LL,
                           0xc538210e2c220e2cLL, 0x851421452cb0d70LL,
                           0x204b085085145142LL, 0x921560834051440cLL,
                           0x4d660e4da60e6595LL, 0x94d914e41c6dc658LL,
                           0x826426591454d365LL, 0x2892072c51030813LL,
                           0xe2c22072cb2ca30bLL, 0x452c70d720538910LL,
                           0x8b2cb2d708e3891LL,  0x81cb1440c204b24eLL,
                           0xda44e38e28c2ca24LL, 0x1dc6da6585d660e4LL,
                           0xe2cb5d338e5d914eLL, 0x38938238LL};
std::deque<int64_t> const Lev2TParametricDescription::offsetIncrs4 =
    std::deque<int64_t>{
        0x3002000000080000LL, 0x20c060LL,           0x8149000004000000LL,
        0x4024924110824824LL, 0xdb6030c360002082LL, 0x6c36c06c301b0d80LL,
        0xb01861b0000db0dbLL, 0x1b7036209188e06dLL, 0x800920006d86db7LL,
        0x4920c2402402490LL,  0x49000208249009LL,   0x4908128128124804LL,
        0x34800104124a44a2LL, 0xc30930900d24020cLL, 0x40009a0924c24d24LL,
        0x4984a069201061aLL,  0x494d049271269262LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x249249249249LL};
std::deque<int64_t> const Lev2TParametricDescription::toStates5 =
    std::deque<int64_t>{0x3801450002c5004LL,  0xc500014b00000e38LL,
                           0x51451401402LL,      0x0LL,
                           0x514000b14010000LL,  0x550000038e00e0LL,
                           0x264518500600b180LL, 0x8208208208208208LL,
                           0x2c50040820820LL,    0x70820a38808c0146LL,
                           0xc37c20c29c30827cLL, 0x20820820800867LL,
                           0xb140102002002080LL, 0x828e202300518000LL,
                           0x830a70c209f1c20LL,  0x51451450853df0dfLL,
                           0x1614214214508214LL, 0x6026026430805050LL,
                           0x2505080643082098LL, 0x4200008314564014LL,
                           0x850020820850051LL,  0x80990c2014140582LL,
                           0x8201920208261809LL, 0x892051990060941LL,
                           0x22492492c22cb242LL, 0x430805050162492cLL,
                           0x8041451586026026LL, 0x37c38020c5b43142LL,
                           0x4208508514508014LL, 0x141405850850051LL,
                           0x51456180980990c2LL, 0xe008316d0c50a010LL,
                           0x2c52cb2c508b21f0LL, 0x600d2c92c22cb249LL,
                           0x873c21c01850821cLL, 0x2c01cb1403cb1420LL,
                           0x2080082145185182LL, 0x4500200208208000LL,
                           0x870061420871803LL,  0x740500f5050821cfLL,
                           0x934d964618609000LL, 0x4c24d34d30824d30LL,
                           0x1860821c600d642LL,  0xc2a072c925dac274LL,
                           0x2c69839891c27472LL, 0x9242082089242242LL,
                           0x8208718034b00900LL, 0x1cb24976b09d0061LL,
                           0x60e624709d1cb0a8LL, 0xd31455d71574ce3eLL,
                           0x1c600d3825c25d74LL, 0x51515454423c3142LL,
                           0xc22c21cb1403c314LL, 0xb2c20821431853LL,
                           0x34508b005145145LL,  0x5515108f0c508718LL,
                           0x8740500f2051454LL,  0xe2534d920618f090LL,
                           0x493826596592c238LL, 0x4423c31421c600d6LL,
                           0x72c2a042cb2d1545LL, 0x422c3983a091c574LL,
                           0xb2c514508b2c52LL,   0xf0c508718034b08bLL,
                           0xa810b2cb45515108LL, 0x2260e824715d1cb0LL,
                           0xe6592c538e2d74ceLL, 0x420c308138938238LL,
                           0x850842022020cb1LL,  0x70c25c20ce0820ceLL,
                           0x4208208208208b0dLL, 0xc20420850821421LL,
                           0x21080880832c5083LL, 0xa50838820838c214LL,
                           0xaaaaaaaaa9c39430LL, 0x1aaa7eaa9fa9faaaLL,
                           0x824820d01420c308LL, 0x7184d37184d94d64LL,
                           0x34c24d071b7136d3LL, 0x990936421534d34dLL,
                           0x834050830c20530LL,  0x34dc613653592092LL,
                           0xa479c6dc4db4dc61LL, 0x920a9f924924924aLL,
                           0x72c220308192a82aLL, 0x724b24a30930920LL,
                           0xd72025c920e2422LL,  0x92d70975c9082247LL,
                           0x880c204924e0892cLL, 0x2c928c24c2481cb0LL,
                           0x80a5248889088749LL, 0x6a861b2aaac74394LL,
                           0x81b2ca6ab27b278LL,  0xa3093092072c2203LL,
                           0xd76985d36915ce5cLL, 0x5d74c25c771b6936LL,
                           0x724e0973892d74d7LL, 0x4c2481cb0880c205LL,
                           0x6174da45739728c2LL, 0x4aa175c6da4db5daLL,
                           0x6a869b2786486186LL, 0xcb14510308186caLL,
                           0x220e2c5144205102LL, 0xcb0d70c538210e2cLL,
                           0x1451420851421452LL, 0x51440c204b085085LL,
                           0xcb1451081440832cLL, 0x94316208488b0888LL,
                           0xfaaa7dfa9f7e79c3LL, 0x30819ea7ea7df7dLL,
                           0x6564855820d01451LL, 0x9613598393698399LL,
                           0xd965364539071b71LL, 0x4e0990996451534LL,
                           0x21560834051440c2LL, 0xd660e4da60e65959LL,
                           0x9207e979c6dc6584LL, 0xa82a8207df924820LL,
                           0x892072c5103081a6LL, 0x2c22072cb2ca30b2LL,
                           0x52c70d720538910eLL, 0x8b2cb2d708e38914LL,
                           0x1cb1440c204b24e0LL, 0x874b2cb28c2ca248LL,
                           0x4394816224488b08LL, 0x9e786aa69b1f7e77LL,
                           0x51030819eca6a9e7LL, 0x8e38a30b2892072cLL,
                           0x6996175983936913LL, 0x74ce39764538771bLL,
                           0xc204e24e08e38b2dLL, 0x28c2ca2481cb1440LL,
                           0x85d660e4da44e38eLL, 0x698607e975c6da65LL,
                           0xa6ca6aa699e7864aLL};
std::deque<int64_t> const Lev2TParametricDescription::offsetIncrs5 =
    std::deque<int64_t>{
        0x3002000000080000LL, 0x20c060LL,           0x100000004000000LL,
        0xdb6db6db50603018LL, 0xa480000200002db6LL, 0x1249208841241240LL,
        0x4000010000104120LL, 0x2492c42092092052LL, 0xc30d800096592d9LL,
        0xb01b0c06c36036d8LL, 0x186c00036c36db0dLL, 0xad860361b01b6c06LL,
        0x360001b75b6dd6ddLL, 0xc412311c0db6030cLL, 0xdb0db6e36e06LL,
        0x9188e06db01861bLL,  0x6dd6db71b72b62LL,   0x4024024900800920LL,
        0x20824900904920c2LL, 0x1201248040049000LL, 0x5524ad4aa4906120LL,
        0x4092402002480015LL, 0x9252251248409409LL, 0x4920100124000820LL,
        0x29128924204a04a0LL, 0x900830d200055549LL, 0x934930c24c24034LL,
        0x418690002682493LL,  0x9a49861261201a48LL, 0xc348001355249d4LL,
        0x24c40930940d2402LL, 0x1a40009a0924e24dLL, 0x6204984a06920106LL,
        0x92494d5492712692LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x4924924924924924LL, 0x2492492492492492LL,
        0x9249249249249249LL, 0x24924924LL};

Lev2TParametricDescription::Lev2TParametricDescription(int w)
    : org::apache::lucene::util::automaton::LevenshteinAutomata::
          ParametricDescription(
              w, 2, new int[]{0,  2,  1,  0,  1,  0,  -1, 0,  0,  -1, 0,  -1,
                              -1, -1, -1, -1, -2, -1, -1, -1, -2, -1, -1, -2,
                              -1, -1, -2, -1, -2, -2, -2, -2, -2, -2, -2, -2,
                              -2, -2, -2, -2, -2, -2, -2, -2, -2})
{
}
} // namespace org::apache::lucene::util::automaton