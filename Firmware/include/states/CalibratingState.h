#pragma once
#include "IState.h"

using namespace mcompass;


class CalibratingState : public IState {
private:
    uint32_t m_lastUpdate = 0;
    int m_lastAzimuth = 0;

public:
    virtual void onEnter(Context& context) override;
    virtual void onExit(Context& context) override;
    virtual void handleEvent(Context& context, Event::Body* evt) override;
    virtual const char* getName() override { return "COMPASS"; }
};