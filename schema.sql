CREATE DATABASE IF NOT EXISTS findly;
USE findly;

CREATE TABLE IF NOT EXISTS users (
    email VARCHAR(100) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    password_hash VARCHAR(100) NOT NULL,
    role VARCHAR(10) NOT NULL DEFAULT 'student'
);

CREATE TABLE IF NOT EXISTS reports (
    id INT PRIMARY KEY,
    type VARCHAR(5) NOT NULL,
    reporter_email VARCHAR(100) NOT NULL,
    status INT NOT NULL DEFAULT 0,
    name VARCHAR(100) NOT NULL,
    category VARCHAR(50) NOT NULL,
    location VARCHAR(100) NOT NULL,
    description VARCHAR(255),
    secret_detail VARCHAR(255),
    FOREIGN KEY (reporter_email) REFERENCES users(email)
);
