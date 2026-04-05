//
// Team.h — 8INF259 TP3
//
// Une Team représente une équipe de techniciens qualifiés pour
// verrouiller certains types de dispositifs de sécurité.
//
// Chaque équipe possède :
//   - name          : nom de l'équipe (ex: "Alpha")
//   - capabilities  : la liste des TYPES de devices que cette équipe
//                     sait verrouiller (ex: ["BREAKER", "FUSE", "TAG"]).
//                     Un device de type "VALVE" ne peut être verrouillé
//                     QUE par une équipe dont les capabilities contiennent
//                     "VALVE".
//   - maxCapacity   : le nombre maximum de devices que cette équipe
//                     peut prendre en charge par machine. Si une machine
//                     a 10 devices et que l'équipe a une capacité de 3,
//                     elle ne peut en verrouiller que 3 — les autres
//                     doivent être assignés à une autre équipe capable.
//
// La méthode canHandle(deviceType) vérifie si l'équipe est qualifiée
// pour un type donné. Elle vous évitera de réécrire la même boucle
// de recherche à chaque assignation dans le Scheduler.
//
// IMPORTANT — Pourquoi un vector<Team*> et pas un unordered_map ?
//   Contrairement aux devices et machines qu'on recherche par ID,
//   les équipes sont toujours parcourues séquentiellement (on cherche
//   "la meilleure équipe capable parmi toutes"). Un vecteur est donc
//   la structure appropriée : accès séquentiel O(n) avec n petit.
//
// NOTE : si le champ "max_capacity" est absent dans le JSON, l'équipe
//        a une capacité illimitée (utilisez -1 comme valeur sentinelle).
//

#ifndef TEAM_H
#define TEAM_H

#include <string>
#include <vector>

class Team
{
private:
	/// Name of this team
	std::string name;

	/// List of every type of devices that this team can handle
	std::vector<std::string> capabilities;

	/// Maximum amount of devices per machine this team can handle
	int maxCapacity;

public:
	Team(const std::string& name, const std::vector<std::string>& capabilities, int maxCapacity);

	[[nodiscard]] std::string getName() const { return name; }
	[[nodiscard]] std::vector<std::string> getCapabilities() const { return capabilities; }
	[[nodiscard]] int getMaxCapacity() const { return maxCapacity; }

	[[nodiscard]] bool canHandle(const std::string& deviceType) const;

	/// Loads every team in the given file
	static std::vector<Team*> BuildTeamsFromJsonFile(const std::string& filename);
};

#endif // TEAM_H
