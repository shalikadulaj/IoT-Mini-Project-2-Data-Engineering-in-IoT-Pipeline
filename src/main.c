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
 *
 * @author      Kavinda Rathnayake ( kavindamadhujith@gmail.com)
 * 
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

#ifndef EMCUTE_ID
#define EMCUTE_ID ("WeatherStation_3")
#endif

#define EMCUTE_PORT (1883U)
#define EMCUTE_PRIO (THREAD_PRIORITY_MAIN - 1)

// LPS331AP related includes
#include "lpsxxx.h"
#include "lpsxxx_params.h"

// ISL29020 related includes
#include  "isl29020.h"
#include "isl29020-internal.h"
#include "isl29020_params.h"


// Declare the LPS331AP device variable
static lpsxxx_t lpsxxx;

// Declare the ISL29020 device variable
static isl29020_t dev;

static int16_t temp;
static uint16_t pres;
static  int lux;

// Struct that contains sensor values
typedef struct sensors {
    int16_t temperature;
    uint16_t pressure;
    int lightLevel;
} t_sensors;

static char stack[THREAD_STACKSIZE_DEFAULT];

// Thread stacks and handler functions
static char lps331ap_stack_T[THREAD_STACKSIZE_MAIN];
static char lps331ap_stack_P[THREAD_STACKSIZE_MAIN];
static char isl29020_stack[THREAD_STACKSIZE_MAIN];

static msg_t queue[8];


// Function declarations
static void *emcute_thread(void *arg);
static int discon(void);
static void *thread_handler_lps331ap_T(void *arg);
static void *thread_handler_lps331ap_P(void *arg);
static void *thread_handler_isl29020(void *arg);
static int pub(char *topic, char *data, int qos);
static int con(char *addr, int port);
static void sensors_values(t_sensors *sensors);
static int cmd_start(int argc, char **argv);


// Function definitions...

float generate_normal_random(float stddev) {
    float M_PI = 3.1415926535;

    // Box-Muller transform to generate random numbers with normal distribution
    float u1 = rand() / (float)RAND_MAX;
    float u2 = rand() / (float)RAND_MAX;
    float z = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
    
    return stddev * z;
}

float add_noise(float stddev) {
    int num;
    float noise_val = 0;
    
    num = rand() % 100 + 1; // use rand() function to get the random number
    if (num >= 50) {
        // Generate a random number with normal distribution based on a stddev
        noise_val = generate_normal_random(stddev);
    }
    return noise_val;
}

static void *emcute_thread(void *arg) {
    (void)arg;
    emcute_run(EMCUTE_PORT, EMCUTE_ID);
    return NULL; /* should never be reached */
}

static int discon(void) {
    int res = emcute_discon();
    if (res == EMCUTE_NOGW) {
        puts("error: not connected to any broker");
        return 1;
    } else if (res != EMCUTE_OK) {
        puts("error: unable to disconnect");
        return 1;
    }
    puts("Disconnect successful");
    return 0;
}

static void *thread_handler_lps331ap_T(void *arg) {
    
    (void)arg; 
    
    if (lpsxxx_init(&lpsxxx, &lpsxxx_params[0]) != LPSXXX_OK) {
         
        puts("Error initializing the LPS331AP sensor");
        
        
    return NULL;
    }
    

    while (1) {
        
      lpsxxx_read_temp(&lpsxxx, &temp);
        
      temp = (temp/100) + add_noise((temp/100));  
        
      //printf("%d\n",temp);
    
      ztimer_sleep(ZTIMER_MSEC, 3000);
    }

    return NULL;
}

static void *thread_handler_lps331ap_P(void *arg) {
    (void)arg;
    
   // static uint16_t pres;


    while (1) {
        
       lpsxxx_read_pres(&lpsxxx, &pres);
       
       pres = pres + add_noise(pres);   
        
       //printf("Pressure: %uhPa\n", pres);
        
        ztimer_sleep(ZTIMER_MSEC, 3000);
    }

    return NULL;
}

static void *thread_handler_isl29020(void *arg) {
   
    (void)arg;


    
    while (1) {
        
       

        // Read the lux value
       if( isl29020_read(&dev) != 0){
            
         lux = lux + add_noise(lux); 
           
         // printf("ligh level: %i lux\n", lux); 
           
       }

        // Sleep for 2 seconds before the next read
        xtimer_sleep(2);
    }


    return NULL;
}

static int pub(char *topic, char *data, int qos) {
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_0;

    switch (qos) {
        case 1:
            flags |= EMCUTE_QOS_1;
            break;
        case 2:
            flags |= EMCUTE_QOS_2;
            break;
        default:
            flags |= EMCUTE_QOS_0;
            break;
    }

    t.name = topic;
    if (emcute_reg(&t) != EMCUTE_OK) {
        puts("error: unable to obtain topic ID");
        return 1;
    }

    if (emcute_pub(&t, data, strlen(data), flags) != EMCUTE_OK) {
        printf("error: unable to publish data to topic '%s [%i]'\n", t.name, (int)t.id);
        return 1;
    }

    printf("published %s on topic %s\n", data, topic);

    return 0;
}

static int con(char *addr, int port) {


    sock_udp_ep_t gw = {.family = AF_INET6, .port = EMCUTE_PORT};
    gw.port = port;

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, addr) == NULL) {
        printf("error parsing IPv6 address\n");
        return 1;
    }

    if (emcute_con(&gw, true, NULL, NULL, 0, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", addr, port);
        
        return 1;
    }
    printf("Successfully connected to gateway at [%s]:%i\n", addr, port);
    return 0;
}

static void sensors_values(t_sensors *sensors) {
    
    sensors->temperature = temp;
    sensors->pressure = pres;
    sensors->lightLevel = lux;
}

static int cmd_start(int argc, char **argv) {
    if (argc < 4) {
        printf("usage: %s <address> <port> <id>\n", argv[0]);
        return 1;
    }

    t_sensors sensors;
    // name of the topic
    char topic[32];
   sprintf(topic,"sensor/node3");

    // json that it will be published
    char json[256];

    while (1) {
        // it tries to connect to the gateway
        if (con(argv[1], atoi(argv[2]))) {
            continue;
        }

        char datetime[20];

        // Get the current time as a time_t value
        time_t currentTime = time(NULL);

        // Convert the time_t value to a struct tm in the local timezone
        struct tm *timeinfo = localtime(&currentTime);

        if (timeinfo == NULL) {
            perror("localtime");
            return 1;
        }

        struct tm modifiedTimeinfo = *timeinfo;

        modifiedTimeinfo.tm_year += 2023 - 1970;
        modifiedTimeinfo.tm_mon += 11;
        modifiedTimeinfo.tm_mday += 28;
        modifiedTimeinfo.tm_hour += 0;
        

        int c = strftime(datetime, sizeof(datetime), "%Y-%m-%d %T", &modifiedTimeinfo);

        if (c == 0) {
            printf("Error! Invalid format\n");
            return 0;
        }

        sensors_values(&sensors);

/*
        // fills the json document
        sprintf(json, "{\"siteId\": \"%d\",\"node\": \"%d\", \"timestamp\": \"%s\", \"temperature\": "
                      "\"%d\", \"pressure\": \"%d\", \"lightLevel\": \"%d\"}", 
                1,atoi(argv[3]), datetime, sensors.temperature, sensors.pressure, sensors.lightLevel);
*/        
                // fills the json document
        sprintf(json, "{\"timestamp\": \"%s\",\"site\": \"%d\", \"node\": \"%d\", \"temperature\": "
                      "\"%d\", \"pressure\": \"%d\"}", 
                datetime,1, atoi(argv[3]), sensors.temperature, sensors.pressure);

        // publish to the topic
        pub(topic, json, 0);

        // it disconnects from the gateway
        discon();

        // it sleeps for five seconds
        ztimer_sleep(ZTIMER_SEC, 10);
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    {"start", "Start the station", cmd_start},
    {NULL, NULL, NULL}
};

int main(void) {
    
     puts("Weather centre application\n");
     puts("Initializing device");
      //Initialize the LPS331AP sensor
     lpsxxx_init(&lpsxxx, &lpsxxx_params[0]);
    
     // Initialize the lux sensor
    
     isl29020_enable(&dev);
     
    
     if (isl29020_init(&dev, &isl29020_params[0]) == 0) {
        puts("[OK]\n");
      
    } else {
        puts("[Failed]\n");
       
    }
     
    
      
     xtimer_sleep(5);
    

    // The main thread needs a message queue for running `ping6`
    msg_init_queue(queue, ARRAY_SIZE(queue));

    // Start the emcute thread
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");

    // Create threads for LPS331AP and ISL29020
    thread_create(lps331ap_stack_T, sizeof(lps331ap_stack_T), THREAD_PRIORITY_MAIN - 1, 0,
                  thread_handler_lps331ap_T, NULL, "lps331ap_thread_Temperature");

    thread_create(lps331ap_stack_P, sizeof(lps331ap_stack_P), THREAD_PRIORITY_MAIN - 2, 0,
                  thread_handler_lps331ap_P, NULL, "lps331ap_thread_Pressure");

    thread_create(isl29020_stack, sizeof(isl29020_stack), THREAD_PRIORITY_MAIN - 3, 0,
                  thread_handler_isl29020, NULL, "isl29020_LUX");

    // Start the shell
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    // This should never be reached
    return 0;
}
