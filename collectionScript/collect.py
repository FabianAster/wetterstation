#!/usr/bin/env python3
"""MQTT -> PostgreSQL collector.

Handles two sensors on one service:

  * MQTT_TOPIC/#         -> weather_measurements   (weather/air-quality station)
  * MQTT_TOPIC_SEN69C/#  -> sen69c_measurements    (Sensirion SEN69C)

The SEN69C table is created idempotently on startup from the Flyway migration
database/migrations/V4__create_sen69c_measurements.sql.
"""

import json
import logging
import os
import sys
import time
from pathlib import Path

import psycopg2
import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv()

MQTT_BROKER = os.getenv("MQTT_BROKER")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "Wetterstation/measurements")
MQTT_TOPIC_SEN69C = os.getenv("MQTT_TOPIC_SEN69C", "Sensirion/measurements")
MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")

PG_CONN = {
    "host": os.getenv("PG_HOST"),
    "port": int(os.getenv("PG_PORT", "5432")),
    "database": os.getenv("PG_DATABASE"),
    "user": os.getenv("PG_USER"),
    "password": os.getenv("PG_PASSWORD"),
}

SEN69C_SCHEMA = (
    Path(__file__).resolve().parent.parent
    / "database"
    / "migrations"
    / "V4__create_sen69c_measurements.sql"
)

# SEN69C payload key -> database column
SEN69C_FIELDS = (
    ("pm1p0", "pm1p0"),
    ("pm2p5", "pm2p5"),
    ("pm4p0", "pm4p0"),
    ("pm10p0", "pm10p0"),
    ("temperature", "temperature"),
    ("humidity", "humidity"),
    ("vocIndex", "voc_index"),
    ("noxIndex", "nox_index"),
    ("hcho", "hcho"),
    ("co2", "co2"),
)

SEN69C_INSERT_SQL = """
    INSERT INTO sen69c_measurements (
        topic, pm1p0, pm2p5, pm4p0, pm10p0, temperature, humidity,
        voc_index, nox_index, hcho, co2
    ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
"""


def store_in_db(data, topic):
    """Store a weather-station measurement using the generic column map."""
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


def store_sen69c(data, topic):
    """Store a Sensirion SEN69C measurement."""
    values = [data.get(payload_key) for payload_key, _ in SEN69C_FIELDS]
    conn = psycopg2.connect(**PG_CONN)
    cur = conn.cursor()
    cur.execute(SEN69C_INSERT_SQL, [topic, *values])
    conn.commit()
    cur.close()
    conn.close()


def ensure_sen69c_schema():
    """Create the SEN69C table/index if they do not exist yet.

    Postgres runs in Docker and may still be starting when systemd starts this
    service, so retry instead of exiting and hitting the restart limit.
    """
    sql = SEN69C_SCHEMA.read_text()
    while True:
        conn = None
        try:
            conn = psycopg2.connect(**PG_CONN)
            with conn, conn.cursor() as cur:
                cur.execute(sql)
            logging.info("SEN69C database schema is ready")
            return
        except psycopg2.OperationalError as exc:
            logging.error("Database not ready: %s; retrying in 5s", exc)
            time.sleep(5)
        finally:
            if conn is not None:
                conn.close()


def is_sen69c_topic(topic):
    return topic == MQTT_TOPIC_SEN69C or topic.startswith(MQTT_TOPIC_SEN69C + "/")


def on_connect(client, userdata, flags, reason_code, properties=None):
    if getattr(reason_code, "value", reason_code) == 0:
        logging.info("Connected to MQTT broker %s:%s", MQTT_BROKER, MQTT_PORT)
        client.subscribe(MQTT_TOPIC + "/#")
        client.subscribe(MQTT_TOPIC_SEN69C + "/#")
        logging.info("Subscribed to %s/# and %s/#", MQTT_TOPIC, MQTT_TOPIC_SEN69C)
    else:
        logging.error("MQTT connection failed: %s", reason_code)


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        logging.warning("Ignoring invalid JSON on %s: %s", msg.topic, exc)
        return

    if not isinstance(payload, dict):
        logging.warning("Ignoring non-object payload on %s", msg.topic)
        return

    logging.info("Received on %s: %s", msg.topic, payload)
    try:
        if is_sen69c_topic(msg.topic):
            store_sen69c(payload, msg.topic)
        else:
            store_in_db(payload, msg.topic)
    except (psycopg2.Error, KeyError) as exc:
        logging.error("Database insert failed on %s: %s", msg.topic, exc)


def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    required = {
        "MQTT_BROKER": MQTT_BROKER,
        "PG_HOST": PG_CONN["host"],
        "PG_DATABASE": PG_CONN["database"],
        "PG_USER": PG_CONN["user"],
        "PG_PASSWORD": PG_CONN["password"],
    }
    missing = [name for name, value in required.items() if not value]
    if missing:
        sys.exit(f"Missing environment variables: {', '.join(missing)}")

    ensure_sen69c_schema()

    # paho-mqtt 2.x needs an explicit callback API version; 1.x does not.
    try:
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    except AttributeError:
        client = mqtt.Client()

    if MQTT_USERNAME and MQTT_PASSWORD:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    client.on_connect = on_connect
    client.on_message = on_message
    client.reconnect_delay_set(min_delay=1, max_delay=30)

    # The broker may still be starting when systemd starts this service.
    # Keep retrying instead of exiting and hitting systemd's restart limit.
    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            break
        except OSError as exc:
            logging.error("MQTT connect to %s:%s failed: %s; retrying in 5s", MQTT_BROKER, MQTT_PORT, exc)
            time.sleep(5)

    client.loop_forever()


if __name__ == "__main__":
    main()
