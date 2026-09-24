#include "AppInterface.h"
#include "YRShell.h"

#include "main.h"

class MyShell : public YRSmallShell {
    protected:
        virtual const char* shellClass( void) { return "MyShell"; }
        virtual const char* mainFileName( ) { return "app/AppInterface.cpp"; }
    public:
        MyShell() {}
        virtual ~MyShell() {}
    };

    MyShell yrShell;

void initializeApp() {
    BufferedSerial::STM32SerialError error = BSerial2.begin(115200);
    assert_param(error == BufferedSerial::STM32SerialError::none);

    BSerial2.init(yrShell.getInq(), yrShell.getOutq());
}

void updateApp() {
    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    // HAL_Delay(500);
    Sliceable::sliceAll();
}