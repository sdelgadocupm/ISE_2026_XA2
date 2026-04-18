#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "RFID.h"    
#include "Principal.h"

extern osThreadId_t tid_Principal;
osThreadId_t tid_ThRFID;                        // thread id
extern volatile state_t modo;
void ThRFID (void *argument);                   // thread function
 
int Init_ThRFID (void) {
	
  tid_ThRFID = osThreadNew(ThRFID, NULL, NULL);
  if (tid_ThRFID == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThRFID (void *argument) {
  RC522_BSP_GPIO_Init();
  RC522_SPI1_Init();
	RC522_Init();
	
  while (1) {
    if(modo == ALARMA){
		 if (RFID_TryDisarm()) {
					osThreadFlagsSet(tid_Principal, 0x01);
					osDelay(500); // anti-rebote para no disparar varias veces
				} else {
					osDelay(50);
			}
		}else {
    osDelay(200);
		}
  }
}
