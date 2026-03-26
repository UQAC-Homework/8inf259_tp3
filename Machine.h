//
// Machine.h — 8INF259 TP3
//
// Une Machine représente un équipement industriel (chaudière, pompe,
// convoyeur...) qui doit être verrouillé selon une procédure de
// lockout/tagout.
//
// Chaque machine possède :
//   - id              : identifiant unique
//   - name            : nom descriptif (ex: "Boiler A")
//   - relatedDevices  : la liste des dispositifs de sécurité (Device*)
//                       qu'il faut verrouiller pour isoler cette machine.
//   - dependsOn       : les IDs des machines qui DOIVENT être verrouillées
//                       AVANT celle-ci.
//
// IMPORTANT — Pourquoi des dépendances ?
//   Dans un environnement industriel, certaines machines alimentent
//   d'autres machines. Par exemple, une chaudière doit être isolée
//   AVANT la pompe qui en dépend — sinon on risque un retour de
//   pression dangereux. Ces dépendances forment un GRAPHE ORIENTÉ :
//   si la Pump dépend du Boiler, c'est une arête Boiler → Pump.
//   Ce graphe doit être ACYCLIQUE (DAG) : un cycle signifierait
//   qu'aucun ordre de verrouillage valide n'existe.
//
// IMPORTANT — Pourquoi des pointeurs (Device*) et pas des IDs ?
//   Le fichier machines.json stocke les devices comme des IDs entiers :
//     "related_devices": [1, 3, 7, 10, 15]
//   Mais dans le programme, on a besoin d'accéder aux informations du
//   device (son type, son nom, son lockTime). Stocker des pointeurs
//   permet d'y accéder directement en O(1) au lieu de re-chercher
//   dans la map à chaque fois.
//   C'est la méthode BuildMachinesFromJsonFile() qui fait cette
//   conversion : elle reçoit la map de devices déjà construite et
//   remplace chaque ID par le pointeur correspondant.
//
// NOTE : si le champ "depends_on" est absent dans le JSON, la machine
//        n'a aucun prérequis (vecteur vide).
//

#ifndef MACHINE_H
#define MACHINE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "Device.h"

using namespace std;

class Machine {
private:
    int id;
    string name;
    vector<Device*> relatedDevices;
    vector<int> dependsOn;

public:
    Machine(int id, const string& name);

    int getId() const { return id; }
    string getName() const { return name; }
    vector<Device*> getRelatedDevices() const { return relatedDevices; }
    vector<int> getDependsOn() const { return dependsOn; }

    static unordered_map<int, Machine*> BuildMachinesFromJsonFile(
        const string& path,
        const unordered_map<int, Device*>& devices);
};

#endif // MACHINE_H