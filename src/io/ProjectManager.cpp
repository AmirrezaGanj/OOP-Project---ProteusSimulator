#include "io/ProjectManager.h"

// Component headers — needed for the deserialization factory
#include "components/sources/GND.h"
#include "components/sources/DCVoltageSource.h"
#include "components/sources/Battery.h"
#include "components/sources/ClockGenerator.h"
#include "components/passive/Resistor.h"
#include "components/passive/Capacitor.h"
#include "components/passive/Inductor.h"
#include "components/interactive/Switch.h"
#include "components/interactive/PushButton.h"
#include "components/interactive/LED.h"
#include "components/interactive/SevenSegment.h"
#include "components/gates/ANDGate.h"
#include "components/gates/ORGate.h"
#include "components/gates/NOTGate.h"
#include "components/gates/NANDGate.h"
#include "components/gates/XORGate.h"
#include "components/gates/DFlipFlop.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

using namespace std;

// ============================================================
// ProjectManager.cpp
// ============================================================

const string ProjectManager::SAVE_DIRECTORY      = "saved_projects/";
const string ProjectManager::RECENT_PROJECTS_FILE = "saved_projects/recent_projects.txt";


// ---- Constructor ----

ProjectManager::ProjectManager()
    : isSaved(false),
      currentFilePath(""),
      currentProjectName("Untitled")
{
    loadRecentProjectsList();
}


// ---- Serialization ----

string ProjectManager::serializeCircuit(Circuit& circuit)
{
    string output = "";

    output += "BEGIN_CIRCUIT\n";

    // Serialize all components
    vector<Component*>& components = circuit.getComponents();
    for (int i = 0; i < (int)components.size(); i++)
    {
        output += components[i]->serialize() + "\n";
    }

    output += "BEGIN_WIRES\n";

    // Serialize all wires
    vector<Wire*>& wires = circuit.getWires();
    for (int i = 0; i < (int)wires.size(); i++)
    {
        output += wires[i]->serialize() + "\n";
    }

    output += "BEGIN_JUNCTIONS\n";

    // Serialize all junctions
    vector<Junction*>& junctions = circuit.getJunctions();
    for (int i = 0; i < (int)junctions.size(); i++)
    {
        output += junctions[i]->serialize() + "\n";
    }

    output += "END_CIRCUIT\n";

    return output;
}


bool ProjectManager::restoreCircuitFromSnapshot(Circuit& circuit,
                                                 const string& snapshot)
{
    circuit.clear();

    istringstream stream(snapshot);
    string line;

    // Tracks which section we are currently parsing
    enum class Section { COMPONENTS, WIRES, JUNCTIONS, NONE };
    Section currentSection = Section::NONE;

    while (getline(stream, line))
    {
        if (line.empty())                 continue;
        if (line == "BEGIN_CIRCUIT")      { currentSection = Section::COMPONENTS; continue; }
        if (line == "BEGIN_WIRES")        { currentSection = Section::WIRES;      continue; }
        if (line == "BEGIN_JUNCTIONS")    { currentSection = Section::JUNCTIONS;  continue; }
        if (line == "END_CIRCUIT")        break;

        if (currentSection == Section::COMPONENTS)
        {
            parseComponentLine(circuit, line);
        }
        else if (currentSection == Section::WIRES)
        {
            parseWireLine(circuit, line);
        }
        else if (currentSection == Section::JUNCTIONS)
        {
            parseJunctionLine(circuit, line);
        }
    }

    return true;
}


// ---- Save operations ----

bool ProjectManager::save(Circuit& circuit)
{
    if (isSaved)
    {
        // Project was previously saved — overwrite without asking
        string content = serializeCircuit(circuit);
        return writeToFile(currentFilePath, content);
    }
    else
    {
        // First time saving — ask user for a project name
        cout << "\nEnter project name to save: ";
        string name;
        getline(cin, name);

        if (name.empty())
        {
            cout << "Save cancelled." << endl;
            return false;
        }

        string filePath = SAVE_DIRECTORY + name + ".txt";

        if (fileExists(filePath))
        {
            // A project with this name already exists
            cout << "A project named '" << name
                 << "' already exists. Overwrite? (y/n): ";
            string response;
            getline(cin, response);

            if (response != "y" && response != "Y")
            {
                cout << "Save cancelled." << endl;
                return false;
            }
        }

        string content = serializeCircuit(circuit);

        if (writeToFile(filePath, content))
        {
            currentFilePath    = filePath;
            currentProjectName = name;
            isSaved            = true;
            addToRecentProjects(filePath);
            cout << "Project saved as '" << name << "'." << endl;
            return true;
        }

        return false;
    }
}


bool ProjectManager::saveAs(Circuit& circuit, const string& filePath)
{
    string content = serializeCircuit(circuit);

    if (writeToFile(filePath, content))
    {
        currentFilePath = filePath;

        // Extract project name from the file path
        size_t lastSlash = filePath.find_last_of("/\\");
        string filename  = (lastSlash == string::npos)
                            ? filePath
                            : filePath.substr(lastSlash + 1);

        size_t dotPos = filename.find_last_of('.');
        currentProjectName = (dotPos == string::npos)
                              ? filename
                              : filename.substr(0, dotPos);

        isSaved = true;
        addToRecentProjects(filePath);
        cout << "Project saved to: " << filePath << endl;
        return true;
    }

    return false;
}


// ---- Load operation ----

bool ProjectManager::openProject(Circuit& circuit, const string& filePath)
{
    string content = readFromFile(filePath);

    if (content.empty())
    {
        cout << "Error: could not open file: " << filePath << endl;
        return false;
    }

    bool success = restoreCircuitFromSnapshot(circuit, content);

    if (success)
    {
        currentFilePath = filePath;

        size_t lastSlash = filePath.find_last_of("/\\");
        string filename  = (lastSlash == string::npos)
                            ? filePath
                            : filePath.substr(lastSlash + 1);

        size_t dotPos = filename.find_last_of('.');
        currentProjectName = (dotPos == string::npos)
                              ? filename
                              : filename.substr(0, dotPos);

        isSaved = true;
        addToRecentProjects(filePath);
        cout << "Project '" << currentProjectName << "' loaded." << endl;
    }

    return success;
}


// ---- Recent projects ----

const vector<string>& ProjectManager::getRecentProjects() const
{
    return recentProjects;
}

void ProjectManager::loadRecentProjectsList()
{
    recentProjects.clear();

    string content = readFromFile(RECENT_PROJECTS_FILE);
    if (content.empty())
    {
        return;
    }

    istringstream stream(content);
    string line;

    while (getline(stream, line))
    {
        if (!line.empty())
        {
            recentProjects.push_back(line);
        }
    }
}


// ---- State queries ----

bool   ProjectManager::getIsSaved()            const { return isSaved;            }
string ProjectManager::getCurrentFilePath()    const { return currentFilePath;    }
string ProjectManager::getCurrentProjectName() const { return currentProjectName; }


void ProjectManager::newProject(Circuit& circuit)
{
    circuit.clear();
    isSaved            = false;
    currentFilePath    = "";
    currentProjectName = "Untitled";
}


string ProjectManager::getDefaultExportFilename() const
{
    return currentProjectName + "_export.png";
}


// ---- Private helpers ----

bool ProjectManager::writeToFile(const string& filePath,
                                  const string& content)
{
    ofstream file(filePath);

    if (!file.is_open())
    {
        cout << "Error: could not write to file: " << filePath << endl;
        return false;
    }

    file << content;
    file.close();
    return true;
}


string ProjectManager::readFromFile(const string& filePath)
{
    ifstream file(filePath);

    if (!file.is_open())
    {
        return "";
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}


void ProjectManager::addToRecentProjects(const string& filePath)
{
    // Remove duplicate if it already exists
    for (int i = 0; i < (int)recentProjects.size(); i++)
    {
        if (recentProjects[i] == filePath)
        {
            recentProjects.erase(recentProjects.begin() + i);
            break;
        }
    }

    // Add to front (most recent first)
    recentProjects.insert(recentProjects.begin(), filePath);

    // Keep only the most recent MAX entries
    while ((int)recentProjects.size() > MAX_RECENT_PROJECTS)
    {
        recentProjects.pop_back();
    }

    saveRecentProjectsList();
}


void ProjectManager::saveRecentProjectsList()
{
    string content = "";

    for (int i = 0; i < (int)recentProjects.size(); i++)
    {
        content += recentProjects[i] + "\n";
    }

    writeToFile(RECENT_PROJECTS_FILE, content);
}


bool ProjectManager::fileExists(const string& filePath) const
{
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}


bool ProjectManager::parseComponentLine(Circuit& circuit,
                                         const string& line)
{
    vector<string> tokens = tokenize(line);
    if (tokens.empty())
    {
        return false;
    }

    string type = tokens[0];

    // Each component's serialize() format:
    // TYPE id label x y [type-specific values] rotation mirrored

    if (tokens.size() < 5)
    {
        return false;
    }

    string id    = tokens[1];
    string label = tokens[2];
    float  x     = stof(tokens[3]);
    float  y     = stof(tokens[4]);
    Vector2D pos(x, y);

    Component* comp = nullptr;

    if (type == "RESISTOR" && tokens.size() >= 8)
    {
        float resistance = stof(tokens[5]);
        comp = new Resistor(id, label, pos, resistance);
    }
    else if (type == "CAPACITOR" && tokens.size() >= 8)
    {
        float capacitance = stof(tokens[5]);
        comp = new Capacitor(id, label, pos, capacitance);
    }
    else if (type == "INDUCTOR" && tokens.size() >= 8)
    {
        float inductance = stof(tokens[5]);
        comp = new Inductor(id, label, pos, inductance);
    }
    else if (type == "GND" && tokens.size() >= 7)
    {
        comp = new GND(id, label, pos);
    }
    else if (type == "DCVOLTAGE" && tokens.size() >= 8)
    {
        float voltage = stof(tokens[5]);
        comp = new DCVoltageSource(id, label, pos, voltage);
    }
    else if (type == "BATTERY" && tokens.size() >= 9)
    {
        float emf      = stof(tokens[5]);
        float intRes   = stof(tokens[6]);
        comp = new Battery(id, label, pos, emf, intRes);
    }
    else if (type == "CLOCK" && tokens.size() >= 8)
    {
        int halfPeriod = stoi(tokens[5]);
        comp = new ClockGenerator(id, label, pos, halfPeriod);
    }
    else if (type == "SWITCH" && tokens.size() >= 8)
    {
        bool closed = (tokens[5] == "1");
        comp = new Switch(id, label, pos, closed);
    }
    else if (type == "BUTTON" && tokens.size() >= 7)
    {
        comp = new PushButton(id, label, pos);
    }
    else if (type == "LED" && tokens.size() >= 9)
    {
        int   colorCode = stoi(tokens[5]);
        float fwdV      = stof(tokens[6]);

        LED::LEDColor color = LED::LEDColor::RED;
        if (colorCode == 1) color = LED::LEDColor::GREEN;
        if (colorCode == 2) color = LED::LEDColor::BLUE;

        comp = new LED(id, label, pos, color, fwdV);
    }
    else if (type == "7SEG" && tokens.size() >= 7)
    {
        comp = new SevenSegment(id, label, pos);
    }
    else if (type == "AND" && tokens.size() >= 9)
    {
        int    numInputs = stoi(tokens[5]);
        double delay     = stod(tokens[6]);
        comp = new ANDGate(id, label, pos, numInputs, delay);
    }
    else if (type == "OR" && tokens.size() >= 9)
    {
        int    numInputs = stoi(tokens[5]);
        double delay     = stod(tokens[6]);
        comp = new ORGate(id, label, pos, numInputs, delay);
    }
    else if (type == "NOT" && tokens.size() >= 8)
    {
        double delay = stod(tokens[5]);
        comp = new NOTGate(id, label, pos, delay);
    }
    else if (type == "NAND" && tokens.size() >= 9)
    {
        int    numInputs = stoi(tokens[5]);
        double delay     = stod(tokens[6]);
        comp = new NANDGate(id, label, pos, numInputs, delay);
    }
    else if (type == "XOR" && tokens.size() >= 9)
    {
        int    numInputs = stoi(tokens[5]);
        double delay     = stod(tokens[6]);
        comp = new XORGate(id, label, pos, numInputs, delay);
    }
    else if (type == "DFF" && tokens.size() >= 8)
    {
        double delay = stod(tokens[5]);
        comp = new DFlipFlop(id, label, pos, delay);
    }

    if (comp != nullptr)
    {
        // Restore rotation and mirror state (last two tokens)
        // rotation = tokens[size-2], mirrored = tokens[size-1]
        int lastIdx = (int)tokens.size() - 1;

        // Apply rotation
        float savedRotation = stof(tokens[lastIdx - 1]);
        while (comp->getRotation() < savedRotation)
        {
            comp->rotate90();
        }

        circuit.addComponent(comp);
        return true;
    }

    cout << "Warning: unknown component type '" << type
         << "' — skipped." << endl;
    return false;
}


bool ProjectManager::parseWireLine(Circuit& circuit,
                                    const string& line)
{
    vector<string> tokens = tokenize(line);

    // Format: WIRE fromCompId fromPinName toCompId toPinName
    if (tokens.size() < 5 || tokens[0] != "WIRE")
    {
        return false;
    }

    string fromCompId  = tokens[1];
    string fromPinName = tokens[2];
    string toCompId    = tokens[3];
    string toPinName   = tokens[4];

    return circuit.addWire(fromCompId, fromPinName,
                           toCompId,   toPinName);
}


bool ProjectManager::parseJunctionLine(Circuit& circuit,
                                        const string& line)
{
    vector<string> tokens = tokenize(line);

    // Format: JUNCTION x y
    if (tokens.size() < 3 || tokens[0] != "JUNCTION")
    {
        return false;
    }

    float x = stof(tokens[1]);
    float y = stof(tokens[2]);

    circuit.addJunction(Vector2D(x, y));
    return true;
}


vector<string> ProjectManager::tokenize(const string& line)
{
    vector<string> tokens;
    istringstream  stream(line);
    string         token;

    while (stream >> token)
    {
        tokens.push_back(token);
    }

    return tokens;
}