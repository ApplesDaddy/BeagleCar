// source: gpio_statemachine_demo

#include "hal/rotary_encoder.h"
#include "hal/gpio.h"
#include "util/common_funcs.h"

#include <stdatomic.h>
#include <pthread.h>


// GPIO constants
#define GPIO_CHIP GPIO_CHIP_2
#define GPIO_BTN_CHIP GPIO_CHIP_0
#define GPIO_LINE_NUMBER_A 7    // GPIO16
#define GPIO_LINE_NUMBER_B 8    // GPIO17
#define GPIO_LINE_NUMBER_BTN 10    // GPIO24 TODO

#define INIT_CHECK() { if(!initialized) { rot_encoder_init(); } }


static bool initialized = false;
static struct GpioLine* line_a = NULL;
static struct GpioLine* line_b = NULL;
static struct GpioLine* line_btn = NULL;
static atomic_int counter = 0;

static atomic_bool stop_thread = false;
static pthread_t pid;

static int max_counter = 500;
static int min_counter = 0;
static int step = 1;


// state machine
static bool clkwise = false;
struct stateEvent
{
    struct state* next_state;
    void (*action)();
};

struct state
{
    struct stateEvent rising_a;
    struct stateEvent rising_b;

    struct stateEvent falling_a;
    struct stateEvent falling_b;
};

static void set_counter_clkwise(void);
static void set_clkwise(void);
static void handle_3_to_0(void);
static void handle_1_to_0(void);
struct state states[] = {
    {   // state 0
        // a: high
        .rising_a = {&states[0], NULL},
        .falling_a = {&states[1], set_clkwise},
        // b: high
        .rising_b = {&states[0], NULL},
        .falling_b = {&states[3], set_counter_clkwise},
    },
    {   // state 1
        // a: low
        .rising_a = {&states[0], handle_1_to_0},
        .falling_a = {&states[1], NULL},
        // b: high
        .rising_b = {&states[1], NULL},
        .falling_b = {&states[2], NULL},
    },
    {   // state 2
        // a: low
        .rising_a = {&states[3], NULL},
        .falling_a = {&states[2], NULL},
        // b: low
        .rising_b = {&states[1], NULL},
        .falling_b = {&states[2], NULL},
    },
    {   // state 3
        // a: high
        .rising_a = {&states[3], NULL},
        .falling_a = {&states[2], NULL},
        // b: low
        .rising_b = {&states[0], handle_3_to_0},
        .falling_b = {&states[3], NULL},
    }
};
struct state* current_state = &states[0];

// ==================================== private helper functions ===================================
static void set_counter_clkwise(void)
{ clkwise = false; }
static void set_clkwise(void)
{ clkwise = true; }
static void handle_1_to_0(void)
{
    if(!clkwise)
    {
        counter -= step;
        counter = counter < min_counter? min_counter : counter;
    }
}
static void handle_3_to_0(void)
{
    if(clkwise)
    {
        counter += step;
        counter = counter > max_counter? max_counter : counter;
    }
}


void* rot_encoder_state_thread(void* args)
{
    INIT_CHECK()
    (void)args; // silence warning
    do
    {
        struct gpiod_line_bulk bulk_events;
        int num_events = gpio_wait_line_change(line_a, line_b, &bulk_events);

        for(int i=0; i<num_events; i++)
        {
            // Get the line event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulk_events, i);
            unsigned int line_number = gpiod_line_offset(line_handle);

            struct gpiod_line_event event;
            int ret = gpiod_line_event_read(line_handle, &event);
            if(ret == -1)
            {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }

            // make sure its this line
            if(line_number != GPIO_LINE_NUMBER_A && line_number != GPIO_LINE_NUMBER_B)
            { continue; }

            // handle event + update state
            bool is_rising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
            struct stateEvent* state_event;
            if(is_rising)
            {
                state_event = line_number == GPIO_LINE_NUMBER_A? &current_state->rising_a :
                                                                 &current_state->rising_b;
            }
            else
            {
                state_event = line_number == GPIO_LINE_NUMBER_A? &current_state->falling_a :
                                                                 &current_state->falling_b;
            }

            if(state_event->action)
            { state_event->action(); }

            current_state = state_event->next_state;
        }
    } while(!stop_thread);

    return NULL;
}


// ======================================= public functions ========================================
void rot_encoder_init(void)
{
    line_a = gpio_open(GPIO_CHIP, GPIO_LINE_NUMBER_A);
    line_b = gpio_open(GPIO_CHIP, GPIO_LINE_NUMBER_B);

    line_btn = gpio_open(GPIO_BTN_CHIP, GPIO_LINE_NUMBER_BTN);
    gpio_set_input(line_btn);

    pthread_create(&pid, NULL, &rot_encoder_state_thread, NULL);

    initialized = true;
}
void rot_encoder_cleanup(void)
{
    stop_thread = true;
    pthread_join(pid, NULL);

    gpio_close(line_a);
    gpio_close(line_b);
    gpio_close(line_btn);

    initialized = false;
}
void rot_encoder_set_step(int count)
{ step = count; }

int rot_encoder_get_val(void)
{
    INIT_CHECK()
    return counter;
}

void rot_encoder_set_min_max(unsigned int min, unsigned int max)
{
    INIT_CHECK()
    min_counter = min;
    max_counter = max;
}
void rot_encoder_set_counter(int count)
{
    INIT_CHECK()
    counter = count;
}

bool rot_encoder_is_pressed(void)
{
    INIT_CHECK()
    return !gpio_get_val(line_btn); // 0 when pressed, else 1
}
