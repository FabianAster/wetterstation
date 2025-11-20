-- Create a users table
CREATE TABLE weather_measurements (
    id SERIAL PRIMARY KEY,
    created_at TIMESTAMP DEFAULT NOW(),
    topic TEXT NOT NULL,
    temperature DECIMAL(20,10) NOT NULL,
    airPressure DECIMAL(20,10) NOT NULL,
    humidity DECIMAL(20,10) NOT NULL,
    voltage DECIMAL(20,10) NOT NULL
);

