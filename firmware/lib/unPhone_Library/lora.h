// lora.h
// the LoRa board and TTN LoRaWAN

#ifndef LORA_H
#define LORA_H

void lora_setup();                       // initialise lora/ttn
void lora_loop();                        // service pending lora transactions
void lora_send(const char *payload);     // send a ttn message

#endif
