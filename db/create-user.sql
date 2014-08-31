CREATE USER 'twitter'@'localhost' IDENTIFIED BY 'something';
GRANT ALL PRIVILEGES ON *.* TO 'twitter'@'localhost' WITH GRANT OPTION;
CREATE USER 'twitter'@'%' IDENTIFIED BY 'something';
GRANT ALL PRIVILEGES ON *.* TO 'twitter'@'%' WITH GRANT OPTION;
