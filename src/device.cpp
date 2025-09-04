#include "devices/olivier.h"
#include "devices/yann.h"


// Singleton implementation
IDevice* IDevice::instance() {

#ifdef DEVICE_OLIVIER
    static Device singleton(2, 1, "LED_OLIVIER");
#elif DEVICE_YANN
    static Device singleton(2, 2, "LED_YANN");
#elif DEVICE_BERTRAND
    static Device singleton(2, 3, "LED_BERTRAND");
#elif DEVICE_MAXIME
    static Device singleton(2, 4, "LED_MAXIME");
#elif DEVICE_JEROME
    static Device singleton(2, 5, "LED_JEROME");
#elif DEVICE_DAVID
    static Device singleton(2, 6, "LED_DAVID");
#endif    
    return &singleton;
}