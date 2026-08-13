#include "simulation/SimulationClock.h"

using namespace std;

// ============================================================
// SimulationClock.cpp
// ============================================================


// ---- Constructor ----

SimulationClock::SimulationClock(double msPerTick)
    : currentState(SimState::STOPPED),
      tickCount(0),
      msPerTick(msPerTick)
{
}


// ---- State control ----

void SimulationClock::run()
{
    // Can transition from either PAUSED or STOPPED to RUNNING.
    // If coming from STOPPED, tick count is already 0 from stop().
    currentState = SimState::RUNNING;
}

void SimulationClock::pause()
{
    // Only meaningful when RUNNING — freezes time in place.
    // Tick count is preserved so simulation can resume exactly.
    if (currentState == SimState::RUNNING)
    {
        currentState = SimState::PAUSED;
    }
}

void SimulationClock::stop()
{
    // Fully resets the simulation — time goes back to zero.
    // All pending events and delays in components should also
    // be cleared by SimulationEngine when it sees this state.
    currentState = SimState::STOPPED;
    tickCount    = 0;
}


// ---- Stepping ----

void SimulationClock::stepOnce()
{
    // Advance exactly one tick regardless of current state,
    // then immediately go to PAUSED so the user can inspect
    // the circuit state before the next step.
    tickCount++;
    currentState = SimState::PAUSED;
}


// ---- Per-frame update ----

bool SimulationClock::tick()
{
    if (currentState == SimState::RUNNING)
    {
        tickCount++;
        return true;    // SimulationEngine should evaluate all components
    }

    return false;       // Simulation is frozen — nothing to evaluate
}


// ---- Getters ----

SimulationClock::SimState SimulationClock::getCurrentState() const
{
    return currentState;
}

long long SimulationClock::getTickCount() const
{
    return tickCount;
}

double SimulationClock::getElapsedMs() const
{
    return (double)tickCount * msPerTick;
}

double SimulationClock::getMsPerTick() const
{
    return msPerTick;
}

bool SimulationClock::isRunning() const
{
    return currentState == SimState::RUNNING;
}

bool SimulationClock::isPaused() const
{
    return currentState == SimState::PAUSED;
}

bool SimulationClock::isStopped() const
{
    return currentState == SimState::STOPPED;
}


// ---- Settings ----

void SimulationClock::setMsPerTick(double ms)
{
    if (ms > 0.0)
    {
        msPerTick = ms;
    }
}