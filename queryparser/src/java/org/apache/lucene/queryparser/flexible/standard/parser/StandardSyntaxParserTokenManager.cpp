using namespace std;

#include "StandardSyntaxParserTokenManager.h"

namespace org::apache::lucene::queryparser::flexible::standard::parser
{

int StandardSyntaxParserTokenManager::jjStopStringLiteralDfa_2(
    int pos, int64_t active0)
{
  switch (pos) {
  default:
    return -1;
  }
}

int StandardSyntaxParserTokenManager::jjStartNfa_2(int pos, int64_t active0)
{
  return jjMoveNfa_2(jjStopStringLiteralDfa_2(pos, active0), pos + 1);
}

int StandardSyntaxParserTokenManager::jjStopAtPos(int pos, int kind)
{
  jjmatchedKind = kind;
  jjmatchedPos = pos;
  return pos + 1;
}

int StandardSyntaxParserTokenManager::jjMoveStringLiteralDfa0_2()
{
  switch (curChar) {
  case 40:
    return jjStopAtPos(0, 13);
  case 41:
    return jjStopAtPos(0, 14);
  case 43:
    return jjStopAtPos(0, 11);
  case 45:
    return jjStopAtPos(0, 12);
  case 58:
    return jjStopAtPos(0, 15);
  case 60:
    jjmatchedKind = 17;
    return jjMoveStringLiteralDfa1_2(0x40000LL);
  case 61:
    return jjStopAtPos(0, 16);
  case 62:
    jjmatchedKind = 19;
    return jjMoveStringLiteralDfa1_2(0x100000LL);
  case 91:
    return jjStopAtPos(0, 26);
  case 94:
    return jjStopAtPos(0, 21);
  case 123:
    return jjStopAtPos(0, 27);
  default:
    return jjMoveNfa_2(0, 0);
  }
}

int StandardSyntaxParserTokenManager::jjMoveStringLiteralDfa1_2(
    int64_t active0)
{
  try {
    curChar = input_stream->readChar();
  } catch (const IOException &e) {
    jjStopStringLiteralDfa_2(0, active0);
    return 1;
  }
  switch (curChar) {
  case 61:
    if ((active0 & 0x40000LL) != 0LL) {
      return jjStopAtPos(1, 18);
    } else if ((active0 & 0x100000LL) != 0LL) {
      return jjStopAtPos(1, 20);
    }
    break;
  default:
    break;
  }
  return jjStartNfa_2(0, active0);
}

std::deque<int64_t> const StandardSyntaxParserTokenManager::jjbitVec0 = {
    0x1LL, 0x0LL, 0x0LL, 0x0LL};
std::deque<int64_t> const StandardSyntaxParserTokenManager::jjbitVec1 = {
    0xfffffffffffffffeLL, 0xffffffffffffffffLL, 0xffffffffffffffffLL,
    0xffffffffffffffffLL};
std::deque<int64_t> const StandardSyntaxParserTokenManager::jjbitVec3 = {
    0x0LL, 0x0LL, 0xffffffffffffffffLL, 0xffffffffffffffffLL};
std::deque<int64_t> const StandardSyntaxParserTokenManager::jjbitVec4 = {
    0xfffefffffffffffeLL, 0xffffffffffffffffLL, 0xffffffffffffffffLL,
    0xffffffffffffffffLL};

int StandardSyntaxParserTokenManager::jjMoveNfa_2(int startState, int curPos)
{
  int startsAt = 0;
  jjnewStateCnt = 33;
  int i = 1;
  jjstateSet[0] = startState;
  int kind = 0x7fffffff;
  for (;;) {
    if (++jjround == 0x7fffffff) {
      ReInitRounds();
    }
    if (curChar < 64) {
      int64_t l = 1LL << curChar;
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0x8bff54f8ffffd9ffLL & l) != 0LL) {
            if (kind > 23) {
              kind = 23;
            }
            jjCheckNAddTwoStates(20, 21);
          } else if ((0x100002600LL & l) != 0LL) {
            if (kind > 7) {
              kind = 7;
            }
          } else if (curChar == 47) {
            jjCheckNAddStates(0, 2);
          } else if (curChar == 34) {
            jjCheckNAddStates(3, 5);
          } else if (curChar == 33) {
            if (kind > 10) {
              kind = 10;
            }
          }
          if (curChar == 38) {
            jjstateSet[jjnewStateCnt++] = 4;
          }
          break;
        case 4:
          if (curChar == 38 && kind > 8) {
            kind = 8;
          }
          break;
        case 5:
          if (curChar == 38) {
            jjstateSet[jjnewStateCnt++] = 4;
          }
          break;
        case 13:
          if (curChar == 33 && kind > 10) {
            kind = 10;
          }
          break;
        case 14:
          if (curChar == 34) {
            jjCheckNAddStates(3, 5);
          }
          break;
        case 15:
          if ((0xfffffffbffffffffLL & l) != 0LL) {
            jjCheckNAddStates(3, 5);
          }
          break;
        case 17:
          jjCheckNAddStates(3, 5);
          break;
        case 18:
          if (curChar == 34 && kind > 22) {
            kind = 22;
          }
          break;
        case 19:
          if ((0x8bff54f8ffffd9ffLL & l) == 0LL) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 20:
          if ((0x8bff7cf8ffffd9ffLL & l) == 0LL) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 22:
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 25:
          if ((0x3ff000000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 24) {
            kind = 24;
          }
          jjAddStates(6, 7);
          break;
        case 26:
          if (curChar == 46) {
            jjCheckNAdd(27);
          }
          break;
        case 27:
          if ((0x3ff000000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 24) {
            kind = 24;
          }
          jjCheckNAdd(27);
          break;
        case 28:
        case 30:
          if (curChar == 47) {
            jjCheckNAddStates(0, 2);
          }
          break;
        case 29:
          if ((0xffff7fffffffffffLL & l) != 0LL) {
            jjCheckNAddStates(0, 2);
          }
          break;
        case 32:
          if (curChar == 47 && kind > 25) {
            kind = 25;
          }
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else if (curChar < 128) {
      int64_t l = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0x97ffffff87ffffffLL & l) != 0LL) {
            if (kind > 23) {
              kind = 23;
            }
            jjCheckNAddTwoStates(20, 21);
          } else if (curChar == 126) {
            if (kind > 24) {
              kind = 24;
            }
            jjstateSet[jjnewStateCnt++] = 25;
          } else if (curChar == 92) {
            jjCheckNAdd(22);
          }
          if (curChar == 78) {
            jjstateSet[jjnewStateCnt++] = 11;
          } else if (curChar == 124) {
            jjstateSet[jjnewStateCnt++] = 8;
          } else if (curChar == 79) {
            jjstateSet[jjnewStateCnt++] = 6;
          } else if (curChar == 65) {
            jjstateSet[jjnewStateCnt++] = 2;
          }
          break;
        case 1:
          if (curChar == 68 && kind > 8) {
            kind = 8;
          }
          break;
        case 2:
          if (curChar == 78) {
            jjstateSet[jjnewStateCnt++] = 1;
          }
          break;
        case 3:
          if (curChar == 65) {
            jjstateSet[jjnewStateCnt++] = 2;
          }
          break;
        case 6:
          if (curChar == 82 && kind > 9) {
            kind = 9;
          }
          break;
        case 7:
          if (curChar == 79) {
            jjstateSet[jjnewStateCnt++] = 6;
          }
          break;
        case 8:
          if (curChar == 124 && kind > 9) {
            kind = 9;
          }
          break;
        case 9:
          if (curChar == 124) {
            jjstateSet[jjnewStateCnt++] = 8;
          }
          break;
        case 10:
          if (curChar == 84 && kind > 10) {
            kind = 10;
          }
          break;
        case 11:
          if (curChar == 79) {
            jjstateSet[jjnewStateCnt++] = 10;
          }
          break;
        case 12:
          if (curChar == 78) {
            jjstateSet[jjnewStateCnt++] = 11;
          }
          break;
        case 15:
          if ((0xffffffffefffffffLL & l) != 0LL) {
            jjCheckNAddStates(3, 5);
          }
          break;
        case 16:
          if (curChar == 92) {
            jjstateSet[jjnewStateCnt++] = 17;
          }
          break;
        case 17:
          jjCheckNAddStates(3, 5);
          break;
        case 19:
        case 20:
          if ((0x97ffffff87ffffffLL & l) == 0LL) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 21:
          if (curChar == 92) {
            jjCheckNAddTwoStates(22, 22);
          }
          break;
        case 22:
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 23:
          if (curChar == 92) {
            jjCheckNAdd(22);
          }
          break;
        case 24:
          if (curChar != 126) {
            break;
          }
          if (kind > 24) {
            kind = 24;
          }
          jjstateSet[jjnewStateCnt++] = 25;
          break;
        case 29:
          jjAddStates(0, 2);
          break;
        case 31:
          if (curChar == 92) {
            jjstateSet[jjnewStateCnt++] = 30;
          }
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else {
      int hiByte = static_cast<int>(curChar >> 8);
      int i1 = hiByte >> 6;
      int64_t l1 = 1LL << (hiByte & 077);
      int i2 = (curChar & 0xff) >> 6;
      int64_t l2 = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if (jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            if (kind > 7) {
              kind = 7;
            }
          }
          if (jjCanMove_2(hiByte, i1, i2, l1, l2)) {
            if (kind > 23) {
              kind = 23;
            }
            jjCheckNAddTwoStates(20, 21);
          }
          break;
        case 15:
        case 17:
          if (jjCanMove_1(hiByte, i1, i2, l1, l2)) {
            jjCheckNAddStates(3, 5);
          }
          break;
        case 19:
        case 20:
          if (!jjCanMove_2(hiByte, i1, i2, l1, l2)) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 22:
          if (!jjCanMove_1(hiByte, i1, i2, l1, l2)) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAddTwoStates(20, 21);
          break;
        case 29:
          if (jjCanMove_1(hiByte, i1, i2, l1, l2)) {
            jjAddStates(0, 2);
          }
          break;
        default:
          break;
        }
      } while (i != startsAt);
    }
    if (kind != 0x7fffffff) {
      jjmatchedKind = kind;
      jjmatchedPos = curPos;
      kind = 0x7fffffff;
    }
    ++curPos;
    if ((i = jjnewStateCnt) == (startsAt = 33 - (jjnewStateCnt = startsAt))) {
      return curPos;
    }
    try {
      curChar = input_stream->readChar();
    } catch (const IOException &e) {
      return curPos;
    }
  }
}

int StandardSyntaxParserTokenManager::jjMoveStringLiteralDfa0_0()
{
  return jjMoveNfa_0(0, 0);
}

int StandardSyntaxParserTokenManager::jjMoveNfa_0(int startState, int curPos)
{
  int startsAt = 0;
  jjnewStateCnt = 3;
  int i = 1;
  jjstateSet[0] = startState;
  int kind = 0x7fffffff;
  for (;;) {
    if (++jjround == 0x7fffffff) {
      ReInitRounds();
    }
    if (curChar < 64) {
      int64_t l = 1LL << curChar;
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0x3ff000000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 28) {
            kind = 28;
          }
          jjAddStates(8, 9);
          break;
        case 1:
          if (curChar == 46) {
            jjCheckNAdd(2);
          }
          break;
        case 2:
          if ((0x3ff000000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 28) {
            kind = 28;
          }
          jjCheckNAdd(2);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else if (curChar < 128) {
      int64_t l = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        default:
          break;
        }
      } while (i != startsAt);
    } else {
      int hiByte = static_cast<int>(curChar >> 8);
      int i1 = hiByte >> 6;
      int64_t l1 = 1LL << (hiByte & 077);
      int i2 = (curChar & 0xff) >> 6;
      int64_t l2 = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        default:
          break;
        }
      } while (i != startsAt);
    }
    if (kind != 0x7fffffff) {
      jjmatchedKind = kind;
      jjmatchedPos = curPos;
      kind = 0x7fffffff;
    }
    ++curPos;
    if ((i = jjnewStateCnt) == (startsAt = 3 - (jjnewStateCnt = startsAt))) {
      return curPos;
    }
    try {
      curChar = input_stream->readChar();
    } catch (const IOException &e) {
      return curPos;
    }
  }
}

int StandardSyntaxParserTokenManager::jjStopStringLiteralDfa_1(
    int pos, int64_t active0)
{
  switch (pos) {
  case 0:
    if ((active0 & 0x20000000LL) != 0LL) {
      jjmatchedKind = 33;
      return 6;
    }
    return -1;
  default:
    return -1;
  }
}

int StandardSyntaxParserTokenManager::jjStartNfa_1(int pos, int64_t active0)
{
  return jjMoveNfa_1(jjStopStringLiteralDfa_1(pos, active0), pos + 1);
}

int StandardSyntaxParserTokenManager::jjMoveStringLiteralDfa0_1()
{
  switch (curChar) {
  case 84:
    return jjMoveStringLiteralDfa1_1(0x20000000LL);
  case 93:
    return jjStopAtPos(0, 30);
  case 125:
    return jjStopAtPos(0, 31);
  default:
    return jjMoveNfa_1(0, 0);
  }
}

int StandardSyntaxParserTokenManager::jjMoveStringLiteralDfa1_1(
    int64_t active0)
{
  try {
    curChar = input_stream->readChar();
  } catch (const IOException &e) {
    jjStopStringLiteralDfa_1(0, active0);
    return 1;
  }
  switch (curChar) {
  case 79:
    if ((active0 & 0x20000000LL) != 0LL) {
      return jjStartNfaWithStates_1(1, 29, 6);
    }
    break;
  default:
    break;
  }
  return jjStartNfa_1(0, active0);
}

int StandardSyntaxParserTokenManager::jjStartNfaWithStates_1(int pos, int kind,
                                                             int state)
{
  jjmatchedKind = kind;
  jjmatchedPos = pos;
  try {
    curChar = input_stream->readChar();
  } catch (const IOException &e) {
    return pos + 1;
  }
  return jjMoveNfa_1(state, pos + 1);
}

int StandardSyntaxParserTokenManager::jjMoveNfa_1(int startState, int curPos)
{
  int startsAt = 0;
  jjnewStateCnt = 7;
  int i = 1;
  jjstateSet[0] = startState;
  int kind = 0x7fffffff;
  for (;;) {
    if (++jjround == 0x7fffffff) {
      ReInitRounds();
    }
    if (curChar < 64) {
      int64_t l = 1LL << curChar;
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0xfffffffeffffffffLL & l) != 0LL) {
            if (kind > 33) {
              kind = 33;
            }
            jjCheckNAdd(6);
          }
          if ((0x100002600LL & l) != 0LL) {
            if (kind > 7) {
              kind = 7;
            }
          } else if (curChar == 34) {
            jjCheckNAddTwoStates(2, 4);
          }
          break;
        case 1:
          if (curChar == 34) {
            jjCheckNAddTwoStates(2, 4);
          }
          break;
        case 2:
          if ((0xfffffffbffffffffLL & l) != 0LL) {
            jjCheckNAddStates(10, 12);
          }
          break;
        case 3:
          if (curChar == 34) {
            jjCheckNAddStates(10, 12);
          }
          break;
        case 5:
          if (curChar == 34 && kind > 32) {
            kind = 32;
          }
          break;
        case 6:
          if ((0xfffffffeffffffffLL & l) == 0LL) {
            break;
          }
          if (kind > 33) {
            kind = 33;
          }
          jjCheckNAdd(6);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else if (curChar < 128) {
      int64_t l = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        case 0:
        case 6:
          if ((0xdfffffffdfffffffLL & l) == 0LL) {
            break;
          }
          if (kind > 33) {
            kind = 33;
          }
          jjCheckNAdd(6);
          break;
        case 2:
          jjAddStates(10, 12);
          break;
        case 4:
          if (curChar == 92) {
            jjstateSet[jjnewStateCnt++] = 3;
          }
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else {
      int hiByte = static_cast<int>(curChar >> 8);
      int i1 = hiByte >> 6;
      int64_t l1 = 1LL << (hiByte & 077);
      int i2 = (curChar & 0xff) >> 6;
      int64_t l2 = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if (jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            if (kind > 7) {
              kind = 7;
            }
          }
          if (jjCanMove_1(hiByte, i1, i2, l1, l2)) {
            if (kind > 33) {
              kind = 33;
            }
            jjCheckNAdd(6);
          }
          break;
        case 2:
          if (jjCanMove_1(hiByte, i1, i2, l1, l2)) {
            jjAddStates(10, 12);
          }
          break;
        case 6:
          if (!jjCanMove_1(hiByte, i1, i2, l1, l2)) {
            break;
          }
          if (kind > 33) {
            kind = 33;
          }
          jjCheckNAdd(6);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    }
    if (kind != 0x7fffffff) {
      jjmatchedKind = kind;
      jjmatchedPos = curPos;
      kind = 0x7fffffff;
    }
    ++curPos;
    if ((i = jjnewStateCnt) == (startsAt = 7 - (jjnewStateCnt = startsAt))) {
      return curPos;
    }
    try {
      curChar = input_stream->readChar();
    } catch (const IOException &e) {
      return curPos;
    }
  }
}

std::deque<int> const StandardSyntaxParserTokenManager::jjnextStates = {
    29, 31, 32, 15, 16, 18, 25, 26, 0, 1, 2, 4, 5};

bool StandardSyntaxParserTokenManager::jjCanMove_0(int hiByte, int i1, int i2,
                                                   int64_t l1, int64_t l2)
{
  switch (hiByte) {
  case 48:
    return ((jjbitVec0[i2] & l2) != 0LL);
  default:
    return false;
  }
}

bool StandardSyntaxParserTokenManager::jjCanMove_1(int hiByte, int i1, int i2,
                                                   int64_t l1, int64_t l2)
{
  switch (hiByte) {
  case 0:
    return ((jjbitVec3[i2] & l2) != 0LL);
  default:
    if ((jjbitVec1[i1] & l1) != 0LL) {
      return true;
    }
    return false;
  }
}

bool StandardSyntaxParserTokenManager::jjCanMove_2(int hiByte, int i1, int i2,
                                                   int64_t l1, int64_t l2)
{
  switch (hiByte) {
  case 0:
    return ((jjbitVec3[i2] & l2) != 0LL);
  case 48:
    return ((jjbitVec1[i2] & l2) != 0LL);
  default:
    if ((jjbitVec4[i1] & l1) != 0LL) {
      return true;
    }
    return false;
  }
}

std::deque<wstring> const
    StandardSyntaxParserTokenManager::jjstrLiteralImages = {
        L"",     nullptr,     nullptr, nullptr, nullptr,   nullptr, nullptr,
        nullptr, nullptr,     nullptr, nullptr, L"\53",    L"\55",  L"\50",
        L"\51",  L"\72",      L"\75",  L"\74",  L"\74\75", L"\76",  L"\76\75",
        L"\136", nullptr,     nullptr, nullptr, nullptr,   L"\133", L"\173",
        nullptr, L"\124\117", L"\135", L"\175", nullptr,   nullptr};
std::deque<wstring> const StandardSyntaxParserTokenManager::lexStateNames = {
    L"Boost", L"Range", L"DEFAULT"};
std::deque<int> const StandardSyntaxParserTokenManager::jjnewLexState = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 0,  -1, -1, -1, -1, 1,  1,  2,  -1, 2,  2,  -1, -1};
std::deque<int64_t> const StandardSyntaxParserTokenManager::jjtoToken = {
    0x3ffffff01LL};
std::deque<int64_t> const StandardSyntaxParserTokenManager::jjtoSkip = {
    0x80LL};

StandardSyntaxParserTokenManager::StandardSyntaxParserTokenManager(
    shared_ptr<CharStream> stream)
{
  input_stream = stream;
}

StandardSyntaxParserTokenManager::StandardSyntaxParserTokenManager(
    shared_ptr<CharStream> stream, int lexState)
    : StandardSyntaxParserTokenManager(stream)
{
  SwitchTo(lexState);
}

void StandardSyntaxParserTokenManager::ReInit(shared_ptr<CharStream> stream)
{
  jjmatchedPos = jjnewStateCnt = 0;
  curLexState = defaultLexState;
  input_stream = stream;
  ReInitRounds();
}

void StandardSyntaxParserTokenManager::ReInitRounds()
{
  int i;
  jjround = 0x80000001;
  for (i = 33; i-- > 0;) {
    jjrounds[i] = 0x80000000;
  }
}

void StandardSyntaxParserTokenManager::ReInit(shared_ptr<CharStream> stream,
                                              int lexState)
{
  ReInit(stream);
  SwitchTo(lexState);
}

void StandardSyntaxParserTokenManager::SwitchTo(int lexState)
{
  if (lexState >= 3 || lexState < 0) {
    throw make_shared<TokenMgrError>(
        L"Error: Ignoring invalid lexical state : " + to_wstring(lexState) +
            L". State unchanged.",
        TokenMgrError::INVALID_LEXICAL_STATE);
  } else {
    curLexState = lexState;
  }
}

shared_ptr<Token> StandardSyntaxParserTokenManager::jjFillToken()
{
  shared_ptr<Token> *const t;
  const wstring curTokenImage;
  constexpr int beginLine;
  constexpr int endLine;
  constexpr int beginColumn;
  constexpr int endColumn;
  wstring im = jjstrLiteralImages[jjmatchedKind];
  curTokenImage = (im == L"") ? input_stream->GetImage() : im;
  beginLine = input_stream->getBeginLine();
  beginColumn = input_stream->getBeginColumn();
  endLine = input_stream->getEndLine();
  endColumn = input_stream->getEndColumn();
  t = Token::newToken(jjmatchedKind, curTokenImage);

  t->beginLine = beginLine;
  t->endLine = endLine;
  t->beginColumn = beginColumn;
  t->endColumn = endColumn;

  return t;
}

shared_ptr<Token> StandardSyntaxParserTokenManager::getNextToken()
{
  shared_ptr<Token> matchedToken;
  int curPos = 0;

  for (;;) {
    try {
      curChar = input_stream->BeginToken();
    } catch (const IOException &e) {
      jjmatchedKind = 0;
      matchedToken = jjFillToken();
      return matchedToken;
    }

    switch (curLexState) {
    case 0:
      jjmatchedKind = 0x7fffffff;
      jjmatchedPos = 0;
      curPos = jjMoveStringLiteralDfa0_0();
      break;
    case 1:
      jjmatchedKind = 0x7fffffff;
      jjmatchedPos = 0;
      curPos = jjMoveStringLiteralDfa0_1();
      break;
    case 2:
      jjmatchedKind = 0x7fffffff;
      jjmatchedPos = 0;
      curPos = jjMoveStringLiteralDfa0_2();
      break;
    }
    if (jjmatchedKind != 0x7fffffff) {
      if (jjmatchedPos + 1 < curPos) {
        input_stream->backup(curPos - jjmatchedPos - 1);
      }
      if ((jjtoToken[jjmatchedKind >> 6] & (1LL << (jjmatchedKind & 077))) !=
          0LL) {
        matchedToken = jjFillToken();
        if (jjnewLexState[jjmatchedKind] != -1) {
          curLexState = jjnewLexState[jjmatchedKind];
        }
        return matchedToken;
      } else {
        if (jjnewLexState[jjmatchedKind] != -1) {
          curLexState = jjnewLexState[jjmatchedKind];
        }
        goto EOFLoopContinue;
      }
    }
    int error_line = input_stream->getEndLine();
    int error_column = input_stream->getEndColumn();
    wstring error_after = L"";
    bool EOFSeen = false;
    try {
      input_stream->readChar();
      input_stream->backup(1);
    } catch (const IOException &e1) {
      EOFSeen = true;
      error_after = curPos <= 1 ? L"" : input_stream->GetImage();
      if (curChar == L'\n' || curChar == L'\r') {
        error_line++;
        error_column = 0;
      } else {
        error_column++;
      }
    }
    if (!EOFSeen) {
      input_stream->backup(1);
      error_after = curPos <= 1 ? L"" : input_stream->GetImage();
    }
    throw make_shared<TokenMgrError>(EOFSeen, curLexState, error_line,
                                     error_column, error_after, curChar,
                                     TokenMgrError::LEXICAL_ERROR);
  EOFLoopContinue:;
  }
EOFLoopBreak:;
}

void StandardSyntaxParserTokenManager::jjCheckNAdd(int state)
{
  if (jjrounds[state] != jjround) {
    jjstateSet[jjnewStateCnt++] = state;
    jjrounds[state] = jjround;
  }
}

void StandardSyntaxParserTokenManager::jjAddStates(int start, int end)
{
  do {
    jjstateSet[jjnewStateCnt++] = jjnextStates[start];
  } while (start++ != end);
}

void StandardSyntaxParserTokenManager::jjCheckNAddTwoStates(int state1,
                                                            int state2)
{
  jjCheckNAdd(state1);
  jjCheckNAdd(state2);
}

void StandardSyntaxParserTokenManager::jjCheckNAddStates(int start, int end)
{
  do {
    jjCheckNAdd(jjnextStates[start]);
  } while (start++ != end);
}
} // namespace org::apache::lucene::queryparser::flexible::standard::parser