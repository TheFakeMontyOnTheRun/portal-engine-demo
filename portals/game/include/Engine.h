#ifndef ENGINE_H
#define ENGINE_H

typedef int32_t (*InitStateCallback)(int32_t tag, void* data);
typedef void (*InitialPaintCallback)(void);
typedef void (*RepaintCallback)(void);
typedef int32_t (*TickCallback)(int32_t tag, void* data);
typedef void (*UnloadStateCallback)();

extern InitStateCallback initStateCallback;
extern InitialPaintCallback initialPaintCallback;
extern RepaintCallback repaintCallback;
extern TickCallback tickCallback;
extern UnloadStateCallback unloadStateCallback;

int32_t MainMenu_initStateCallback(int32_t tag, void* data);
void MainMenu_initialPaintCallback(void);
void MainMenu_repaintCallback(void);
int32_t MainMenu_tickCallback(int32_t tag, void* data);
void MainMenu_unloadStateCallback();

int32_t Crawler_initStateCallback(int32_t tag, void* data);
void Crawler_initialPaintCallback(void);
void Crawler_repaintCallback(void);
void graphicsDrawRect(int16_t x, int16_t y, uint16_t dx, uint16_t dy, uint8_t pixel);
int32_t Crawler_tickCallback(int32_t tag, void* data);
void Crawler_unloadStateCallback();

extern bool isRunning;

enum ECommand {
  kCommandNone,
  kCommandUp,
  kCommandRight,
  kCommandDown,
  kCommandLeft,
  kCommandFire1,
  kCommandFire2,
  kCommandBack
};

enum EGameMenuState {
  kMainMenu,
  kPracticeCrawling,
  kQuit,
};

enum EPresentationState {
  kAppearing,
  kWaitingForInput,
  kConfirmInputBlink1,
  kConfirmInputBlink2,
  kConfirmInputBlink3,
  kConfirmInputBlink4,
  kConfirmInputBlink5,
  kConfirmInputBlink6,
  kFade  
};

extern long timeUntilNextState;
extern EPresentationState currentPresentationState;


extern NativeBitmap* currentBackgroundBitmap;
extern uint8_t cursorPosition;
extern int32_t nextNavigationSelection;
extern int32_t menuStateToReturn;
#endif
