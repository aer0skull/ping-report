CREATE TABLE ping(
    ping_value FLOAT NOT NULL,
    ping_ts INT NOT NULL
);

CREATE TABLE ping_stats(
    ping_max FLOAT NOT NULL,
    ping_min FLOAT NOT NULL,
    ping_mean FLOAT NOT NULL,
    ping_high INT NOT NULL,
    ping_loss INT NOT NULL,
    ping_reached INT NOT NULL,
    ping_stats_ts INT NOT NULL
);

