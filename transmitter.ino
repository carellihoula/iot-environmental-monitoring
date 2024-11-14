#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h> 
#include <DHT.h>
#include <ArduinoJson.h>
#include "security.h" // security file



// Définir le type de capteur et le pin GPIO utilisé
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure espClient;
PubSubClient client(espClient);



void setup() {
    Serial.begin(115200);
    dht.begin();

    // Connexion au WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnecté au WiFi");

    // Configuration du serveur MQTT
    espClient.setCACert(root_ca); // Ajouter le certificat racine pour sécuriser la connexion
    client.setServer(mqtt_server, mqtt_port);
    

    // Connexion au broker MQTT avec authentification
    while (!client.connected()) {
        Serial.print("Connexion au broker MQTT...");
        if (client.connect("ESP32_DHT22", mqtt_user, mqtt_password)) {
            Serial.println("Connecté !");
              
        } else {
            Serial.print("Échec, rc=");
            Serial.print(client.state());
            Serial.println(" Nouvelle tentative dans 5 secondes...");
            delay(5000);
        }
    }
}

void loop() {
    // Lire la température et l'humidité
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Erreur de lecture du capteur DHT22 !");
        return;
    }

    // Créer un objet JSON
    DynamicJsonDocument doc(200);
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;

    // Convertir en JSON
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);

    // Publier les données
    if (client.publish("esp32/dht22/data", jsonBuffer)) {
        Serial.println("Données envoyées au topic MQTT : esp32/sensor");
    } else {
        Serial.println("Échec de l'envoi des données");
    }

    delay(2000);
}
