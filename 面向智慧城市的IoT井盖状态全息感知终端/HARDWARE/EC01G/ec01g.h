#ifndef __EC01G_H
#define __EC01G_H

#include "sys.h" 

#define EC_RST PCout(13)

void usart_send_string(USART_TypeDef* USARTx, const char* str); 
void ec01g_init(u32 bound);
void ec01g_rst(void);
void ec01g_handle(void);
void nb_data_send(void);

#endif
