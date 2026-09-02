#include "emailservice.h"
#include "apikeys.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>

void EmailService::envoyerNotificationAssignation(const QString &toEmail,
                                                  const QString &formateurNom,
                                                  const QString &coursTitre,
                                                  const QString &dateDebut,
                                                  const QString &dateFin,
                                                  QWidget *parentPourMessages)
{
    if (toEmail.trimmed().isEmpty()) {
        qDebug() << "EmailService: aucune adresse email fournie, envoi annulé.";
        return;
    }

    // Le manager est parenté à qApp pour survivre le temps de la requête asynchrone
    auto *manager = new QNetworkAccessManager(qApp);

    QJsonObject templateParams;
    templateParams["to_email"] = toEmail;
    templateParams["formateur_nom"] = formateurNom;
    templateParams["cours_titre"] = coursTitre;
    templateParams["date_debut"] = dateDebut;
    templateParams["date_fin"] = dateFin;

    QJsonObject body;
    body["service_id"] = EMAILJS_SERVICE_ID;
    body["template_id"] = EMAILJS_TEMPLATE_ID;
    body["user_id"] = EMAILJS_PUBLIC_KEY;
    body["accessToken"] = EMAILJS_PRIVATE_KEY;
    body["template_params"] = templateParams;

    QNetworkRequest request(QUrl("https://api.emailjs.com/api/v1.0/email/send"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply, parentPourMessages, manager]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "EmailService: notification envoyée avec succès.";
            if (parentPourMessages) {
                QMessageBox::information(parentPourMessages, "Notification envoyée",
                                         "L'email de notification a été envoyé au formateur.");
            }
        } else {
            QString erreur = reply->errorString();
            QString reponse = reply->readAll();
            qDebug() << "EmailService: échec de l'envoi -" << erreur << reponse;
            if (parentPourMessages) {
                QMessageBox::warning(parentPourMessages, "Échec de l'envoi",
                                     "L'email n'a pas pu être envoyé.\n" + erreur);
            }
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}