#include "nn.h"
#include "CUtils.h"

#include <time.h>
#include <math.h>

double CUtils::rand() {
    return ::rand() % 10000 / 10000.0;
}
