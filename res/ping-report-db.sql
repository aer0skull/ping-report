CREATE TABLE HourlyReport(
    ping_max FLOAT NOT NULL,
    ping_min FLOAT NOT NULL,
    ping_mean FLOAT NOT NULL,
    nb_ping_high INT NOT NULL,
    nb_ping_loss INT NOT NULL,
    nb_ping_reached INT NOT NULL,
    report_day DATE NOT NULL,
    report_hour INT NOT NULL
);

CREATE VIEW DailyReport AS
SELECT
    H.report_day,
    MAX(H.ping_max) AS daily_worst_ping,
    MIN(H.ping_min) AS daily_best_ping,
    AVG(H.ping_mean) AS daily_mean_ping,
    SUM(H.nb_ping_high) AS daily_high_ping,
    SUM(H.nb_ping_loss) AS daily_loss_ping,
    SUM(H.nb_ping_reached) AS daily_reached_ping,
    (
        SELECT
            SH.report_hour
        FROM
            HourlyReport SH
        WHERE
            SH.ping_mean = (
                SELECT
                    MIN(SSH.ping_mean)
                FROM
                    HourlyReport SSH
                WHERE
                    SSH.report_day = sh.report_day
            )
        AND
            SH.report_day = H.report_day
    ) daily_best_hour,
    (
        SELECT
            SH.report_hour
        FROM
            HourlyReport SH
        WHERE
            SH.ping_mean = (
                SELECT
                    MAX(SSH.ping_mean)
                FROM
                    HourlyReport SSH
                WHERE
                    SSH.report_day = sh.report_day
            )
        AND
            SH.report_day = H.report_day
    ) daily_worst_hour
FROM
    HourlyReport H
GROUP BY
    report_day
