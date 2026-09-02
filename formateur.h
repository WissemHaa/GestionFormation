#ifndef FORMATEUR_H
#define FORMATEUR_H

#include <QString>
#include <QDate>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QPair>

class Formateur {
private:
    int idFormateur;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString specialite;
    QDate dateEmbauche;
    QString statut;

public:
    Formateur();
    Formateur(int id, const QString &nom, const QString &prenom, const QString &email,
              const QString &telephone, const QString &specialite,
              const QDate &dateEmbauche, const QString &statut);


    // Getters
    int getIdFormateur() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getEmail() const;
    QString getTelephone() const;
    QString getSpecialite() const;
    QDate getDateEmbauche() const;
    QString getStatut() const;

    // Setters
    void setIdFormateur(int id);
    void setNom(const QString &nom);
    void setPrenom(const QString &prenom);
    void setEmail(const QString &email);
    void setTelephone(const QString &telephone);
    void setSpecialite(const QString &specialite);
    void setDateEmbauche(const QDate &date);
    void setStatut(const QString &statut);

    // ---- Requêtes CRUD (prepared statements) ----
    bool ajouter();                          // INSERT - utilise les attributs de l'objet courant
    bool modifier();                         // UPDATE - basé sur idFormateur
    bool supprimer();                        // DELETE - basé sur idFormateur
    static QList<Formateur> afficherTous();  // SELECT * - retourne la liste de tous les formateurs

    // Recherche/tri multicritères (on l'ajoutera plus tard)
    static QList<Formateur> rechercher(const QString &motCle, const QString &specialite,
                                       const QString &statut, const QString &colonneTri,
                                       bool ordreAscendant);
    static QStringList listeSpecialites(); // pour peupler les combobox dynamiquement
    static QMap<QString, int> statsParSpecialite();
    static QMap<QString, int> statsParStatut();

    static QList<QPair<int, QString>> listeIdNom();

    static Formateur getParId(int id);

};

#endif // FORMATEUR_H