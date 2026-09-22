/*
 * Serial commands at 115200 baudrate:
 *   1  all low
 *   2  all high
 *   3  all hi-Z, analyzer should read all low (TXU0104 input pull-downs)
 *   4  walking one
 *   5  frequency fan (default at boot)
 *   6  cycle through 1-5, 2 s each
 *   ?  help and fan frequency table
 */

#include <Arduino.h>
#include <hardware/gpio.h>

#define N_CH          24
#define FAN_TICK_US   100
#define WALK_STEP_US  1000
#define SYNC_US       500
#define AUTO_STEP_MS  2000

#define MODE_LOW   1
#define MODE_HIGH  2
#define MODE_HIZ   3
#define MODE_WALK  4
#define MODE_FAN   5
#define MODE_AUTO  6

static const uint8_t CH_PIN[N_CH] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 26
};

static const char *MODE_NAME[] = {
    "", "all low", "all high", "all hi-z", "walking one", "freq fan", "auto"
};

static uint32_t ch_mask;
static uint8_t  mode = MODE_FAN;

static uint8_t  auto_step;
static uint32_t auto_t0;

static uint32_t fan_tick;
static uint32_t fan_next;
static bool     fan_restart = true;

void drive_all(bool level)
{
    gpio_put_masked(ch_mask, level ? ch_mask : 0);
    for (int i = 0; i < N_CH; i++)
        gpio_set_dir(CH_PIN[i], GPIO_OUT);
}

void release_all()
{
    for (int i = 0; i < N_CH; i++)
        gpio_set_dir(CH_PIN[i], GPIO_IN);
}

void sync_pulse()
{
    drive_all(true);
    delayMicroseconds(SYNC_US);
    gpio_clr_mask(ch_mask);
    delayMicroseconds(SYNC_US);
}

void walk_sweep()
{
    sync_pulse();
    release_all();
    for (int i = 0; i < N_CH; i++) {
        uint32_t bit = 1ul << CH_PIN[i];
        gpio_set_mask(bit);
        gpio_set_dir(CH_PIN[i], GPIO_OUT);
        delayMicroseconds(WALK_STEP_US);
        gpio_clr_mask(bit);
        gpio_set_dir(CH_PIN[i], GPIO_IN);
    }
}

void fan_step()
{
    if (fan_restart) {
        drive_all(false);
        fan_tick = 0;
        fan_next = micros();
        fan_restart = false;
    }

    while ((int32_t)(micros() - fan_next) < 0)
        ;
    fan_next += FAN_TICK_US;

    uint32_t out = 0;
    for (int i = 0; i < N_CH; i++)
        if ((fan_tick / (i + 1)) & 1)
            out |= 1ul << CH_PIN[i];
    gpio_put_masked(ch_mask, out);
    fan_tick++;
}

void begin_step(uint8_t m)
{
    fan_restart = true;
    if (m != MODE_WALK)
        sync_pulse();

    if (m == MODE_LOW)
        drive_all(false);
    else if (m == MODE_HIGH)
        drive_all(true);
    else if (m == MODE_HIZ)
        release_all();
}

void set_mode(uint8_t m)
{
    mode = m;
    Serial.printf("mode %u: %s\n", m, MODE_NAME[m]);

    if (m == MODE_AUTO) {
        auto_step = MODE_LOW;
        auto_t0 = millis();
        Serial.printf("auto: %s\n", MODE_NAME[auto_step]);
        begin_step(auto_step);
    } else {
        begin_step(m);
    }
}

void print_help()
{
    Serial.println();
    Serial.println("la_channel_test");
    Serial.println("1 low  2 high  3 hi-z  4 walk  5 fan  6 auto  ? help");
    Serial.println();
    Serial.println(" ch  gpio  fan freq");
    for (int i = 0; i < N_CH; i++) {
        float hz = 1e6f / (2.0f * (i + 1) * FAN_TICK_US);
        Serial.printf("%3d  GP%-2d  %8.2f Hz\n", i + 1, CH_PIN[i], hz);
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);

    for (int i = 0; i < N_CH; i++) {
        gpio_init(CH_PIN[i]);
        gpio_disable_pulls(CH_PIN[i]);
        ch_mask |= 1ul << CH_PIN[i];
    }
    drive_all(false);

    delay(2000);
    print_help();
    set_mode(MODE_FAN);
}

void loop()
{
    if (Serial.available()) {
        int c = Serial.read();
        if (c >= '1' && c <= '6')
            set_mode(c - '0');
        else if (c == '?')
            print_help();
    }

    if (mode == MODE_AUTO && millis() - auto_t0 >= AUTO_STEP_MS) {
        auto_t0 = millis();
        auto_step = (auto_step == MODE_FAN) ? MODE_LOW : auto_step + 1;
        Serial.printf("auto: %s\n", MODE_NAME[auto_step]);
        begin_step(auto_step);
        return;
    }

    uint8_t run = (mode == MODE_AUTO) ? auto_step : mode;

    if (run == MODE_WALK)
        walk_sweep();
    else if (run == MODE_FAN)
        fan_step();
}