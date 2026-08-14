#include "utils/UndoRedoManager.h"

using namespace std;

// ============================================================
// UndoRedoManager.cpp
// ============================================================


// ---- Constructor ----

UndoRedoManager::UndoRedoManager()
    : currentIndex(-1)
{
}


// ---- Snapshot management ----

void UndoRedoManager::saveSnapshot(const string& serializedCircuit)
{
    // If we are not at the end of the history (i.e. the user
    // previously undid some steps), discard all redo snapshots
    // beyond the current position. This matches standard behavior:
    // making a new change after an undo wipes the redo history.
    if (currentIndex < (int)snapshots.size() - 1)
    {
        snapshots.erase(snapshots.begin() + currentIndex + 1,
                        snapshots.end());
    }

    // If we have hit the history cap, drop the oldest snapshot
    // to make room for the new one
    if ((int)snapshots.size() >= MAX_HISTORY)
    {
        snapshots.erase(snapshots.begin());
        currentIndex--;
    }

    snapshots.push_back(serializedCircuit);
    currentIndex = (int)snapshots.size() - 1;
}


// ---- Undo / Redo ----

string UndoRedoManager::undo()
{
    if (!canUndo())
    {
        return "";
    }

    currentIndex--;
    return snapshots[currentIndex];
}

string UndoRedoManager::redo()
{
    if (!canRedo())
    {
        return "";
    }

    currentIndex++;
    return snapshots[currentIndex];
}


// ---- State queries ----

bool UndoRedoManager::canUndo() const
{
    // Can undo as long as there is a snapshot before the current one.
    // currentIndex == 0 means we are at the initial empty state —
    // there is nothing to undo beyond that.
    return currentIndex > 0;
}

bool UndoRedoManager::canRedo() const
{
    // Can redo if there are snapshots after the current index
    return currentIndex < (int)snapshots.size() - 1;
}

int UndoRedoManager::undoCount() const
{
    return currentIndex;
}

int UndoRedoManager::redoCount() const
{
    return (int)snapshots.size() - 1 - currentIndex;
}


// ---- Reset ----

void UndoRedoManager::clear()
{
    snapshots.clear();
    currentIndex = -1;
}

void UndoRedoManager::initialize(const string& initialSerializedCircuit)
{
    clear();
    snapshots.push_back(initialSerializedCircuit);
    currentIndex = 0;
}