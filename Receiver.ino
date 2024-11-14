#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "security.h" 


WiFiClientSecure espClient;
PubSubClient client(espClient);



// Fonction de callback pour recevoir les messages MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message reçu [");
    Serial.print(topic);
    Serial.print("] : ");
    

      if (length == 0) {
        Serial.println("Message vide reçu");
        return;
    } 
    // Convertir le payload en chaîne JSON
    char jsonBuffer[length + 1];
    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';

    // Afficher le JSON brut pour débogage
    Serial.print("Données brutes reçues : ");
    Serial.println(jsonBuffer);

    // Parser le JSON
   DynamicJsonDocument doc(200);


    DeserializationError error = deserializeJson(doc, jsonBuffer);
    if (error) {
        Serial.print("Erreur de parsing JSON : ");
        Serial.println(error.c_str());
        return;
    }

    // Extraire et afficher les valeurs
    float temperature = doc["temperature"];
    float humidity = doc["humidity"];
    Serial.print("Température : ");
    Serial.print(temperature);
    Serial.print(" °C, Humidité : ");
    Serial.print(humidity);
    Serial.println(" %");

    // Republier les données pour le frontend
    client.publish("esp32/frontend/data", jsonBuffer, true);
}

void setup() {
    Serial.begin(115200);

    // Connexion au WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnecté au WiFi");

    // Configuration du serveur MQTT et du callback
    //client.setServer(mqtt_server, mqtt_port);
    //client.setCallback(callback);
    // Configuration du serveur MQTT sécurisé
    

    espClient.setCACert(root_ca); // Ajouter le certificat racine pour sécuriser la connexion
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    /* Connexion au broker MQTT
    while (!client.connected()) {
        Serial.print("Connexion au broker MQTT...");
        if (client.connect("ESP32_Receiver")) {
            Serial.println("Connecté !");
            client.subscribe("esp32/sensor");
        } else {
            Serial.print("Échec, rc=");
            Serial.print(client.state());
            Serial.println(" Nouvelle tentative dans 5 secondes...");
            delay(5000);
        }
    }*/

    // Connexion au broker MQTT avec authentification
    while (!client.connected()) {
        Serial.print("Connexion au broker MQTT...");
        if (client.connect("ESP32_Receiver", mqtt_user, mqtt_password)) {
            Serial.println("Connecté !");
            client.subscribe("esp32/dht22/data");  // Remplacez "esp32/sensor" par un topic de test si nécessaire
        } else {
            Serial.print("Échec, rc=");
            Serial.print(client.state());
            Serial.println(" Nouvelle tentative dans 5 secondes...");
            delay(5000);
        }
    }
}

void loop() {
  // Vérifier et reconnecter au WiFi si déconnecté
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi déconnecté. Tentative de reconnexion...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print("pas connecté au wifi.");
        }
        Serial.println("\nReconnecté au WiFi");
    }
    //Vérifier et reconnecter au MQTT si déconnecté
   if (!client.connected()) {
        Serial.println("Client MQTT déconnecté. Tentative de reconnexion...");
        while (!client.connected()) {
            if (client.connect("ESP32_Receiver", mqtt_user, mqtt_password)) {
                Serial.println("Reconnecté au broker MQTT.");
                client.subscribe("esp32/dht22/data");
            } else {
                Serial.print("Reconnect échoué, rc=");
                Serial.print(client.state());
                Serial.println(". Nouvelle tentative dans 5 secondes...");
                delay(5000);
            }
        }
    }
    client.loop(); // Maintenir la connexion MQTT active
}
