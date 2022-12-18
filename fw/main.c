#include <stdio.h>
#include <stdlib.h>

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "shift.pio.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usbtmc_app.h"

uint8_t digits[] = {
    0b11111100, // 0
    0b01100000, // 1
    0b11011010, // 2 
    0b11110011, // 3
    0b01100110, // 4
    0b10110110, // 5
    0b10111110, // 6
    0b11100000, // 7
    0b11111110, // 8
    0b11110110, // 9
};

// Channel 0 is GPIO26
#define CAPTURE_CHANNEL 0
#define CAPTURE_DEPTH 100

uint8_t capture_buf[CAPTURE_DEPTH];

void led_indicator_pulse(void) {
    static uint32_t last = 0;
    static bool state = false;

    if ((board_millis() - last) < 500)
    {
        state = !state;
        board_led_write(state);
    }
}


void pio_print(PIO pio, int32_t num, uint8_t dp) {
    // TODO: dp to turn on DP light at 1s, 10s, 100s place
    pio_sm_put(
        pio, 0,
        (
             (digits[(num/100) % 10] << 16) |
             (digits[(num/10) % 10] << 8) |
             (digits[num % 10])
        )
    );
}
int main() {

    // stdio_init_all();
    board_init(); //tinyusb board init
    tusb_init();

    PIO pio = pio0;
    uint offset = pio_add_program(pio0, &shiftout_program);
    shiftout_program_init(pio0, 0, offset, 8, 9, 10);

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    adc_gpio_init(27);

    adc_init();

    adc_set_round_robin(0x3);

    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    adc_fifo_setup(
            true,    // Write each completed conversion to the sample FIFO
            true,    // Enable DMA data request (DREQ)
            1,       // DREQ (and IRQ) asserted when at least 1 sample present
            false,   // We won't see the ERR bit because of 8 bit reads; disable.
            true     // Shift each sample to 8 bits when pushing to FIFO
            );

    adc_set_clkdiv(480000); // 100Hz

    // Set up the DMA to start transferring data as soon as it appears in FIFO
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);

    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);

    dma_channel_configure(
            dma_chan,
            &cfg,
            capture_buf,    // dst
            &adc_hw->fifo,  // src
            CAPTURE_DEPTH,  // transfer count
            true            // start immediately
    );

    adc_run(true);
    // Once DMA finishes, stop any new conversions from starting, and clean up
    // the FIFO in case the ADC was still mid-conversion.
    dma_channel_wait_for_finish_blocking(dma_chan);


    int i = 0;
    int num = capture_buf[0];

    while (true) {

        //printf("Stack: %d\n", adc_fifo_get_level());
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V

        //const float conversion_factor = 3.3f / (1 << 12);
        //uint16_t result = adc_fifo_get();
        //
        //printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);

        tud_task(); // tinyusb device task
        usbtmc_app_task_iter();

        i++;
        num = capture_buf[1];

        pio_print(pio0, capture_buf[0], 0);

        dma_channel_wait_for_finish_blocking(dma_chan);
        dma_channel_set_write_addr(dma_chan, capture_buf, 1);

        //adc_fifo_drain();
        //i %= 10;
    }

    return 0;
}

