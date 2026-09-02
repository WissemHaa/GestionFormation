#include "groqservice.h"
#include "apikeys.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>

void GroqService::poserQuestion(const QString &question,
                                const QString &contexte,
                                std::function<void(const QString&)> callback)
{
    auto *manager = new QNetworkAccessManager(qApp);

    QString promptSysteme =
        "Tu es un assistant intégré à une application de gestion de centre de formation. "
        "Tu réponds UNIQUEMENT en te basant sur les données fournies ci-dessous. "
        "Si l'information n'est pas dans les données, dis-le clairement au lieu d'inventer. "
        "Réponds de façon concise, en français.\n\n"
        "=== DONNÉES ACTUELLES ===\n" + contexte;

    QJsonArray messages;

    QJsonObject messageSysteme;
    messageSysteme["role"] = "system";
    messageSysteme["content"] = promptSysteme;
    messages.append(messageSysteme);

    QJsonObject messageUtilisateur;
    messageUtilisateur["role"] = "user";
    messageUtilisateur["content"] = question;
    messages.append(messageUtilisateur);

    QJsonObject body;
    body["model"] = "openai/gpt-oss-120b";
    body["messages"] = messages;
    body["temperature"] = 0.3;

    QNetworkRequest request(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(GROQ_API_KEY).toUtf8());

    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply, callback, manager]() {
        QString resultat;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();
            QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty()) {
                resultat = choices[0].toObject()["message"].toObject()["content"].toString();
            } else {
                resultat = "Réponse inattendue de l'IA.";
                qDebug() << "Groq: réponse inattendue -" << data;
            }
        } else {
            resultat = "Erreur lors de la communication avec l'assistant IA.";
            qDebug() << "Groq: erreur -" << reply->errorString() << reply->readAll();
        }

        callback(resultat);
        reply->deleteLater();
        manager->deleteLater();
    });
}