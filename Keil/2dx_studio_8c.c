#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "vl53l1x_api.h"
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"

#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           													// activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          												// activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};																		// ready?

    GPIO_PORTB_AFSEL_R |= 0x0C;           																	// 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;             																	// 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C;             																	// 5) enable digital I/O on PB2,3

  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
    I2C0_MCR_R = I2C_MCR_MFE;                      													// 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                       	// 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
        
}


void PortJ_Init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;                    // activate clock for Port J
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R8) == 0){};    // allow time for clock to stabilize
  GPIO_PORTJ_DIR_R &= ~0x02;                                            // make PJ1 in 
  GPIO_PORTJ_DEN_R |= 0x02;                                             // enable digital I/O on PJ1
    
    GPIO_PORTJ_PCTL_R &= ~0x000000F0;                                     //  configure PJ1 as GPIO 
    GPIO_PORTJ_AMSEL_R &= ~0x02;                                            //  disable analog functionality on PJ1        
    GPIO_PORTJ_PUR_R |= 0x02;                                                    //    enable weak pull up resistor
}

void PortN1_Init(void){				//FROM MS1
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12; //activate the clock for Port N
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};//allow time for clock to stabilize
	GPIO_PORTN_DIR_R=0b00000010; //Make PN1 output, to turn on LED's
	GPIO_PORTN_DEN_R=0b00000010; //Enable PN1
	
	return;
}
void PortM_Init(void){				//From MS1
	//Use PortM pins for output
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;				// activate clock for Port N
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};	// allow time for clock to stabilize
	GPIO_PORTM_DIR_R |= 0xFF;        								// make PN0 out (PN0 built-in LED1)
  GPIO_PORTM_AFSEL_R &= ~0xFF;     								// disable alt funct on PN0
  GPIO_PORTM_DEN_R |= 0xFF;        								// enable digital I/O on PN0
																									// configure PN1 as GPIO
  GPIO_PORTM_AMSEL_R &= ~0xFF;     								// disable analog functionality on PN0		
	return;
}

void spin(int direction){
    for(int i=0; i<8; i++){
			if(direction == 1) { //Counter-clockwise 
				GPIO_PORTM_DATA_R = 0b00001100;
        SysTick_Wait10ms(1);
        GPIO_PORTM_DATA_R = 0b00000110;
        SysTick_Wait10ms(1);
        GPIO_PORTM_DATA_R = 0b00000011;
        SysTick_Wait10ms(1);
        GPIO_PORTM_DATA_R = 0b00001001;
        SysTick_Wait10ms(1);
      }
      else if(direction == 0) { //Clockwise
        GPIO_PORTM_DATA_R = 0b00001001;
        SysTick_Wait10ms(1);
        GPIO_PORTM_DATA_R = 0b00000011;
        SysTick_Wait10ms(1);
				GPIO_PORTM_DATA_R = 0b00000110;
        SysTick_Wait10ms(1);
				GPIO_PORTM_DATA_R = 0b00001100;
        SysTick_Wait10ms(1);
      }
      else {
        GPIO_PORTH_DATA_R = 0b00000000;
        SysTick_Wait10ms(1);
			}        
    }
        GPIO_PORTM_DATA_R = 0b00000000;
}
uint16_t	dev = 0x29;			//address of the ToF sensor as an I2C slave peripheral
int status=0;

int main(void) {
  uint8_t sensorState=0;
  uint16_t wordData;
  uint16_t Distance; 
  uint8_t dataReady;

	//initialize
	PortJ_Init();//onboard push button
	PortM_Init();//stepper
	PortN1_Init();//required light
	PLL_Init();	//PLL
	SysTick_Init();//wait timer
	onboardLEDs_Init();//onboard leds
	I2C_Init();//I^2C comms
	UART_Init();///UART Comms


	/* Those basic I2C read functions can be used to check your own I2C functions */
	status = VL53L1X_GetSensorId(dev, &wordData);//check sensor id

	// Booting ToF chip
	while(sensorState==0)													//while still booting up
	{
		status = VL53L1X_BootState(dev, &sensorState);//check status
		SysTick_Wait10ms(10);//wait timer
	}
	
	FlashAllLEDs();//flash leds when done
	
	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
	
	/* This function must to be called to initialize the sensor with the default setting  */
	status = VL53L1X_SensorInit(dev);//sensor status
	Status_Check("SensorInit", status);//check status
	
		while(1)//always running
		{
			status = VL53L1X_StartRanging(dev);//start using ToF
			Restart://future restart location
																																					//This function has to be called to enable the ranging
			if((GPIO_PORTJ_DATA_R&0b00000010) == 0)//if active low detected
			{
					for(int i = 0; i < 64; i++) 																		//Get the Distance Measures 64 times
					{
						while (dataReady == 0)																				//wait until the ToF sensor's data is ready
						{
							status = VL53L1X_CheckForDataReady(dev, &dataReady);				//check if data ready				
							FlashLED3(1);																								//Flash third led
							VL53L1_WaitMs(dev, 5);																			//small wait timer
						}dataReady = 0;																								//reset for next iteration
	  
																																					//read the data values from ToF sensor
						status = VL53L1X_GetDistance(dev, &Distance);									//The Measured Distance value
						double distance = Distance;																		//value as a double
						distance /=1000;																							//value in meters
						FlashLED1(1);																									//FlashAllLEDs required led

						status = VL53L1X_ClearInterrupt(dev); 												//clear interrupt has to be called to enable next interrupt

																																					//print the resulted readings to UART
						sprintf(printf_buffer,"%f\r\n", distance);										//print to PC
						UART_printf(printf_buffer);																		//extra buffer

						spin(1);																											//spin stepper (1/64)th full rotation
						SysTick_Wait10ms(1);																					//wait timer
						
						if((GPIO_PORTJ_DATA_R&0b00000010) == 0) //Check if push button is pressed (Active low configuration)
						{
							for(int j = 0; j < i; j++)//return to original position (before first iteration
							{
								spin(0); //Increment back the amount it has spun
              }
							goto Restart;																							
            }
						
					}
			for(int i = 0; i<64;i++)																						//64 time
			{
				spin(0);																													//spin in reverse direction
			}
			
			VL53L1X_StopRanging(dev);																						//stop using ToF
			
			}
		}
}

