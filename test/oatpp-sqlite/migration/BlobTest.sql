CREATE TABLE test_blobs (
  f_string      NUMERIC,
  f_blob        BLOB
);

INSERT INTO test_blobs
(f_string, f_blob) VALUES (null, null);

INSERT INTO test_blobs
(f_string, f_blob) VALUES ('', x'');

INSERT INTO test_blobs
(f_string, f_blob) VALUES ('hello world', x'68656c6c6f20776f726c64');
