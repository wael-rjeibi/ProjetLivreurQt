#include "vehicule.h"

// Constructeur par défaut
Vehicule::Vehicule()
{
}

// Constructeur avec paramètres
Vehicule::Vehicule(int id, const QString &marque, const QString &matriculation, int anneeVehicule)
    : id(id), marque(marque), matriculation(matriculation), anneeVehicule(anneeVehicule)
{
}

// Getters
int Vehicule::getId() const
{
    return id;
}

QString Vehicule::getMarque() const
{
    return marque;
}

QString Vehicule::getMatriculation() const
{
    return matriculation;
}

int Vehicule::getAnneeVehicule() const
{
    return anneeVehicule;
}

// Setters
void Vehicule::setId(int i)
{
    id = i;
}

void Vehicule::setMarque(const QString &m)
{
    marque = m;
}

void Vehicule::setMatriculation(const QString &mat)
{
    matriculation = mat;
}

void Vehicule::setAnneeVehicule(int annee)
{
    anneeVehicule = annee;
}
