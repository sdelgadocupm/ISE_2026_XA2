#include "cmsis_os2.h"
#include "Driver_USART.h"
#include "UARTManager.h"

extern ARM_DRIVER_USART Driver_USART3;

static ARM_DRIVER_USART *USARTdrv = &Driver_USART3;
static osThreadId_t tid_ThCom = NULL;
static osThreadId_t tid_ThRecep = NULL;

// Callback ÚNICA para ambos threads
static void myUSART_callback(uint32_t event){
    if(event & ARM_USART_EVENT_SEND_COMPLETE){
        if (tid_ThCom != NULL) {
            osThreadFlagsSet(tid_ThCom, 0x01);
        }
        if (tid_ThRecep != NULL) {
            osThreadFlagsSet(tid_ThRecep, 0x04);
        }
    }
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
        if (tid_ThRecep != NULL) {
            osThreadFlagsSet(tid_ThRecep, 0x02);
        }
    }
}

// Inicializar UART una sola vez
void UART_Init(void) {
    USARTdrv->Initialize(myUSART_callback);
    USARTdrv->PowerControl(ARM_POWER_FULL);
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|
                      ARM_USART_DATA_BITS_8|
                      ARM_USART_PARITY_NONE|
                      ARM_USART_STOP_BITS_1|
                      ARM_USART_FLOW_CONTROL_NONE, 9600);
    
    // Habilitar RX Y TX
    USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
    USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
}

// Guardar IDs de threads
void UART_SetThreadIds(osThreadId_t tx_id, osThreadId_t rx_id) {
    tid_ThCom = tx_id;
    tid_ThRecep = rx_id;
}

// Función para enviar
int UART_Send(uint8_t *data, uint32_t len) {
    return USARTdrv->Send(data, len);
}

// Función para recibir
int UART_Receive(uint8_t *data, uint32_t len) {
    return USARTdrv->Receive(data, len);
}

ARM_DRIVER_USART* UART_GetDriver(void) {
    return USARTdrv;
}