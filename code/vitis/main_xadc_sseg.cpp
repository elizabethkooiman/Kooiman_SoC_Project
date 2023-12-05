/*****************************************************************//**
 * @file main_vanilla_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "xadc_core.h"
#include "sseg_core.h"

/**
 * blink once per second for 5 times.
 * provide a sanity check for timer (based on SYS_CLK_FREQ)
 * @param led_p pointer to led instance
 */
void timer_check(GpoCore *led_p) {
   int i;

   for (i = 0; i < 5; i++) {
      led_p->write(0xffff);
      sleep_ms(500);
      led_p->write(0x0000);
      sleep_ms(500);
      debug("timer check - (loop #)/now: ", i, now_ms());
   }
}

/**
 * check individual led
 * @param led_p pointer to led instance
 * @param n number of led
 */
void led_check(GpoCore *led_p, int n) {
   int i;

   for (i = 0; i < n; i++) {
      led_p->write(1, i);
      sleep_ms(200);
      led_p->write(0, i);
      sleep_ms(200);
   }
}

/**
 * leds flash according to switch positions.
 * @param led_p pointer to led instance
 * @param sw_p pointer to switch instance
 */
void sw_check(GpoCore *led_p, GpiCore *sw_p) {
   int i, s;

   s = sw_p->read();
   for (i = 0; i < 30; i++) {
      led_p->write(s);
      sleep_ms(50);
      led_p->write(0);
      sleep_ms(50);
   }
}

/**
 * uart transmits test line.
 * @note uart instance is declared as global variable in chu_io_basic.h
 */
void uart_check() {
   static int loop = 0;

   uart.disp("uart test #");
   uart.disp(loop);
   uart.disp("\n\r");
   loop++;
}


/**
 * read FPGA internal voltage temperature
 * @param adc_p pointer to xadc instance
 */

void adc_check(XadcCore *adc_p, GpoCore *led_p) {
   double reading;
   int n, i;
   uint16_t raw;

   for (i = 0; i < 5; i++) {
      // display 12-bit channel 0 reading in LED
      raw = adc_p->read_raw(0);
      raw = raw >> 4;
      led_p->write(raw);
      // display on-chip sensor and 4 channels in console
      uart.disp("FPGA vcc/temp: ");
      reading = adc_p->read_fpga_vcc();
      uart.disp(reading, 3);
      uart.disp(" / ");
      reading = adc_p->read_fpga_temp();
      uart.disp(reading, 3);
      uart.disp("\n\r");
      for (n = 0; n < 4; n++) {
         uart.disp("analog channel/voltage: ");
         uart.disp(n);
         uart.disp(" / ");
         reading = adc_p->read_adc_in(n);
         uart.disp(reading, 3);
         uart.disp("\n\r");
      } // end for
      sleep_ms(200);
   }
}




/**
 * Test pattern in 7-segment LEDs
 * @param sseg_p pointer to 7-seg LED instance
 */

void sseg_check(SsegCore *sseg_p) {
   int i, n;
   uint8_t dp;

   //turn off led
   for (i = 0; i < 8; i++) {
      sseg_p->write_1ptn(0xff, i);
   }
   //turn off all decimal points
   sseg_p->set_dp(0x00);

   // display 0x0 to 0xf in 4 epochs
   // upper 4  digits mirror the lower 4
   for (n = 0; n < 4; n++) {
      for (i = 0; i < 4; i++) {
         sseg_p->write_1ptn(sseg_p->h2s(i + n * 4), 3 - i);
         sseg_p->write_1ptn(sseg_p->h2s(i + n * 4), 7 - i);
         sleep_ms(300);
      } // for i
   }  // for n
      // shift a decimal point 4 times
   for (i = 0; i < 4; i++) {
      bit_set(dp, 3 - i);
      sseg_p->set_dp(1 << (3 - i));
      sleep_ms(300);
   }
   //turn off led
   for (i = 0; i < 8; i++) {
      sseg_p->write_1ptn(0xff, i);
   }
   //turn off all decimal points
   sseg_p->set_dp(0x00);
}

int int2sseg(int n){
	int val;
	switch(n){
	case 0:
		val = 0xc0;
		break;
	case 1:
		val = 0xf9;
		break;
	case 2:
		val = 0xa4;
		break;
	case 3:
		val = 0xb0;
		break;
	case 4:
		val = 0x99;
		break;
	case 5:
		val = 0x92;
		break;
	case 6:
		val = 0x82;
		break;
	case 7:
		val = 0xf8;
		break;
	case 8:
		val = 0x80;
		break;
	case 9:
		val = 0x98;
		break;
	}
	return val;
}

void xadc_sseg(XadcCore* adc, SsegCore* sseg,GpoCore* led,int sw){
	double reading;
	int tens, ones, tenth, hundredth,hundreds, thousands;
	if(sw == 1){
		reading = adc->read_fpga_vcc();
		tens = int(reading/10.0);
	   	ones = int(reading-(tens*10.0));
	   	tenth = int((reading-(tens*10.0)-ones)*10.0);
	   	hundredth=int((reading-(tens*10.0)-ones-(tenth * 0.1))*100.0);
		sseg->write_1ptn(0xc1, 0); //V
	   	sseg->write_1ptn(int2sseg(hundredth), 1);
	   	sseg->write_1ptn(int2sseg(tenth), 2);
	   	sseg->write_1ptn(int2sseg(ones), 3);
	   	sseg->write_1ptn(0xff, 4);
	    sseg->write_1ptn(0xff, 5);
	   	sseg->write_1ptn(0xff, 6);
	   	sseg->write_1ptn(0xff, 7);
	   	sseg->set_dp(8);
	   	led->write(0x0000);
	}
	else if(sw == 2){
		reading = adc->read_fpga_temp();
		tens = int(reading/10.0);
	   	ones = int(reading-(tens*10.0));
	   	tenth = int((reading-(tens*10.0)-ones)*10.0);
	   	hundredth=int((reading-(tens*10.0)-ones-(tenth * 0.1))*100.0);
		sseg->write_1ptn(0xc6 , 0);
	   	sseg->write_1ptn(int2sseg(hundredth), 1);
	   	sseg->write_1ptn(int2sseg(tenth), 2);
	   	sseg->write_1ptn(int2sseg(ones), 3);
	   	sseg->write_1ptn(int2sseg(tens), 4);
	    sseg->write_1ptn(0xff, 5);
	   	sseg->write_1ptn(0xff, 6);
	   	sseg->write_1ptn(0xff, 7);
	   	sseg->set_dp(8);
		led->write(0x0000);
	}
	else if(sw == 3){
		reading = adc->read_adc_in(0);
		thousands = int(reading/1000.0);
		hundreds = int((reading-thousands*1000)/100.0);
		tens = int((reading-thousands*1000 - hundreds * 100)/10.0);
	    ones = int(reading-(tens*10.0));
        tenth = int((reading-(tens*10.0)-ones)*10.0);
        hundredth=int((reading-(tens*10.0)-ones-(tenth * 0.1))*100.0);
		sseg->write_1ptn(0xc1, 0);
		sseg->write_1ptn(int2sseg(hundredth), 1);
		sseg->write_1ptn(int2sseg(tenth), 2);
		sseg->write_1ptn(int2sseg(ones), 3);
		sseg->write_1ptn(int2sseg(tens), 4);
		sseg->write_1ptn(int2sseg(hundreds), 5);
		sseg->write_1ptn(int2sseg(thousands), 6);
		sseg->write_1ptn(0xff, 7);
		sseg->set_dp(8);
		if(reading>0.9){
			led->write(0xffff);
		}
		else if(reading>0.8){
			led->write(0xffff);
		}
		else if(reading > 0.7){
			led->write(0x3fff);
		}
		else if(reading > 0.6){
			led->write(0x0fff);
		}
		else if(reading > 0.5){
			led->write(0x03ff);
		}
		else if(reading > 0.4){
			led->write(0x00ff);
		}
		else if(reading > 0.3){
			led->write(0x003f);
		}
		else if (reading > 0.2){
			led->write(0x000f);
		}
		else if (reading > 0.2){
			led->write(0x0003);
		}
		else{
			led->write(0x0000);
		}

	}
	else{
		sseg->write_1ptn(0xff, 0);
		sseg->write_1ptn(0xff, 1);
		sseg->write_1ptn(0xff, 2);
		sseg->write_1ptn(0xff, 3);
		sseg->write_1ptn(0xff, 4);
		sseg->write_1ptn(0xff, 5);
		sseg->write_1ptn(0xff, 6);
		sseg->write_1ptn(0xff, 7);
		sseg->set_dp(0);
	}
	sleep_ms(200);
}

// instantiate switch, led, xadc, sseg
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
XadcCore adc(get_slot_addr(BRIDGE_BASE, S5_XDAC));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));

int main() {

	int switch_in;
   while (1) {
	   if(sw.read(0)){
		   switch_in = 1;
	   }
	   else if(sw.read(1)){
		   switch_in = 2;
	   }
	   else if(sw.read(2)){
		   switch_in = 3;
	   }
	   else{
		   switch_in = 0;
	   }
		xadc_sseg(&adc, &sseg,&led,switch_in);
	 
   } //while
} //main

