//
// Device.h — 8INF259 TP3
//
// Un Device représente un dispositif de sécurité physique utilisé dans
// une procédure de lockout/tagout : valve, disjoncteur, cadenas, fusible, etc.
//
// Chaque device possède :
//   - id        : identifiant unique (correspond à l'ID dans devices.json)
//   - type      : catégorie du device (ex: "VALVE", "BREAKER", "FUSE"...)
//                 C'est le type qui détermine QUELLE ÉQUIPE est qualifiée
//                 pour verrouiller ce device.
//   - name      : nom descriptif (ex: "Main Circuit Breaker")
//   - lockTime  : temps nécessaire pour verrouiller ce device, en unités
//                 abstraites. Un Flow Meter (lockTime=6) prend 6x plus de
//                 temps qu'un Tag (lockTime=1). Ce temps est utilisé par le
//                 Scheduler pour calculer la charge de chaque équipe.
//
// La méthode statique BuildDevicesFromJsonFile() lit le fichier JSON et
// construit tous les devices en mémoire.
//
// IMPORTANT — Pourquoi retourner un unordered_map<int, Device*> ?
//   Les machines référencent leurs devices par ID (tableau d'entiers dans
//   le JSON). Pour convertir ces IDs en pointeurs Device*, on a besoin d'un
//   accès par clé en O(1). Un vector obligerait à chercher séquentiellement
//   → O(n) par lookup, ce qui serait inefficace.
//
// IMPORTANT — Pourquoi des pointeurs (Device*) ?
//   Un même device peut être partagé entre plusieurs machines (ex: le
//   Main Circuit Breaker apparaît dans Boiler, Pump ET Press). On veut
//   UN SEUL objet en mémoire, référencé par plusieurs machines. Si on
//   stockait des copies (Device au lieu de Device*), modifier ou marquer
//   un device comme "déjà verrouillé" dans une machine ne serait pas
//   visible depuis les autres.
//
// NOTE : si le champ "lock_time" est absent dans le JSON, utilisez la
//        valeur par défaut 1.
//

#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <unordered_map>

using namespace std;

class Device {
private:
    int id;
    string type;
    string name;
    int lockTime;

public:
    Device(int id, const string& type, const string& name, int lockTime);

    int getId() const { return id; }
    string getType() const { return type; }
    string getName() const { return name; }
    int getLockTime() const { return lockTime; }

    static unordered_map<int, Device*> BuildDevicesFromJsonFile(const string& path);
};

#endif // DEVICE_H