#include "utils.h"

int get_number_sign(int number)
{
    if (number > 0) return +1;
    if (number < 0) return -1;
    return 0;
}
