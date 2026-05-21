-- migrate:up

CREATE TABLE auth_user (
  username TEXT PRIMARY KEY,
  password TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
  last_logged_at TEXT,
  role TEXT NOT NULL DEFAULT 'guard' CHECK (role IN ('root', 'admin', 'guard')),
  enabled INTEGER NOT NULL DEFAULT 1 CHECK (enabled IN (0, 1))
);

CREATE TABLE auth_session (
  key TEXT PRIMARY KEY,
  username TEXT NOT NULL,
  revoked INTEGER NOT NULL DEFAULT 0 CHECK (revoked IN (0, 1)),
  expires_at TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (username) REFERENCES auth_user (username)
);

CREATE TABLE cm_document_type (
  name TEXT PRIMARY KEY,
  display_name TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE cm_employee (
  name TEXT NOT NULL,
  document_number TEXT NOT NULL,
  document_type TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (document_type) REFERENCES cm_document_type (name)
);

-- migrate:down

DROP TABLE IF EXISTS cm_employee;
DROP TABLE IF EXISTS cm_document_type;
DROP TABLE IF EXISTS auth_session;
DROP TABLE IF EXISTS auth_user;
