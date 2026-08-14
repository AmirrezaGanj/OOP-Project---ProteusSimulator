#pragma once

#include <string>
#include <vector>
#include "core/Circuit.h"

using namespace std;

// ============================================================
// ProjectManager — handles all file I/O for circuit projects.
//
// Responsibilities:
//   - Serialize a Circuit to a text string or file
//   - Deserialize a text string or file back into a Circuit
//   - Manage the list of recently opened projects (max 5)
//   - Track whether the current project has been saved before
//     (the `isSaved` flag from the project spec)
//   - Handle Save, Save As, and Open operations
//   - Provide serializeCircuit() for UndoRedoManager snapshots
//
// File format (plain text):
//   BEGIN_CIRCUIT
//   <component lines from each component's serialize()>
//   BEGIN_WIRES
//   <wire lines>
//   BEGIN_JUNCTIONS
//   <junction lines>
//   END_CIRCUIT
//
// All project files are saved in the saved_projects/ directory
// unless Save As specifies a different path.
// ============================================================

class ProjectManager
{
public:

    static const int MAX_RECENT_PROJECTS = 5;

    ProjectManager();

    // ---- Serialization (used by both file I/O and UndoRedoManager) ----

    // Converts the entire circuit into a saveable string.
    // Called by UndoRedoManager for snapshots and by save() for files.
    string serializeCircuit(Circuit& circuit);

    // Restores the circuit from a serialized string.
    // Clears the circuit first, then reconstructs all components,
    // wires, and junctions from the string.
    // Returns true on success, false on parse error.
    bool restoreCircuitFromSnapshot(Circuit& circuit,
                                    const string& snapshot);

    // ---- Save operations ----

    // Saves the circuit to the current file path.
    // If the project has never been saved (isSaved == false),
    // prompts the user to enter a name (via console input).
    // If a project with that name already exists, asks to overwrite.
    // Returns true on success.
    bool save(Circuit& circuit);

    // Saves the circuit to a user-specified path.
    // Always prompts for a path/name regardless of isSaved state.
    // Returns true on success.
    bool saveAs(Circuit& circuit, const string& filePath);

    // ---- Load operation ----

    // Loads a project from the given file path into the circuit.
    // Clears the current circuit first.
    // Updates currentFilePath and isSaved to true.
    // Adds the file to the recent projects list.
    // Returns true on success.
    bool openProject(Circuit& circuit, const string& filePath);

    // ---- Recent projects ----

    const vector<string>& getRecentProjects() const;

    // Loads the recent projects list from the recent_projects.txt file
    void loadRecentProjectsList();

    // ---- State queries ----

    bool          getIsSaved()          const;
    string        getCurrentFilePath()  const;
    string        getCurrentProjectName() const;

    // Called when a new blank project is started
    void newProject(Circuit& circuit);

    // ---- Image export (screenshot) ----
    // Declared here for organizational convenience;
    // actual pixel capture is in ImageExporter.
    // Returns the default export filename for the current project.
    string getDefaultExportFilename() const;

private:

    // True once the current project has been saved at least once
    bool   isSaved;

    // Full path to the current save file (empty if never saved)
    string currentFilePath;

    // Just the project name without path or extension
    string currentProjectName;

    // Up to 5 recently opened project file paths
    vector<string> recentProjects;

    // Path to the directory where projects are stored
    static const string SAVE_DIRECTORY;

    // Path to the file that stores the recent projects list
    static const string RECENT_PROJECTS_FILE;

    // ---- Private helpers ----

    // Writes a string to a file. Returns true on success.
    bool writeToFile(const string& filePath, const string& content);

    // Reads an entire file into a string. Returns "" on failure.
    string readFromFile(const string& filePath);

    // Adds a file path to the recent projects list and persists it.
    // Removes duplicates and keeps only the last MAX_RECENT_PROJECTS entries.
    void addToRecentProjects(const string& filePath);

    // Saves the recent projects list to recent_projects.txt
    void saveRecentProjectsList();

    // Checks if a file already exists at the given path
    bool fileExists(const string& filePath) const;

    // Parses one component line and adds the component to the circuit.
    // Returns true if the line was successfully parsed.
    bool parseComponentLine(Circuit& circuit, const string& line);

    // Parses one wire line and adds the wire to the circuit.
    bool parseWireLine(Circuit& circuit, const string& line);

    // Parses one junction line and adds the junction to the circuit.
    bool parseJunctionLine(Circuit& circuit, const string& line);

    // Splits a string by spaces into a vector of tokens
    vector<string> tokenize(const string& line);
};