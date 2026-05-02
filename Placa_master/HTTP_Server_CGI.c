/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.0.0
 *----------------------------------------------------------------------------*/
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "Logger.h"
#include "ThCom.h"
#include "Recepcion.h"
#include "Memoria.h"
#include "Master.h"
//#include "Board_LED.h"                  // ::Board Support:LED

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif

// http_server.c
extern uint16_t AD_in (uint32_t ch);
//extern uint8_t  get_button (void);




// Local variables.
static uint8_t P2;
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];
extern int adc_value;
uint32_t volts;
uint32_t mv_total ;
uint32_t mvolts ;


//Varables necesarias para las medidas


extern osMessageQueueId_t mid_ComQueue;

extern uint16_t temp;
extern uint16_t co2;
extern uint16_t tvoc;
extern uint16_t consumo;
extern uint16_t estado;
extern uint16_t modo;


//extern char fecha_actual[11];           // "YYYY-MM-DD"
//extern char hora_actual[9];             // "HH:MM:SS"
extern char ultima_actualizacion[25];   // "YYYY-MM-DD HH:MM:SS" no se de donde sale pero esta en la web

static char last_pg[8] = {0};

// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

// Process query string received by GET request.
void netCGI_ProcessQuery (const char *qstr) {
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter

    switch (var[0]) {
      case 'i': // Local IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_Address;       }
        else               { opt = netIF_OptionIP6_StaticAddress; }
        break;

      case 'm': // Local network mask
        if (var[1] == '4') { opt = netIF_OptionIP4_SubnetMask; }
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP6_DefaultGateway; }
        else               { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        else               { opt = netIF_OptionIP6_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        else               { opt = netIF_OptionIP6_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      case '6': typ = NET_ADDR_IP6; break;

      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) {
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];

  if (code != 0) {
    // Ignore all other codes
    return;
  }

  P2 = 0;
  
  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    if (var[0] != 0) {
      // First character is non-null, string exists
      if (strcmp (var, "led0=on") == 0) {
        P2 |= 0x01;
      }
      else if (strcmp (var, "led1=on") == 0) {
        P2 |= 0x02;
      }
      else if (strcmp (var, "led2=on") == 0) {
        P2 |= 0x04;
      }
      else if (strcmp (var, "led3=on") == 0) {
        P2 |= 0x08;
      }
      else if (strcmp (var, "ctrl=Browser") == 0) {
        
      }
      else if ((strncmp (var, "pw0=", 4) == 0) ||
               (strncmp (var, "pw2=", 4) == 0)) {
        // Change password, retyped password
        if (netHTTPs_LoginActive()) {
          if (passw[0] == 1) {
            strcpy (passw, var+4);
          }
          else if (strcmp (passw, var+4) == 0) {
            // Both strings are equal, change the password
            netHTTPs_SetPassword (passw);
          }
        }
      }
      else if (strncmp (var, "lcd1=", 5) == 0) {
        // LCD Module line 1 text
       
      }
      else if (strncmp (var, "lcd2=", 5) == 0) {
        
      }else if(strncmp(var, "pg=3", 3) == 0)
      {
        strncpy(last_pg, var + 3, sizeof(last_pg)-1);
          last_pg[sizeof(last_pg)-1] = '\0';
      }else if(strcmp(var, "cmd=disable_alarm") == 0)
      {
        //Enviamos los comando por UART
        MSGQUEUE_OBJ_COM_t out;
          snprintf(out.Mensaje,sizeof(out.Mensaje), " 4 1\n");
        out.TamMens = (uint8_t)strlen(out.Mensaje);
        osMessageQueuePut(mid_ComQueue,&out, 0U,0U);
        
      }else if(strncmp(var, "th_temp=",8) == 0)
      {
        th_temp = (float)atof(var + 8);
         guardar_umbrales(th_temp, th_co2, th_tvoc);
        Master_confirm();

      }else if(strncmp(var, "th_co2=",7) == 0)
      {
        th_co2 = (float)atof(var + 7);
        guardar_umbrales(th_temp, th_co2, th_tvoc);
        Master_confirm();

      }else if(strncmp(var, "th_tvoc=", 8) == 0)
      {
        th_tvoc = atoi(var + 8);
        guardar_umbrales(th_temp, th_co2, th_tvoc);
        Master_confirm();
    }
  }

  } while (data);
  
}

// Generate dynamic web data from a script line.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  int32_t socket;
  netTCP_State state;
  NET_ADDR r_client;
  const char *lang;
  uint32_t len = 0U;
  uint8_t id;
  static uint32_t adv;
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;

  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
    case 'a' :
      // Network parameters from 'network.cgi'
      switch (env[3]) {
        case '4': typ = NET_ADDR_IP4; break;
        case '6': typ = NET_ADDR_IP6; break;

        default: return (0);
      }
      
      switch (env[2]) {
        case 'l':
          // Link-local address
          if (env[3] == '4') { return (0);                             }
          else               { opt = netIF_OptionIP6_LinkLocalAddress; }
          break;

        case 'i':
          // Write local IP address (IPv4 or IPv6)
          if (env[3] == '4') { opt = netIF_OptionIP4_Address;       }
          else               { opt = netIF_OptionIP6_StaticAddress; }
          break;

        case 'm':
          // Write local network mask
          if (env[3] == '4') { opt = netIF_OptionIP4_SubnetMask; }
          else               { return (0);                       }
          break;

        case 'g':
          // Write default gateway IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_DefaultGateway; }
          else               { opt = netIF_OptionIP6_DefaultGateway; }
          break;

        case 'p':
          // Write primary DNS server IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
          else               { opt = netIF_OptionIP6_PrimaryDNS; }
          break;

        case 's':
          // Write secondary DNS server IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
          else               { opt = netIF_OptionIP6_SecondaryDNS; }
          break;
      }

      netIF_GetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
      netIP_ntoa (typ, ip_addr, ip_string, sizeof(ip_string));
      len = (uint32_t)sprintf (buf, &env[5], ip_string);
      break;

    case 'b':
     
      break;

    case 'c':
      // TCP status from 'tcp.cgi'
      while ((uint32_t)(len + 150) < buflen) {
        socket = ++MYBUF(pcgi)->idx;
        state  = netTCP_GetState (socket);

        if (state == netTCP_StateINVALID) {
          /* Invalid socket, we are done */
          return ((uint32_t)len);
        }

        // 'sprintf' format string is defined here
        len += (uint32_t)sprintf (buf+len,   "<tr align=\"center\">");
        if (state <= netTCP_StateCLOSED) {
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>-</td><td>-</td>"
                                             "<td>-</td><td>-</td></tr>\r\n",
                                             socket,
                                             netTCP_StateCLOSED);
        }
        else if (state == netTCP_StateLISTEN) {
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>%d</td><td>-</td>"
                                             "<td>-</td><td>-</td></tr>\r\n",
                                             socket,
                                             netTCP_StateLISTEN,
                                             netTCP_GetLocalPort(socket));
        }
        else {
          netTCP_GetPeer (socket, &r_client, sizeof(r_client));

          netIP_ntoa (r_client.addr_type, r_client.addr, ip_string, sizeof (ip_string));
          
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>%d</td>"
                                             "<td>%d</td><td>%s</td><td>%d</td></tr>\r\n",
                                             socket, netTCP_StateLISTEN, netTCP_GetLocalPort(socket),
                                             netTCP_GetTimer(socket), ip_string, r_client.port);
        }
      }
      /* More sockets to go, set a repeat flag */
      len |= (1u << 31);
      break;

    case 'd':
      // System password from 'system.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], netHTTPs_LoginActive() ? "Enabled" : "Disabled");
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], netHTTPs_GetPassword());
          break;
      }
      break;

    case 'e':
      // Browser Language from 'language.cgi'
      lang = netHTTPs_GetLanguage();
      if      (strncmp (lang, "en", 2) == 0) {
        lang = "English";
      }
      else if (strncmp (lang, "de", 2) == 0) {
        lang = "German";
      }
      else if (strncmp (lang, "fr", 2) == 0) {
        lang = "French";
      }
      else if (strncmp (lang, "sl", 2) == 0) {
        lang = "Slovene";
      }
      else {
        lang = "Unknown";
      }
      len = (uint32_t)sprintf (buf, &env[2], lang, netHTTPs_GetLanguage());
      break;

    case 'f':

      break;
						
//    case 'g':
//      // AD Input from 'ad.cgi'
//      switch (env[2]) {
//        case '1':
////          adv = AD_in(0);
//          len = (uint32_t)sprintf (buf, &env[4], adv);
//          break;
//				case '2':
//          len = (uint32_t)sprintf (buf, &env[4], (double)((float)adv*3.3f)/4096);
//          break;
//        case '3':
//          adv = (adv * 100) / 4096;
//          len = (uint32_t)sprintf (buf, &env[4], adv);
//          break;
//      }
//      break;
      
        case 's':
          switch (env[2])
          {
            case '1':{
                const char *cls = "guardian";
                const char *txt = "GUARDIAN";
                  if(modo == 1)
                  {
                    cls = "pre";
                    txt = "PREACTIVACION";
                  }else if(modo == 2)
                  {
                    cls = "alarm";
                    txt = "ACTIVACION";
                  }
                  len = (uint32_t)sprintf (buf, &env[4], cls, txt);
              break;
            }
            case 2:
            {
              //Vemos el estado de la energia 
              const char *pwr; 
              if(modo == 0)
              {
                pwr = "Bajo consumo (60s)";
              }else if(modo == 1)
              {
                pwr = "Vigilancia (periodo corto)";
              }else 
              {
                pwr = "Alarma activa";
              }
              len = (uint32_t)sprintf(buf, &env[4], pwr);
              break;
            }
            case 3: //Consumo de la Energia (?mA¿)
            {
              float cons = (float)consumo;
              len = (uint32_t)sprintf(buf, &env[4], consumo);
              break;
            }
            case 4: //Ultima_actualizacion
//              len = (uint32_t)sprintf(buf, &env[4], ultima_actualizacion);
              break;
              case '5':
//              len = (uint32_t)sprintf(buf, &env[4], fecha_actual);
              break;
            case '6':
//                len = (uint32_t)sprintf(buf, &env[4], hora_actual);
              break;
            case '7': {
               float t = ((float)temp) / 10.0f ;
                len = (uint32_t)sprintf(buf, &env[4], t);
          break;
            }
            case '8':
              len = (uint32_t)sprintf(buf, &env[4], (int)co2); 
              break;
              case '9':
              len = (uint32_t)sprintf(buf, &env[4], (int)tvoc); 
              break;
            }
          
            break;
              case 'k':
                switch(env[2])
                {
                  case '1' : 
                    len = (uint32_t)sprintf(buf, &env[4], (int)th_temp); 
                    break;
                   case '2' : 
                    len = (uint32_t)sprintf(buf, &env[4], (int)th_co2); 
                    break;
                    case '3' : 
                    len = (uint32_t)sprintf(buf, &env[4], (int)th_tvoc); 
                    break;
                }
                break;
                    case 'h':
                    {
                      AlarmEvent_t medidas;
                      uint16_t write_idx = Logger_GetWriteIndex(); //para sacar el indice 
                      uint16_t mensajes;
                      
                      int32_t  idx = (int32_t)write_idx -1;
                      
                       if(idx < 0)
                       {
                         idx = (int32_t)MAX_ALARM_EVENTS - 1; //Que vaya rellenado 
                       }                         
                      while(mensajes < 20)
                      {
                        if(Logger_ReadEvent((uint16_t)idx,&medidas))
                        {
                          char actualizacion[16];
                          snprintf(actualizacion, sizeof(actualizacion),"%02u:%02u:%02u", medidas.hora_activacion,medidas.minuto_activacion,medidas.segundo_activacion);
                        
                          char desactivacion[16];
                              if (medidas.tipo_desactivacion == 0xFF) {
                              strcpy(desactivacion, "---");
                                } else {
                                      snprintf(desactivacion, sizeof(desactivacion), "%02u:%02u:%02u", medidas.hora_desac, medidas.minuto_desac, medidas.segundo_desac);
                                }
                          char metodo[32];
                                if(medidas.tipo_desactivacion == 0)
                                {
                                  snprintf(metodo,sizeof(metodo), "RIFD %02X%02X%02X%02X", medidas.rfid[0],  medidas.rfid[1], medidas.rfid[2], medidas.rfid[3]);
                                  
                                }else if(medidas.tipo_desactivacion == 1)
                                {
                                  snprintf(metodo,sizeof(metodo),"REMOTO");
                                  
                                }else
                                {
                                   snprintf(metodo,sizeof(metodo),"PENDIENTE");
                                }
                                float t = ((float)medidas.temperatura) / 10.0f;

                              if ((len + 240) >= buflen) {
                                    len |= (1u << 31);
                                break;
                                }
                              
                                len +=  (uint32_t)sprintf(buf + len,
                                          "<tr>"
                                          "<td>%u</td>"
                                          "<td>%s</td>"
                                          "<td>%.1f</td>"
                                          "<td>%u</td>"
                                          "<td>%u</td>"
                                          "<td>%s</td>"
                                          "<td>%s</td>"
                                            "</tr>\r\n",
                                        (unsigned)(mensajes + 1),
                                        actualizacion,
                                          t,
                                        (unsigned)medidas.eco2,
                                          (unsigned)medidas.tvoc,
                                        desactivacion,
                                      metodo );
                                          mensajes++;
                                        }
                                          idx--;
                          if(idx < 0)
                          {
                            idx = (int32_t)MAX_ALARM_EVENTS - 1;
                          }
                          if ((uint16_t)idx == write_idx)
                          {
                            break;
                          }
                        }
                      break;
                    }
                  
//    case 'x':
//      // AD Input from 'ad.cgx'
//      adv = AD_in (0);
//      len = (uint32_t)sprintf (buf, &env[1], adv);
//      break;

  }
  return (len);
}

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
