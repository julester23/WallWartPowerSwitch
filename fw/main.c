#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define INA230_ADDR 0x02


int main() {
    i2c_init(i2c_default, 100 * 1000);
    uint8_t rxdata[5], ret;
    ret = i2c_read_blocking(i2c_default, INA230_ADDR, rxdata, 1, false);
    return 0;
}
