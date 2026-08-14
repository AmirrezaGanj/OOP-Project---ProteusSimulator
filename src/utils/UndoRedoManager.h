#pragma once

#include <string>
#include <vector>

using namespace std;

// ============================================================
// UndoRedoManager — manages undo and redo history.
//
// Works with in-memory snapshots: each snapshot is a full
// serialized string of the circuit state (same format as
// the save file). When the user undoes or redoes, the caller
// takes the returned snapshot string and passes it to
// ProjectManager to restore the circuit from it.
//
// How it works:
//   - snapshots is a vector of serialized circuit strings
//   - currentIndex points to the currently active snapshot
//   - saveSnapshot(): pushes a new snapshot after currentIndex
//     and discards any redo history beyond that point
//   - undo(): steps currentIndex back by 1, returns snapshot
//   - redo(): steps currentIndex forward by 1, returns snapshot
//
// The history is capped at MAX_HISTORY snapshots to avoid
// using too much memory on large circuits.
//
// Usage flow:
//   1. User places a component -> Canvas calls saveSnapshot()
//   2. User presses Ctrl+Z     -> Canvas calls undo(), gets
//      snapshot string, passes to ProjectManager to restore
//   3. User presses Ctrl+Y     -> Canvas calls redo(), same
// ============================================================

class UndoRedoManager
{
public:

    static const int MAX_HISTORY = 50;

    UndoRedoManager();

    // ---- Snapshot management ----

    // Saves a new snapshot of the circuit state.
    // Call this every time the circuit changes:
    // component placed, moved, deleted, wire added, etc.
    // Passing the full serialized circuit string as the snapshot.
    void saveSnapshot(const string& serializedCircuit);

    // ---- Undo / Redo ----

    // Steps back one snapshot. Returns the serialized circuit
    // string for the previous state.
    // Only call when canUndo() is true.
    string undo();

    // Steps forward one snapshot. Returns the serialized circuit
    // string for the next state.
    // Only call when canRedo() is true.
    string redo();

    // ---- State queries ----

    bool canUndo() const;
    bool canRedo() const;

    // Returns how many undo steps are available
    int  undoCount() const;

    // Returns how many redo steps are available
    int  redoCount() const;

    // ---- Reset ----

    // Clears all history. Called when a new project is created
    // or a project is loaded from file.
    void clear();

    // Seeds the manager with an initial empty-circuit snapshot
    // so that undoing all the way returns to a blank canvas.
    void initialize(const string& initialSerializedCircuit);

private:

    // All saved circuit snapshots
    vector<string> snapshots;

    // Index of the currently active snapshot.
    // Undo moves it left, Redo moves it right.
    int currentIndex;
};