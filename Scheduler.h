//
// Scheduler.h — 8INF259 TP3
//
// Le Scheduler est la classe centrale du projet. C'est lui qui orchestre
// tout le processus de verrouillage des machines.
//
// ═══════════════════════════════════════════════════════════════
//  CE FICHIER CONTIENT UNIQUEMENT LES MÉTHODES PUBLIQUES.
//  C'EST À VOUS DE DÉFINIR :
//    - Les attributs privés (quelles structures de données stocker ?)
//    - Les méthodes privées (quelles fonctions utilitaires créer ?)
//    - Les structs/classes auxiliaires si nécessaire
//  Réfléchissez à ce dont chaque méthode a besoin pour fonctionner.
// ═══════════════════════════════════════════════════════════════
//
// Le Scheduler reçoit en entrée les devices, machines et équipes
// déjà chargés en mémoire. Son travail se décompose en étapes :
//
// ─── ÉTAPE 1 : Analyse des données ─────────────────────────────
//
//   displayNotConnectedDevices()
//     Certains devices dans devices.json ne sont rattachés à aucune
//     machine. Ce sont des "orphelins" — potentiellement des erreurs
//     de configuration ou des pièces de réserve. Cette méthode les
//     identifie et les affiche.
//     → Réfléchissez : comment savoir efficacement si un device est
//       référencé par au moins une machine ?
//
//   displayTeams()
//     Affiche les équipes avec leurs capabilities et leur capacité.
//     Simple affichage, pas de logique complexe.
//
// ─── ÉTAPE 2 : Graphe de dépendances ───────────────────────────
//
//   buildDependencyGraph()
//     Les machines ont des dépendances entre elles : si la Pump
//     dépend du Boiler, le Boiler doit être verrouillé EN PREMIER.
//     Cette méthode construit un GRAPHE ORIENTÉ à partir des
//     champs "depends_on" des machines.
//     → Réfléchissez : quelle structure de données pour représenter
//       un graphe orienté ? Comment stocker les arêtes ?
//
//   hasCycle()
//     Un cycle dans les dépendances (A dépend de B, B dépend de C,
//     C dépend de A) signifie qu'aucun ordre valide n'existe. Il faut
//     le détecter et le signaler comme erreur.
//     → Réfléchissez : comment détecter un cycle dans un graphe orienté ?
//
//   topologicalSort(tieBreaker)
//     Trouve un ordre de verrouillage qui RESPECTE les dépendances :
//     une machine n'est traitée que quand TOUS ses prérequis sont faits.
//     C'est un parcours en largeur (BFS) sur le graphe de dépendances :
//       1. On part des machines sans prérequis (aucune dépendance)
//       2. On les traite
//       3. On "libère" les machines qui dépendaient d'elles
//       4. Celles qui n'ont plus de prérequis en attente entrent dans la file
//       5. On répète jusqu'à ce que toutes les machines soient traitées
//
//     Le paramètre tieBreaker est une lambda (comme celles vues en cours) :
//     quand PLUSIEURS machines sont prêtes en même temps (même niveau
//     dans le graphe), on utilise le tieBreaker pour choisir laquelle
//     passer en premier. Ex: trier par ID croissant.
//     → Réfléchissez : une simple file (queue) suffit-elle pour gérer
//       le tie-breaking, ou faut-il une structure plus adaptée ?
//
// ─── ÉTAPE 3 : Ordonnancement ──────────────────────────────────
//
//   schedule()
//     C'est ici que le vrai travail se fait. Pour chaque machine
//     (dans l'ordre trouvé par topologicalSort), pour chaque device
//     de cette machine :
//
//       a) Si le device a DÉJÀ été verrouillé par une machine
//          précédente → on le passe (un device ne se verrouille
//          qu'une seule fois, même s'il est partagé).
//
//       b) Sinon, on l'assigne à une équipe. L'équipe choisie doit :
//          - Être CAPABLE de gérer ce type de device (capabilities)
//          - Ne pas avoir DÉPASSÉ sa capacité pour cette machine
//          - Avoir la CHARGE la plus faible parmi les candidates
//            (pour équilibrer le travail entre les équipes et
//            minimiser le temps total)
//
//     Le temps de début d'un device = le temps total actuel de
//     l'équipe choisie. Le temps de fin = début + lockTime du device.
//
//     → Réfléchissez : comment suivre efficacement quels devices ont
//       déjà été verrouillés ? Quelle complexité pour la recherche ?
//     → Réfléchissez : comment choisir la "meilleure" équipe parmi
//       les candidates ?
//
//   getMakespan()
//     Retourne le MAKESPAN : le temps de la dernière opération de la
//     dernière équipe à finir. C'est le max des temps totaux de toutes
//     les équipes. Plus il est bas, meilleur est l'ordonnancement.
//
// ─── ÉTAPE 4 : Affichage des résultats ─────────────────────────
//
//   displaySummary()
//     Affiche l'ordre topologique, puis un résumé par machine :
//     combien de devices verrouillés, combien ignorés ("already locked"),
//     et lesquels ont été ignorés.
//
//   displayGantt()
//     Affiche un diagramme de Gantt en colonnes :
//       - Chaque colonne = une équipe
//       - Chaque ligne = une unité de temps (T=0, T=1, T=2, ...)
//       - Chaque cellule = le device en cours de verrouillage
//       - Cellule vide = l'équipe est inactive à ce moment
//       - Si un device prend plusieurs unités de temps, les lignes
//         suivantes affichent  "  pour indiquer la continuation
//     Ce diagramme permet de visualiser le parallélisme entre équipes.
//
//   displayStats()
//     Affiche les statistiques finales :
//       - Le makespan
//       - Le nombre de devices et le temps total par équipe
//       - L'équipe "goulot d'étranglement" (celle qui finit en dernier)
//       - Le nombre de devices verrouillés vs total
//       - Le nombre de "already locked" skips
//       - L'efficacité : temps utile / (makespan × nb équipes) × 100
//         Une efficacité de 100% signifie que toutes les équipes ont
//         travaillé sans interruption jusqu'à la fin. Une efficacité
//         basse signifie que certaines équipes finissent bien avant
//         les autres (déséquilibre de charge).
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Device.h"
#include "Machine.h"
#include "Team.h"

using namespace std;

class Scheduler {
private:
    // ════════════════════════════════════════════════════════════
    //  À VOUS DE COMPLÉTER :
    //  Quels attributs ? Quelles structures de données ?
    //  Quelles méthodes utilitaires privées ?
    //
    //  Pensez à ce dont chaque méthode publique a besoin :
    //    - Stocker les données d'entrée (devices, machines, teams)
    //    - Représenter le graphe de dépendances
    //    - Suivre l'état du scheduling (qui est verrouillé, charge
    //      de chaque équipe, résultats par machine...)
    //    - Stocker le makespan
    // ════════════════════════════════════════════════════════════

public:
    Scheduler(const unordered_map<int, Device*>& devices,
              const unordered_map<int, Machine*>& machines,
              const vector<Team*>& teams);

    // Analyse des données
    void displayNotConnectedDevices();
    void displayTeams();

    // Graphe + tri topologique
    void buildDependencyGraph();
    bool hasCycle() const;
    vector<int> topologicalSort(
        const function<bool(const Machine*, const Machine*)>& tieBreaker);

    // Scheduling
    void schedule();
    int getMakespan() const;

    // Affichage
    void displaySummary();
    void displayGantt();
    void displayStats();
};

#endif // SCHEDULER_H