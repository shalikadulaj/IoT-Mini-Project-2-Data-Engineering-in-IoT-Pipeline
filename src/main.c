
```
/*
 * Copyright (C) 2023 University of Oulu - IOT
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top-level
 * directory for more details.
 */

/**
 * @ingroup     Sensing_Layer
 * @{
 *
 * @file
 * @brief       Firmware Development to Read Weather Data using RIOT
 * @author      Kavinda Rathnayake ( kavindamadhujith@gmail.com)
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "timex.h"
#include "thread.h"
#include "mutex.h"
#include "ztimer.h"
#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "periph/wdt.h"

// Sensor related includes
#include "lpsxxx.h"
#include "lpsxxx_params.h"
#include "isl29020.h"
#include "isl29020-internal.h"
#include "isl29020_params.h"

#ifndef EMCUTE_ID
#define EMCUTE_ID ("WeatherStation_3")
#endif

#define EMCUTE_PORT (1883U)
#define EMCUTE_PRIO (THREAD_PRIORITY_MAIN - 1)

// Device variable declarations
static lpsxxx_t lpsxxx;
static isl29020_t dev;

// Sensor reading variables
static int16_t temp;
static uint16_t pres;
static int lux;

// Sensor values struct
typedef struct sensors {
    int16_t temperature;
    uint16_t pressure;
    int lightLevel;
} t_sensors;

// Thread stacks and handler functions
static char stack[THREAD_STACKSIZE_DEFAULT];
static char lps331ap_stack_T[THREAD_STACKSIZE_MAIN];
static char lps331ap_stack_P[THREAD_STACKSIZE_MAIN];
static char isl29020_stack[THREAD_STACKSIZE_MAIN];
static msg_t queue[8];

// Function prototypes
static void *emcute_thread(void *arg);
static int discon(void);
static void *thread_handler_lps331ap_T(void *arg);
static void *thread_handler_lps331ap_P(void *arg);
static void *thread_handler_isl29020(void *arg);
static int pub(char *topic, char *data, int qos);
static int con(char *addr, int port);
static void sensors_values(t_sensors *sensors);
static int cmd_start(int argc, char **argv);
static float generate_normal_random(float stddev);
static float add_noise(float stddev);

// Function implementations...

static void *emcute_thread(void *arg) {
    (void)arg;
    emcute_run(EMCUTE_PORT, EMCUTE_ID);
    return NULL; // should never be reached
}

// Other function implementations...

int main(void) {
    puts("Weather centre application\n");
    puts("Initializing device");

    // Initialize the LPS331AP sensor
    lpsxxx_init(&lpsxxx, &lpsxxx_params[0]);
    
    // Initialize the ISL29020 sensor
    isl29020_enable(&dev);
    if (isl29020_init(&dev, &isl29020_params[0]) == 0) {
        puts("[OK]\n");
    } else {
        puts("[Failed]\n");
    }

    xtimer_sleep(5);

    // Initialize message queue for main thread
    msg_init_queue(queue, ARRAY_SIZE(queue));

    // Start the EMCUTE thread for MQTT-SN communication
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0, emcute_thread, NULL, "emcute");

    // Create threads for LPS331AP and ISL29020 sensors
    thread_create(lps331ap_stack_T, sizeof(lps331ap_stack_T), THREAD_PRIORITY_MAIN - 1, 0,
                  thread_handler_lps331ap_T, NULL, "lps331ap_thread_Temperature");
    thread_create(lps331ap_stack_P, sizeof(lps331ap_stack_P), THREAD_PRIORITY_MAIN - 2, 0,
                  thread_handler_lps331ap_P, NULL, "lps331ap_thread_Pressure");
    thread_create(isl29020_stack, sizeof(isl29020_stack), THREAD_PRIORITY_MAIN - 3, 0,
                  thread_handler_isl29020, NULL, "isl29020_LUX");

    // Start the shell
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0; // This should never be reached
}

// @}
