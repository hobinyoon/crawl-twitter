use twitter4;

INSERT INTO users_crawled (id, crawled_at)
  SELECT id, crawled_at
  FROM users
  WHERE status IN ('NF', 'P');

DELETE FROM users WHERE status IN ('NF', 'P');
