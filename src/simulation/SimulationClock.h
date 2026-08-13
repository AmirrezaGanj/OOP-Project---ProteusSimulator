#pragma once

#include <string>

using namespace std;

// ============================================================
// SimulationClock — manages simulation time and run state.
//
// The SimulationEngine calls tick() once per frame when the
// simulation is running. The clock tracks how many ticks have
// elapsed and converts that to a millisecond time value that
// components like ClockGenerator and DFlipFlop use.
//
// Three states:
//   RUNNING — tick() advances time each call
//   PAUSED  — tick() does nothing, time is frozen
//   STOPPED — tick() does nothing, time is reset to zero
//
// Step mode (for debugging sequential circuits):
//   When stepModeActive is true, the clock advances exactly
//   one tick per call to stepOnce(), then returns to PAUSED.
// ============================================================

class SimulationClock
{
public:

    enum class SimState
    {
        RUNNING,
        PAUSED,
        STOPPED
    };

    // msPerTick: how many simulated milliseconds each tick represents.
    // Default is 1ms per tick — matches the propagation delay units
    // used in LogicGate.
    SimulationClock(double msPerTick = 1.0);

    // ---- State control ----

    void run();     // Transition to RUNNING
    void pause();   // Transition to PAUSED  (preserves tick count)
    void stop();    // Transition to STOPPED (resets tick count to 0)

    // ---- Stepping ----

    // Advances the simulation by exactly one tick regardless of state.
    // Used for step-by-step debugging mode.
    // After calling this the state returns to PAUSED.
    void stepOnce();

    // ---- Per-frame update ----

    // Called once per frame by SimulationEngine.
    // If RUNNING: increments tickCount by 1 and returns true.
    // If PAUSED or STOPPED: does nothing and returns false.
    // The return value tells SimulationEngine whether to evaluate components.
    bool tick();

    // ---- Getters ----

    SimState getCurrentState()  const;
    long long getTickCount()    const;
    double    getElapsedMs()    const;   // tickCount * msPerTick
    double    getMsPerTick()    const;

    bool isRunning() const;
    bool isPaused()  const;
    bool isStopped() const;

    // ---- Settings ----

    void setMsPerTick(double ms);

private:

    SimState  currentState;
    long long tickCount;      // total ticks since last stop/reset
    double    msPerTick;      // simulated ms per tick
};