
#ifndef VEHICULE_H
#define VEHICULE_H

#include <QString>

/**
 * @brief La classe Vehicule représente un véhicule avec un ID, une marque, une matriculation et une année de véhicule.
 */
class Vehicule
{
public:
    // === Constructeurs ===
    Vehicule();  // constructeur par défaut
    Vehicule(int id, const QString &marque, const QString &matriculation, int anneeVehicule);

    // === Getters ===
    int getId() const;
    QString getMarque() const;
    QString getMatriculation() const;
    int getAnneeVehicule() const;

    // === Setters ===
    void setId(int id);
    void setMarque(const QString &marque);
    void setMatriculation(const QString &matriculation);
    void setAnneeVehicule(int anneeVehicule);

private:
    // === Attributs ===
    int id;
    QString marque;
    QString matriculation;
    int anneeVehicule;
};

#endif // VEHICULE_H
