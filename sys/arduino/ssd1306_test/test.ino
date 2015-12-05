


#include <Arduino.h>
#include <SPI.h>
#include "u8x8.h"



/*=============================================*/
/* callbacks */

extern "C" uint8_t u8x8_gpio_and_delay_arduino(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t i;
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      /* new code */
      for( i = 0; i < U8X8_PIN_CNT; i++ )
	if ( u8x8->pins[i] != U8X8_PIN_NONE )
	  pinMode(u8x8->pins[i], OUTPUT);
	  
      /* old code, can be removed */
      pinMode(8, OUTPUT);
      pinMode(9, OUTPUT);
      pinMode(10, OUTPUT);
      pinMode(11, OUTPUT);
      pinMode(13, OUTPUT);
      break;
  
    case U8X8_MSG_DELAY_MILLI:
      delay(arg_int);
      break;
      
    case U8X8_MSG_GPIO_DC:
	digitalWrite(9, arg_int);
	break;
	
    case U8X8_MSG_GPIO_CS:
	digitalWrite(10, arg_int);
	break;
	
    case U8X8_MSG_GPIO_RESET:
	digitalWrite(8, arg_int);
	break;
	
    case U8X8_MSG_GPIO_CLOCK:
	digitalWrite(13, arg_int);
	break;
      
    case U8X8_MSG_GPIO_DATA:
	digitalWrite(11, arg_int);
	break;
      
    default:
      return 0;
  }
  return 1;
}



extern "C" uint8_t u8x8_byte_arduino_hw_spi(u8x8_t *u8g2, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t *data;
 
  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:

      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 )
      {
	SPI.transfer((uint8_t)*data);
	data++;
	arg_int--;
      }
      break;
    case U8X8_MSG_BYTE_INIT:
      /* disable chipselect */
      u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_disable_level);
      /* no wait required here */
      
      /* for SPI: setup correct level of the clock signal */
      digitalWrite(13, u8g2->display_info->sck_takeover_edge);
      break;
    case U8X8_MSG_BYTE_SET_DC:
      u8x8_gpio_SetDC(u8g2, arg_int);
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      SPI.begin();
      SPI.setClockDivider( SPI_CLOCK_DIV2 );
      SPI.setDataMode(SPI_MODE0);
      SPI.setBitOrder(MSBFIRST);
      
      u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_enable_level);  
      u8g2->gpio_and_delay_cb(u8g2, U8X8_MSG_DELAY_NANO, u8g2->display_info->post_chip_enable_wait_ns, NULL);
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      u8g2->gpio_and_delay_cb(u8g2, U8X8_MSG_DELAY_NANO, u8g2->display_info->pre_chip_disable_wait_ns, NULL);
      u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_disable_level);
      SPI.end();
      break;
    case U8X8_MSG_BYTE_SET_I2C_ADR:
      break;
    case U8X8_MSG_BYTE_SET_DEVICE:
      break;
    default:
      return 0;
  }
  return 1;
}

/*=============================================*/
/* interface setup procedures */

/*
u8x8_SetPin(u8x8,pin,val)

values for "pin":
U8X8_PIN_D0
U8X8_PIN_CLOCK
U8X8_PIN_D1
U8X8_PIN_DATA
U8X8_PIN_D2
U8X8_PIN_D3
U8X8_PIN_D4
U8X8_PIN_D5
U8X8_PIN_D6
U8X8_PIN_D7
U8X8_PIN_E
U8X8_PIN_CS
U8X8_PIN_DC
U8X8_PIN_RESET

values vor "val":
any number or:
U8X8_PIN_NONE
*/


/*
  use U8X8_PIN_NONE as value for "reset", if there is no reset line
*/
void u8x8_Setup_SW_4Wire_SPI(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t clock, uint8_t data, uint8_t cs, uint8_t dc, uint8_t reset)
{
  /* setup defaults and reset pins to U8X8_PIN_NONE */
  u8x8_SetupDefaults(u8x8);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_CLOCK, clock);
  u8x8_SetPin(u8x8, U8X8_PIN_DATA, data);
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_DC, dc);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);

  /* setup specific callbacks */
  u8x8->display_cb = display_cb;
  u8x8->cad_cb = u8x8_cad_001;
  u8x8->byte_cb = u8x8_byte_8bit_sw_spi;
  u8x8->gpio_and_delay_cb = u8x8_gpio_and_delay_arduino;

  /* setup display info */
  u8x8_display_Setup(u8x8);

}

/*
  use U8X8_PIN_NONE as value for "reset", if there is no reset line
*/
void u8x8_Setup_HW_4Wire_SPI(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t cs, uint8_t dc, uint8_t reset)
{
  /* setup defaults and reset pins to U8X8_PIN_NONE */
  u8x8_SetupDefaults(u8x8);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_DC, dc);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);

  /* setup specific callbacks */
  u8x8->display_cb = display_cb;
  u8x8->cad_cb = u8x8_cad_001;
  u8x8->byte_cb = u8x8_byte_arduino_hw_spi;
  u8x8->gpio_and_delay_cb = u8x8_gpio_and_delay_arduino;

  /* setup display info */
  u8x8_display_Setup(u8x8);

}

/*=============================================*/
/* display setup procedures */

void u8x8_Setup_UC1701_DOGS102(u8x8_t *u8x8)
{
  /* setup defaults and reset pins to U8X8_PIN_NONE */
  u8x8_SetupDefaults(u8x8);
  
  /* setup specific callbacks */
  u8x8->display_cb = u8x8_d_uc1701_dogs102;
  u8x8->cad_cb = u8x8_cad_001;
  u8x8->byte_cb = u8x8_byte_8bit_sw_spi;
  u8x8->gpio_and_delay_cb = u8x8_gpio_and_delay_arduino;

  /* setup display info */
  u8x8_display_Setup(u8x8);
}

void u8x8_Setup_SSD1306_128x64_NONAME(u8x8_t *u8x8)
{
  /* setup defaults and reset pins to U8X8_PIN_NONE */
  u8x8_SetupDefaults(u8x8);
  
  /* setup specific callbacks */
  u8x8->display_cb = u8x8_d_ssd1306_128x64_noname;
  u8x8->cad_cb = u8x8_cad_001;
  //u8x8->byte_cb = u8x8_byte_arduino_hw_spi;
  u8x8->byte_cb = u8x8_byte_8bit_sw_spi;
  u8x8->gpio_and_delay_cb = u8x8_gpio_and_delay_arduino;
  
  /* setup display info */
  u8x8_display_Setup(u8x8);
}


u8x8_t u8x8;
uint8_t tile[8] = { 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0 };

void setup(void)
{
  //u8x8_Setup_UC1701_DOGS102(&u8x8);  
  //u8x8_Setup_SSD1306_128x64_NONAME(&u8x8);
  //u8x8_Setup_SW_4Wire_SPI(&u8x8, u8x8_d_ssd1306_128x64_noname, 13, 11, 10, 9, 8);
  u8x8_Setup_HW_4Wire_SPI(&u8x8, u8x8_d_ssd1306_128x64_noname, 10, 9, 8);		// works!!
  //u8x8_Setup_SW_4Wire_SPI(&u8x8, u8x8_d_uc1701_dogs102, 13, 11, 10, 9, 8);
  //u8x8_Setup_HW_4Wire_SPI(&u8x8, u8x8_d_uc1701_dogs102, 10, 9, 8);		// broken???? does not work
}

void loop(void)
{
  u8x8_display_Init(&u8x8);  
  //u8x8_display_SetFlipMode(&u8x8, 1);


  for(;;)
  {
    u8x8_display_ClearScreen(&u8x8);  
    u8x8_display_SetPowerSave(&u8x8, 0);
    //u8x8_display_SetContrast(&u8x8, 10);
    
    delay(500);

    u8x8_SetFont(&u8x8, u8x8_font_chroma48medium8_r);
    u8x8_DrawString(&u8x8, 0, 0, "Hello World");
    u8x8_DrawString(&u8x8, 3, 1, "ABCdefg");
    
    u8x8_display_DrawTile(&u8x8, 1, 1, 1, tile);
    u8x8_display_DrawTile(&u8x8, 2, 2, 1, tile);
    u8x8_display_DrawTile(&u8x8, 3, 3, 1, tile);
    u8x8_display_DrawTile(&u8x8, 4, 4, 1, tile);
    u8x8_display_DrawTile(&u8x8, 5, 5, 1, tile);
    u8x8_display_DrawTile(&u8x8, 6, 6, 1, tile);

    delay(2000);
  }
}

