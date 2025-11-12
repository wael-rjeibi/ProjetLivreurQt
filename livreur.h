#ifndef LIVREUR_H
#define LIVREUR_H

#include <QString>

/**
 * @brief La classe Livreur représente un livreur avec un ID, un nom, un prénom et une expérience.
 */
class Livreur
{
public:
    // Constructeurs
    Livreur();
    Livreur(const QString &id, const QString &nom, const QString &prenom, int experience);

    // Accesseurs (getters)
    QString getId() const;
    QString getNom() const;
    QString getPrenom() const;
    int getExperience() const;

    // Mutateurs (setters)
    void setId(const QString &id);
    void setNom(const QString &nom);
    void setPrenom(const QString &prenom);
    void setExperience(int experience);

private:
    QString m_id;          // Identifiant du livreur
    QString m_nom;         // Nom du livreur
    QString m_prenom;      // Prénom du livreur
    int m_experience;      // Années d'expérience du livreur
};

#endif // LIVREUR_H
