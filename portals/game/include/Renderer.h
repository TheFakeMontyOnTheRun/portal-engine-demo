#ifndef RENDERER_H
#define RENDERER_H

void eventsInit(void);
void eventsHandle(void);
void videoDestroy(void);
void videoFlip(void);

extern uint8_t lastCommand;
extern uint8_t buffer[320*200];

#endif
