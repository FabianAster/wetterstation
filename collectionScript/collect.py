import psycopg2
import paho.mqtt.client as mqtt
import json
from dotenv import load_dotenv
import os

load_dotenv()

MQTT_BROKER = os.getenv("MQTT_BROKER")
MQTT_PORT = int(os.getenv("MQTT_PORT"))
MQTT_TOPIC = os.getenv("MQTT_TOPIC")
MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")

PG_CONN = {
    "host": os.getenv("PG_HOST"),
    "database": os.getenv("PG_DATABASE"),
    "user": os.getenv("PG_USER"),
    "password": os.getenv("PG_PASSWORD"),
}


def store_in_db(data):
    conn = psycopg2.connect(**PG_CONN)
    cur = conn.cursor()
    cur.execute(
        """
        INSERT INTO weather_measurements (temperature, topic, airPressure, humidity, voltage)
        VALUES (%s, %s, %s, %s, %s)
        """,
        (
            data["temp"],
            MQTT_TOPIC,
            data["pressure"],
            data["humidity"],
            data["currentVoltage"],
        ),
    )
    conn.commit()
    cur.close()
    conn.close()


def on_message(client, userdata, msg):
    payload = json.loads(msg.payload.decode())
    print(f"Received: {payload}")
    store_in_db(payload)


client = mqtt.Client()
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT)
client.subscribe(MQTT_TOPIC)
client.loop_forever()
