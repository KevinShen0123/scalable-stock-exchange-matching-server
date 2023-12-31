DROP TABLE IF EXISTS POSITION CASCADE;
DROP TABLE IF EXISTS ACCOUNT CASCADE;
DROP TABLE IF EXISTS ORDERS CASCADE;
DROP TABLE IF EXISTS EXECUTED_ORDERS CASCADE;

CREATE TABLE ACCOUNT(
ACCOUNT_NUMBER DOUBLE PRECISION PRIMARY KEY,
BALANCE   DOUBLE PRECISION 
);
CREATE TABLE POSITION(
POSITION_ID SERIAL PRIMARY KEY,
SYMBOL_NAME  VARCHAR(256),
AMOUNT       DOUBLE PRECISION,
ACCOUNT_NUMBER DOUBLE PRECISION,
FOREIGN KEY(ACCOUNT_NUMBER) REFERENCES ACCOUNT(ACCOUNT_NUMBER)
);
CREATE TABLE ORDERS(
   ORDER_ID SERIAL PRIMARY KEY,
  SYMBOL_NAME VARCHAR(256),
  AMOUNT   DOUBLE PRECISION,
  PRICE_LIMIT DOUBLE PRECISION,
  EXECUTED_AMOUNT DOUBLE PRECISION,
  CANCELED_AMOUNT DOUBLE PRECISION,
  LAST_UPDATE   VARCHAR(256),
  ACCOUNT_NUMBER DOUBLE PRECISION,
  FOREIGN KEY(ACCOUNT_NUMBER) REFERENCES ACCOUNT(ACCOUNT_NUMBER),
  EXECUTE_TIME VARCHAR(256),
  EXECUTE_PRICE DOUBLE PRECISION
);
CREATE TABLE EXECUTED_ORDERS(
   ORDER_ID  INT,
  SYMBOL_NAME VARCHAR(256),
  EXECUTED_AMOUNT DOUBLE PRECISION,
  ACCOUNT_NUMBER DOUBLE PRECISION,
  FOREIGN KEY(ACCOUNT_NUMBER) REFERENCES ACCOUNT(ACCOUNT_NUMBER),
  EXECUTE_TIME VARCHAR(256),
  EXECUTE_PRICE DOUBLE PRECISION
);





