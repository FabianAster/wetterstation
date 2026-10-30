-- Sensirion SEN69C measurements.
-- Applied by Flyway and also idempotently by collectionScript/collect.py.

CREATE TABLE IF NOT EXISTS sen69c_measurements (
    id BIGSERIAL PRIMARY KEY,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    topic TEXT NOT NULL,
    pm1p0 DOUBLE PRECISION,
    pm2p5 DOUBLE PRECISION,
    pm4p0 DOUBLE PRECISION,
    pm10p0 DOUBLE PRECISION,
    temperature DOUBLE PRECISION,
    humidity DOUBLE PRECISION,
    voc_index DOUBLE PRECISION,
    nox_index DOUBLE PRECISION,
    hcho DOUBLE PRECISION,
    co2 INTEGER
);

CREATE INDEX IF NOT EXISTS idx_sen69c_measurements_created_at
    ON sen69c_measurements (created_at DESC);
