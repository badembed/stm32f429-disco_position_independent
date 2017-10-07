/**
  ******************************************************************************
  * @file    Template/main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_l3gd20.h"

#include <stdio.h>

/** @addtogroup Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define WRONG_SECTOR 0xffff
#define SUCCESS 1
#define FAIL    0
#define OLD_ADDR 0x08000000
#define NEW_ADDR 0x08020000

// increment MAGIC for second test
#define MAGIC    1

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 0xffffffff - erase value
int const magic __attribute__((__section__(".mysection"))) = 0xffffffff;
void (*p_update)(void);
int *p_magic = (int *)0x080201ac;

/* Private function prototypes -----------------------------------------------*/

uint32_t GetSector(uint32_t Address);
void user_LCD_init(void);
int copy_code(void);
void debug_out(char *str);
void first_update(void);
void second_update(void);
int erase_system(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

uint32_t g_data = 0;
static uint32_t s_data = 0;
const uint32_t c_data = 1;

#define DEF1 0xffffffff
#define DEF2 0

void test_global(uint32_t val) {
    g_data += val;
}

void test_static(uint32_t val) {
    s_data += val;
}

uint32_t test_global_const(void) {
    return c_data;
}

void test_local_const(void) {
    debug_out("test_local_const");
}

uint32_t test_define(void) {
    return(DEF1 + DEF2);
}

extern void Reset_Handler(void);

void main(void)
{
    uint32_t tmp;
    uint32_t def;

    user_LCD_init();

    test_global(1);
    test_static(1);
    tmp = test_global_const();
    test_local_const();
    def = test_define();

    def += tmp;

    __disable_irq();
    user_LCD_init();

    if(*p_magic != MAGIC) {
        debug_out("first_update");
        copy_code();
        SCB->VTOR = NEW_ADDR;
        p_update = (&first_update - OLD_ADDR) + NEW_ADDR;
    } else {
        SCB->VTOR = NEW_ADDR;
        debug_out("second_update");
        //p_update = NEW_ADDR + Reset_Handler;
        p_update = (&second_update - OLD_ADDR) + NEW_ADDR;
    }

    p_update();

    debug_out("MAIN infinite");
    while (1);
}

void first_update(void)
{
    debug_out("COPY finished");
    while(1);
}

void second_update(void)
{
    uint32_t tmp;
    uint32_t def;

    test_global(1);
    test_static(1);
    tmp = test_global_const();
    test_local_const();
    def = test_define();

    def += tmp;

    debug_out("APP in new partition");
    erase_system();
    debug_out("first partition was erased");

    LCD_DisplayStringLine(LCD_LINE_8,
                          (uint8_t *) "IT's LIVE");
    while(1);
}

int erase_system(void)
{
    uint32_t sector;

    FLASH_Unlock();

    sector = GetSector(ADDR_FLASH_SECTOR_0);
    FLASH_EraseSector(sector, VoltageRange_3);

    sector = GetSector(ADDR_FLASH_SECTOR_1);
    FLASH_EraseSector(sector, VoltageRange_3);

    sector = GetSector(ADDR_FLASH_SECTOR_2);
    FLASH_EraseSector(sector, VoltageRange_3);

    sector = GetSector(ADDR_FLASH_SECTOR_3);
    FLASH_EraseSector(sector, VoltageRange_3);

    sector = GetSector(ADDR_FLASH_SECTOR_4);
    FLASH_EraseSector(sector, VoltageRange_3);

    FLASH_Lock();
}

int copy_code(void)
{
    uint32_t new_code_addr = NEW_ADDR;
    uint32_t *old_code_addr = (uint32_t *)OLD_ADDR;

    debug_out("START");
    FLASH_Unlock();

    // 16 Kbytes
    uint32_t sector = GetSector(new_code_addr);

    debug_out("ERASE");
    if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE) {
        debug_out("ERROR ERASE");
        while(1);
    }

    debug_out("WRITE");
    for(int i = 0; i < ((64*1024)/4); i++) {
        if(new_code_addr == p_magic) {
            new_code_addr += 4;
            continue;
        }
        FLASH_ProgramWord(new_code_addr, *(old_code_addr+i));
        new_code_addr += 4;
    }

    FLASH_ProgramWord(p_magic, MAGIC);

    FLASH_Lock();

    debug_out("CHECK");
    new_code_addr = NEW_ADDR;

/*
    for(int i = 0; i < ((1*1024)/4); i++) {
        if(*(uint32_t *)new_code_addr != *(old_code_addr+i)) {
            debug_out("NOT SUCCESS");
            return FAIL;
        }
        new_code_addr += 4;
    }
*/

    SCB->VTOR = NEW_ADDR;

    debug_out("SUCCESS");
    return SUCCESS;
}

void debug_out(char *str)
{
    LCD_DisplayStringLine(LCD_LINE_6,
                          (uint8_t *) str);
}

void user_LCD_init(void)
{
    LCD_Init();
    LCD_LayerInit();

    LTDC_Cmd(ENABLE);

    LCD_SetLayer(LCD_FOREGROUND_LAYER);
    LCD_Clear(LCD_COLOR_BLACK);
    LCD_SetBackColor(LCD_COLOR_BLACK);
    LCD_SetTextColor(LCD_COLOR_WHITE);

    LCD_SetFont(&Font8x12);

    LCD_DisplayStringLine(LCD_LINE_3,
                          (uint8_t *)" The BADEMBED generation ");
}

uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_Sector_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_Sector_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_Sector_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_Sector_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_Sector_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_Sector_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_Sector_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_Sector_7;
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_Sector_8;
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_Sector_9;
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_Sector_10;
    }
    else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    {
        sector = WRONG_SECTOR;
    }

    return sector;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
