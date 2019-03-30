#ifndef DEF_HEADER_FPSMANAGER
#define DEF_HEADER_FPSMANAGER

#include <string>
#include <time.h>
#include <unistd.h>

class FPSManager {
public:
    static const uint32_t interval = 1000000;

    FPSManager(int _control);

    ~FPSManager();

    std::string toString();

    double fps();

    bool canMaj();

    void maj();

    void waitUntilNextFrame();

    void setControl(uint32_t fps);

private:
    long control; // microseconds for each frame (goal)
    long last;    // last maj
    long frame;
    struct timespec timer;
	uint32_t counter;

    long getTime();
};


#endif //DEF_HEADER_FPSMANAGER
