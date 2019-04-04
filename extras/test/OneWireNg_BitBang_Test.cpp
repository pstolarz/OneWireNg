#include "common.h"

class OneWireNg_BitBang_Test: OneWireNg_BitBang
{
private:
    OneWireNg_BitBang_Test(): OneWireNg_BitBang(false) {}

    virtual int readGpioIn(GpioType gpio) {
        return 1;
    }

    virtual void writeGpioOut(GpioType gpio, int state) {}

    virtual void setGpioAsInput(GpioType gpio) {}

    virtual void setGpioAsOutput(GpioType gpio, int state) {}
};

int main(void)
{
    return 0;
}
