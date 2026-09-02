#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <QString>
#include <QWidget>

class EmailService {
public:
    // Envoie une notification d'assignation de cours à un formateur via EmailJS.
    // parentPourMessages : widget parent pour afficher un QMessageBox de confirmation/erreur (peut être nullptr).
    static void envoyerNotificationAssignation(const QString &toEmail,
                                               const QString &formateurNom,
                                               const QString &coursTitre,
                                               const QString &dateDebut,
                                               const QString &dateFin,
                                               QWidget *parentPourMessages = nullptr);
};

#endif // EMAILSERVICE_H