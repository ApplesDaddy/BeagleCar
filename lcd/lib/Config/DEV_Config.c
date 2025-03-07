/*****************************************************************************
* | File        :   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* | This version:   V2.0
* | Date        :   2019-07-08
* | Info        :   Basic version
*
******************************************************************************/
#include "DEV_Config.h"

#if USE_DEV_LIB
#include <lgpio.h>

int GPIO_Handle1;
int GPIO_Handle2;
int SPI_Handle;

typedef struct {
    int gpiochip;   // The GPIO chip number (e.g., 1, 2)
    int handle;     // The GPIO handle, after being claimed
    int line;       // The line number within the gpiochip
} DEV_GPIO_Pin;

// Define the GPIO pins based on your BeagleY-AI mappings
// DEV_GPIO_Pin LCD_DC_PIN  = {1, -1, 42}; // DS is GPIO25 on gpiochip1 line 42
// DEV_GPIO_Pin LCD_RST_PIN = {1, -1, 33}; // RST is GPIO27 on gpiochip1 line 33
// DEV_GPIO_Pin LCD_BL_PIN  = {2, -1, 11}; // BL is GPIO18 on gpiochip2 line 11
DEV_GPIO_Pin LCD_DC_PIN  = {1, -1, 33}; // DS  is GPIO27 on gpiochip1 line 33
DEV_GPIO_Pin LCD_RST_PIN = {1, -1, 41}; // RST is GPIO22 on gpiochip1 line 41
DEV_GPIO_Pin LCD_BL_PIN  = {2, -1, 18}; // BL  is GPIO13 on gpiochip2 line 18


// Define the Pin constants
#define LCD_RST 1
#define LCD_DC  2
#define LCD_BL  3

// Array to map Pin constants to DEV_GPIO_Pin structures
DEV_GPIO_Pin* DEV_GPIOS[4]; // Index 0 unused

#endif

void DEV_SetBacklight(UWORD Value)
{
#ifdef USE_DEV_LIB
    DEV_Digital_Write(LCD_BL, Value);
#endif
}

/*****************************************
                    GPIO
*****************************************/
void DEV_Digital_Write(UWORD Pin, UBYTE Value)
{
#ifdef USE_DEV_LIB
    DEV_GPIO_Pin* gpio_pin = DEV_GPIOS[Pin];
    if (gpio_pin == NULL) {
        printf("Invalid GPIO Pin: %d\n", Pin);
        return;
    }
    lgGpioWrite(gpio_pin->handle, gpio_pin->line, Value);
#endif
}

UBYTE DEV_Digital_Read(UWORD Pin)
{
    UBYTE Read_value = 0;
#ifdef USE_DEV_LIB
    DEV_GPIO_Pin* gpio_pin = DEV_GPIOS[Pin];
    if (gpio_pin == NULL) {
        printf("Invalid GPIO Pin: %d\n", Pin);
        return 0;
    }
    Read_value = lgGpioRead(gpio_pin->handle, gpio_pin->line);
#endif
    return Read_value;
}

void DEV_GPIO_Mode(UWORD Pin, UWORD Mode)
{
#ifdef USE_DEV_LIB
    DEV_GPIO_Pin* gpio_pin = DEV_GPIOS[Pin];
    if (gpio_pin == NULL) {
        printf("Invalid GPIO Pin: %d\n", Pin);
        return;
    }
    if(Mode == 0 || Mode == LG_SET_INPUT){
        lgGpioClaimInput(gpio_pin->handle, LFLAGS, gpio_pin->line);
    } else {
        lgGpioClaimOutput(gpio_pin->handle, LFLAGS, gpio_pin->line, LG_LOW);
    }
#endif   
}

/**
 * delay x ms
**/
void DEV_Delay_ms(UDOUBLE xms)
{
#ifdef USE_DEV_LIB  
    lguSleep(xms/1000.0);
#endif
}

static void DEV_GPIO_Init(void)
{
    DEV_GPIO_Mode(LCD_RST, 1);
    DEV_GPIO_Mode(LCD_DC, 1);
    DEV_GPIO_Mode(LCD_BL, 1);
}

UBYTE DEV_ModuleInit(void)
{
    printf("Entering DEV_ModuleInit...\n");

#ifdef USE_DEV_LIB
    // printf("  --> USE_DEV_LIB\n");

    // Open gpiochip1
    // printf("--> OPENING GPIO BANK 1...\n");
    GPIO_Handle1 = lgGpiochipOpen(1);
    if (GPIO_Handle1 < 0)
    {
        printf("gpiochip1 Export Failed\n");
        return -1;
    }

    // Open gpiochip2
    // printf("--> OPENING GPIO BANK 2...\n");
    GPIO_Handle2 = lgGpiochipOpen(2);
    if (GPIO_Handle2 < 0)
    {
        printf("gpiochip2 Export Failed\n");
        return -1;
    }

    // Assign handles to pins
    LCD_DC_PIN.handle  = GPIO_Handle1;
    LCD_RST_PIN.handle = GPIO_Handle1;
    LCD_BL_PIN.handle  = GPIO_Handle2;

    // Initialize the DEV_GPIOS array
    DEV_GPIOS[LCD_RST] = &LCD_RST_PIN;
    DEV_GPIOS[LCD_DC]  = &LCD_DC_PIN;
    DEV_GPIOS[LCD_BL]  = &LCD_BL_PIN;

    // Open SPI channel
    SPI_Handle = lgSpiOpen(0, 0, 25000000, 0);
    // printf("  --> SPI Handle: %d\n", SPI_Handle);
    if (SPI_Handle < 0) {
        printf("Unable to open SPI channel via lgSpiOpen. Handle = %d\n", SPI_Handle);
        perror("Unable to open SPI");
        return -1;
    }
    DEV_GPIO_Init();

#else
    printf("  --> OOPS!\n");
#endif

    // printf("  --> Done DEV_ModuleInit()\n");
    return 0;
}

void DEV_SPI_WriteByte(uint8_t Value)
{
#ifdef USE_DEV_LIB 
    lgSpiWrite(SPI_Handle, (char*)&Value, 1);
#endif
}

void DEV_SPI_Write_nByte(uint8_t *pData, uint32_t Len)
{
#ifdef USE_DEV_LIB 
    lgSpiWrite(SPI_Handle, (char*)pData, Len);
#endif
}

void DEV_ModuleExit(void)
{
#ifdef USE_DEV_LIB 
    lgSpiClose(SPI_Handle);
    lgGpiochipClose(GPIO_Handle1);
    lgGpiochipClose(GPIO_Handle2);
#endif
}
