#ifndef CONSTANT_HPP
#define CONSTANT_HPP

int const MAX_MSG_LENGTH {1024 * 2};
int const HEAD_TOTAL_LENGTH{4};
int const HEAD_ID_LENGTH{2};
int const HEAD_DATA_LENGTH{2};
int const MAX_RECVQUE_SIZE{10000};
int const MAX_SENDQUE_SIZE{1000};

enum MSG_ID_TYPE{
    MSG_HEELO_WORLD = 1001,
};

#endif