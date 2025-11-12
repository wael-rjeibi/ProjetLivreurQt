#include "livreur.h"

// Constructeur par défaut
Livreur::Livreur()
{
}

// Constructeur avec paramètres
Livreur::Livreur(const QString &id, const QString &nom, const QString &prenom, int experience)
    : m_id(id), m_nom(nom), m_prenom(prenom), m_experience(experience)
{
}

// Accesseurs
QString Livreur::getId() const
{
    return m_id;
}

QString Livreur::getNom() const
{
    return m_nom;
}

QString Livreur::getPrenom() const
{
    return m_prenom;
}

int Livreur::getExperience() const
{
    return m_experience;
}

// Mutateurs
void Livreur::setId(const QString &id)
{
    m_id = id;
}

void Livreur::setNom(const QString &nom)
{
    m_nom = nom;
}

void Livreur::setPrenom(const QString &prenom)
{
    m_prenom = prenom;
}

void Livreur::setExperience(int experience)
{
    m_experience = experience;
}
