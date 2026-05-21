-- Passwords are "test"

INSERT INTO auth_user (username, password, role) VALUES
  ('wesker',  'pbkdf2_sha256$600000$a1b2c3d4e5f60001a1b2c3d4e5f60002$dd9d9f15012950ec171e57aa4a7be34f814a1c1b9875b9e73d6e587775e1dbf8', 'root'),
  ('jill',    'pbkdf2_sha256$600000$b2c3d4e5f6a70003b2c3d4e5f6a70004$03b4c3addeef494f4a2d4c9739a4428731c6074200968c3e41faf433da601dca', 'admin'),
  ('chris',   'pbkdf2_sha256$600000$c3d4e5f6a7b80005c3d4e5f6a7b80006$8cd4bbfd3b8a112e61b77af6f9ca4c2031e6be5681db3f73655963a9e7029fa5', 'guard'),
  ('barry',   'pbkdf2_sha256$600000$d4e5f6a7b8c90007d4e5f6a7b8c90008$a13cb317c9bc025aeaa096d08bb5408c97a81d8a28eb9e5555534013760e8de4', 'guard'),
  ('rebecca', 'pbkdf2_sha256$600000$e5f6a7b8c9d00009e5f6a7b8c9d0000a$149ebac4c611035f96ec3aeaaa5d2bf9a8a8a4154f12e512a98b8ff39676fc17', 'guard');

INSERT INTO cm_document_type (name, display_name) VALUES
  ('dni', 'DNI'),
  ('ca',  'Carné de Extranjería');

INSERT INTO cm_employee (name, document_number, document_type) VALUES
  ('Jill Valentine',    '10000001', 'dni'),
  ('Chris Redfield',    '10000002', 'dni'),
  ('Barry Burton',      '10000003', 'dni'),
  ('Rebecca Chambers',  '10000004', 'dni'),
  ('Albert Wesker',     '10000005', 'dni'),
  ('Richard Aiken',     '10000006', 'dni'),
  ('Enrico Marini',     'CE20000001', 'ca'),
  ('Forest Speyer',     'CE20000002', 'ca'),
  ('Brad Vickers',      '10000009', 'dni'),
  ('Joseph Frost',      '10000010', 'dni');
