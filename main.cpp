//
// main.cpp — 8INF259 TP3
//
// Phase 1 : Chargement (Device, Machine, Team)
// Phase 2 : Logique de base via Manager (rétrocompatibilité)
// Phase 3 : Ordonnancement avancé via Scheduler
// Phase 4 : Déverrouillage LIFO
//

#include <iostream>
#include "Device.h"
#include "Machine.h"
#include "Team.h"
#include "Scheduler.h"

// Changer le ASSETS_TYPE pour : small, big, errors ou balanced (en fonction de vos tests)
#define ASSETS_TYPE balanced 

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)
#define ASSETS_PATH(file) ("./assets/" TOSTRING(ASSETS_TYPE) "/" file)

using namespace std;

int main() {
    // =======================================================
    //  PHASE 1 : Chargement des données
    // =======================================================
    const auto devices  = Device::BuildDevicesFromJsonFile(ASSETS_PATH("devices.json"));
    const auto machines = Machine::BuildMachinesFromJsonFile(ASSETS_PATH("machines.json"), devices);
    const auto teams    = Team::BuildTeamsFromJsonFile(ASSETS_PATH("teams.json"));

    // --- Construction du Scheduler ---
    Scheduler scheduler(devices, machines, teams);

    // --- Pré-scheduling : infos sur les données ---
    scheduler.displayNotConnectedDevices();
    scheduler.displayTeams();

    // --- Graphe de dépendances ---
    scheduler.buildDependencyGraph();

    if (scheduler.hasCycle()) {
        cerr << "ERREUR FATALE : dependance circulaire detectee !" << endl;
        return 1;
    }

    // --- Tri topologique ---
    // La lambda sert de tie-breaker quand plusieurs machines
    // sont prêtes en même temps (même niveau dans le DAG).
    scheduler.topologicalSort([](const Machine* m1, const Machine* m2) {
        return m1->getId() < m2->getId();
    });

    // --- Scheduling glouton ---
    scheduler.schedule();

    // --- Affichage des résultats ---
    scheduler.displaySummary();
    scheduler.displayGantt();
    scheduler.displayStats();

    return 0;
}