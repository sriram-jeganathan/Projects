-- Run once against your database before rebuilding:
--   mysql -u sriram -p invoice_system < alter.sql

USE invoice_system;

ALTER TABLE invoices
    ADD COLUMN cgst            DECIMAL(10,2) NOT NULL DEFAULT 0 AFTER gst_total,
    ADD COLUMN sgst            DECIMAL(10,2) NOT NULL DEFAULT 0 AFTER cgst,
    ADD COLUMN igst            DECIMAL(10,2) NOT NULL DEFAULT 0 AFTER sgst,
    ADD COLUMN place_of_supply VARCHAR(64) DEFAULT NULL          AFTER igst;

-- Optional: backfill old invoices, assuming they were intra-state
-- (splits the existing gst_total evenly into CGST + SGST).
UPDATE invoices
SET cgst = gst_total / 2, sgst = gst_total / 2
WHERE igst = 0 AND gst_total > 0 AND cgst = 0 AND sgst = 0;
