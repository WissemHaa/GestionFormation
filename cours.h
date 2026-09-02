#ifndef COURS_H
#define COURS_H

#include <QString>
#include <QDate>
#include <QList>
#include <QMap>

class Cours {
private:
    int idCours;
    QString titre;
    QString description;
    int dureeHeures;
    QString niveau;
    QDate dateDebut;
    QDate dateFin;
    int idFormateur;        // -1 ou 0 = non assigné
    QString nomFormateur;   // rempli uniquement par les jointures (affichage)
    QString statut;

public:
    Cours();
    Cours(int id, const QString &titre, const QString &description, int duree,
          const QString &niveau, const QDate &debut, const QDate &fin,
          int idFormateur, const QString &nomFormateur, const QString &statut);

    // Getters
    int getIdCours() const;
    QString getTitre() const;
    QString getDescription() const;
    int getDureeHeures() const;
    QString getNiveau() const;
    QDate getDateDebut() const;
    QDate getDateFin() const;
    int getIdFormateur() const;
    QString getNomFormateur() const;
    QString getStatut() const;

    // Setters
    void setIdCours(int id);
    void setTitre(const QString &t);
    void setDescription(const QString &d);
    void setDureeHeures(int d);
    void setNiveau(const QString &n);
    void setDateDebut(const QDate &d);
    void setDateFin(const QDate &d);
    void setIdFormateur(int id);
    void setStatut(const QString &s);

    // CRUD
    bool ajouter();
    bool modifier();
    bool supprimer();
    static QList<Cours> afficherTous();
    static QList<Cours> rechercher(const QString &motCle, const QString &niveau,
                                   const QString &statut, const QString &colonneTri,
                                   bool ordreAscendant);

    static QMap<QString, int> statsParNiveau();
    static QMap<QString, int> statsParStatut();

    static QList<Cours> listerParFormateur(int idFormateur);

};

#endif // COURS_H