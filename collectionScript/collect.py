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


def store_in_db(data, topic):
    column_map = [
        ("temperature", "temp"),
        ("airPressure", "pressure"),
        ("humidity", "humidity"),
        ("voc", "voc"),
        ("airQuality", "airQuality"),
        ("airQualityAccuracy", "airQualityAccuracy"),
        ("co2", "co2"),
        ("bVoc", "bVoc"),
        ("voltage", "currentVoltage"),
    ]
    columns = []
    values = []
    for db_col, data_key in column_map:
        if data_key in data:
            columns.append(db_col)
            values.append(data[data_key])
    columns.append("topic")
    values.append(topic)

    if len(values) < 2:  # Only topic present, skip insert
        return

    conn = psycopg2.connect(**PG_CONN)
    cur = conn.cursor()
    query = f"INSERT INTO weather_measurements ({', '.join(columns)}) VALUES ({', '.join(['%s'] * len(values))})"

    cur.execute(query, values)
    conn.commit()
    cur.close()
    conn.close()


def on_message(client, userdata, msg):
    payload = json.loads(msg.payload.decode())
    print(f"Received: {payload} on topic: {msg.topic}")
    store_in_db(payload, msg.topic)


client = mqtt.Client()
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT)
client.subscribe(MQTT_TOPIC + "/#")
client.loop_forever()
