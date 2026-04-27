#include "cmsis_os2.h"                          // CMSIS RTOS header file
 #include "Master.h"
 #include "Memoria.h"
 #include <stdbool.h>
 	#include <string.h> 
	#include <stdio.h>
/*----------------------------------------------------------------------------
*      MASTER: PRINCIPAL
 *---------------------------------------------------------------------------*/
 
 extern Modos_master_t modo;
 Modos_master_t modo = INI;
extern osThreadId_t TID_MASTER;
bool desactivacion = false;
extern bool web_finalizado; //ESTO VALIDA QUE SE HAYA ESCRITO EN LA WEB 

void automata_Master (void)
{
  switch(modo)
  {
    
    case INI:
      Init_I2C(); //Inicializamos la memoria 
                  //Inicalizamos la UART
                  //Inicializamos el RTC y el SNTP 
        osDelay(10000); // Esperamos a que ocurra todo
    printf("CONFIGURACION CORRECTA");
//        uint32_t flagEncendido = osThreadFlagsWait(ALARMA_SUBIDA_DE_TEMPERATURA,osFlagsWaitAny,osWaitForever); //ESPERAMOS POR UART A LA SUBIDA DE LA TEMPERATURA
//    if (flagEncendido && ALARMA_SUBIDA_DE_TEMPERATURA);
//    {
//      modo  = ESPERA; 
//    }
    break;
    case ESPERA:
      //CUANDO LLEGUE LA SUBIDA DE TEMEPARTURA ENTRA EN ESTE ESTADO EN EÑ QUE ESPERAMOS A QUE  RECIBA LAS TRAMAS Y DEMAS
    if(....) //CUANDO SE COMPLETE LA TRAMA 
    {
      modo =  RECIBIR;
      desactivacion = true;
    }      //ESPERAR A QUE LA UART MANDE EL MENSAJE DE COMPLETADO 
     if(....) 
     {
       modo = ESCRIBIR;
     }       
    case RECIBIR:
       
    break;
    
    case ESCRIBIR:
      if(!web_finalizado)
      {
        if(!desactivacion)
        {
          modo = ESCRIBIR;
        }else{
          modo = ESPERA;
        }
        
      }else{
        //AQUI ESPERAMOS A QUE LAS COSAS SE HAGAN EN LA WEB CON FALGS O LO QUE SEA
      }
    break;
    
    
  }
}