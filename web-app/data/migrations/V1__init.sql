CREATE TABLE berytus_account(
    AccountID BIGINT UNSIGNED AUTO_INCREMENT NOT NULL,
    PRIMARY KEY (AccountID)
);

CREATE TABLE berytus_account_user_attributes(
    AccountID BIGINT UNSIGNED AUTO_INCREMENT NOT NULL,
    UserAttributeDictionary JSON NOT NULL,
    PRIMARY KEY (AccountID)
);


-- this table is not write-heavy:
CREATE TABLE berytus_account_def(
    AccountVersion INT UNSIGNED AUTO_INCREMENT NOT NULl,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (AccountVersion)
);

-- this table is not write-heavy:
CREATE TABLE berytus_account_def_category(
    AccountCategory VARCHAR(256) PRIMARY KEY NOT NULL,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL
);

/*
 * Given a category, return "available" versions that we can
 * create accounts for. If a user tries to create an account with a
 * (category, version) tuple that does not exist in this table,
 * we reject it.
 *
 * Moreover, this would enable several categories to share
 * account versions. E.g., an account category (e.g. Custommer)
 * can choose from one of many account field structures
 * (e.g. username+password or username+key).
 */
CREATE TABLE berytus_account_def_category_version(
    AccountCategory VARCHAR(256) NOT NULL,
    AccountVersion INT UNSIGNED NOT NULL,
    Description VARCHAR(1024) DEFAULT NULL,
    AllowNewRegistrations TINYINT(1) DEFAULT 1,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (AccountCategory, AccountVersion),
    CONSTRAINT fk_badcv_AccountCategory
        FOREIGN KEY (AccountCategory) REFERENCES berytus_account_def_category(AccountCategory),
    CONSTRAINT fk_badcv_AccountVersion
        FOREIGN KEY (AccountVersion) REFERENCES berytus_account_def(AccountVersion)
);

-- this table is not write-heavy:
CREATE TABLE berytus_account_def_field(
    AccountVersion INT UNSIGNED NOT NULL,
    FieldID VARCHAR(256) NOT NULl,
    FieldType ENUM('Identity', 'ForeignIdentity', 'Password', 'SecurePassword', 'ConsumablePassword', 'Key', 'SharedKey') NOT NULL,
    FieldOptions JSON DEFAULT NULL,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (AccountVersion, FieldID),
    CONSTRAINT fk_badf_AccountVersion
        FOREIGN KEY (AccountVersion) REFERENCES berytus_account_def(AccountVersion)
);

-- This table holds the primary identity fields of an account structure.
-- That is, an array of field ids, e.g., ['accountId', 'name'], is sufficient
-- to retrieve the account id by selecting the maching records in
-- `berytus_account_field`. If all the returned records for (version, fieldIds)
-- correspond to a single account id, we have found our account.
-- this table is not write-heavy:
CREATE TABLE berytus_account_def_key_field_id(
    AccountVersion INT UNSIGNED NOT NULL,
    FieldID VARCHAR(256) NOT NULl,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (AccountVersion, FieldID),
    CONSTRAINT fk_badkfi_AccountVersionFieldID
        FOREIGN KEY (AccountVersion, FieldID) REFERENCES berytus_account_def_field(AccountVersion, FieldID)
);

-- Make sure that each record stored in berytus_account_def_key_field_id corresponds
-- to an IdentityField or ForeginIdentityField. It is not a write-heavy table,
-- so triggers are ok.
DELIMITER $$
CREATE TRIGGER berytus_account_def_key_field_id_chk BEFORE INSERT ON berytus_account_def_key_field_id
FOR EACH ROW
BEGIN
    DECLARE t ENUM('Identity', 'ForeignIdentity', 'Password', 'SecurePassword', 'ConsumablePassword', 'Key', 'SharedKey');
    SET t = (SELECT FieldType FROM berytus_account_def_field
            WHERE   AccountVersion = NEW.AccountVersion
            AND     FieldID = NEW.FieldID);
    IF t != 'Identity' AND t != 'ForeignIdentity' THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Berytus Error: The inserted account key field must correspond to an Identity or Foreign Identity Field. Got otherwise.';
    END IF;
END$$
DELIMITER ;

CREATE TABLE berytus_account_field(
    AccountID BIGINT(21) UNSIGNED NOT NULL,
    AccountVersion INT UNSIGNED NOT NULL,
    FieldID VARCHAR(256) NOT NULL,
    FieldValue JSON NOT NULL, -- can contain a password hash, private key, public key, email, etc.
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (AccountID, AccountVersion, FieldID),
    CONSTRAINT fk_baf_AccountID
        FOREIGN KEY (AccountID) REFERENCES berytus_account(AccountID),
    CONSTRAINT fk_baf_AccountVersionFieldID
        FOREIGN KEY (AccountVersion, FieldID)
            REFERENCES berytus_account_def_field(AccountVersion, FieldID)
);


CREATE TABLE berytus_account_def_auth_challenge(
    AccountVersion INT UNSIGNED NOT NULL,
    ChallengeID VARCHAR(256) NOT NULL, -- specified by the admin
    ChallengeType ENUM('DigitalSignature', 'Password', 'SecureRemotePassword', 'ForeignIdentityOtp') NOT NULL,
    ChallengeParameters JSON DEFAULT NULL,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (AccountVersion, ChallengeID)
);

CREATE TABLE berytus_account_auth_session(
    SessionID BIGINT(21) UNSIGNED AUTO_INCREMENT NOT NULL,
    AccountID BIGINT(21) UNSIGNED NOT NULL,
    AccountVersion INT UNSIGNED NOT NULL,
    Outcome ENUM('Pending', 'Aborted', 'Succeeded') DEFAULT "Pending" NOT NULL,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY (SessionID),
    CONSTRAINT fk_baas_AccountID
        FOREIGN KEY (AccountID) REFERENCES berytus_account(AccountID),
    CONSTRAINT fk_baas_AccountVersion
        FOREIGN KEY (AccountVersion) REFERENCES berytus_account_def(AccountVersion)
);

CREATE TABLE berytus_account_auth_challenge(
    SessionID BIGINT(21) UNSIGNED AUTO_INCREMENT NOT NULL,
    ChallengeID VARCHAR(256) NOT NULL,
    Outcome ENUM('Pending', 'Aborted', 'Succeeded') DEFAULT "Pending" NOT NULL,
    PRIMARY KEY (SessionID, ChallengeID),
    CONSTRAINT fk_baac_SessionID
        FOREIGN KEY (SessionID) REFERENCES berytus_account_auth_session(SessionID)
);

CREATE TABLE berytus_account_auth_challenge_message(
    SessionID BIGINT(21) UNSIGNED NOT NULL,
    ChallengeID VARCHAR(256) NOT NULL,
    MessageName VARCHAR(256) NOT NULL,
    Request JSON DEFAULT NULL,
    Expected JSON DEFAULT NULL, -- E.g., expected OTP code or password hash.
    Response JSON DEFAULT NULL, -- E.g., the relayed OTP code from the manager.
    StatusMsg VARCHAR(256) DEFAULT NULL, -- "Ok" | "Error:*"
    PRIMARY KEY (SessionID, ChallengeID, MessageName),
    CONSTRAINT fk_baacm_SessionID
        FOREIGN KEY (SessionID) REFERENCES berytus_account_auth_session(SessionID)
);

/*
MariaDB [(..)]> SELECT * FROM berytus_account_field_dictionary;
+----------------+-----------+------------------------------------------------------------------------------------+
| AccountVersion | AccountID | FieldList                                                                          |
+----------------+-----------+------------------------------------------------------------------------------------+
|              1 |         1 | {"password":"passbob","username":"bob123"}                                         |
|              2 |         1 | {"securePassword":"securePassBob","username":"bob123"}                             |
|              1 |         2 | {"password":"passjohn","username":"john123"}                                       |
|              2 |         2 | {"securePassword":"securePassJohn","username":"john123"}                           |
|           2000 |        21 | {"partyId":"jerry-and-sons","securePassword":"securePassJerry","username":"jerry"} |
|           2000 |        22 | {"partyId":"jerry-and-sons","securePassword":"securePassCaleb","username":"caleb"} |
|           2000 |        23 | {"partyId":"marks-and-sons","securePassword":"securePassMark","username":"mark"}   |
|           2000 |        24 | {"partyId":"mark-and-sons","securePassword":"securePassMatty","username":"matty"}  |
+----------------+-----------+------------------------------------------------------------------------------------+
*/
CREATE VIEW berytus_account_field_dictionary AS
SELECT  AccountVersion, AccountID,
        CONCAT('{',
        GROUP_CONCAT(CONCAT('"', FieldID, '"', ':', FieldValue)
                    ORDER BY FieldID ASC SEPARATOR ','),
        '}') AS FieldList
FROM berytus_account_field
GROUP BY CONCAT(AccountVersion, '-', AccountID)
ORDER BY AccountID ASC;

/*
MariaDB [(..)]> SELECT * FROM berytus_account_key_field_dictionary;
+----------------+--------+-------------------------------------------------+
| AccountVersion | AccountID | FieldList                                       |
+----------------+-----------+-------------------------------------------------+
|              1 |         1 | {"username":"bob123"}                           |
|              2 |         1 | {"username":"bob123"}                           |
|              1 |         2 | {"username":"john123"}                          |
|              2 |         2 | {"username":"john123"}                          |
|           2000 |        21 | {"partyId":"jerry-and-sons","username":"jerry"} |
|           2000 |        22 | {"partyId":"jerry-and-sons","username":"caleb"} |
|           2000 |        23 | {"partyId":"marks-and-sons","username":"mark"}  |
|           2000 |        24 | {"partyId":"mark-and-sons","username":"matty"}  |
+----------------+-----------+-------------------------------------------------+
*/
CREATE VIEW berytus_account_key_field_dictionary AS
SELECT  auf.AccountVersion, auf.AccountID,
        CONCAT('{',
        GROUP_CONCAT(CONCAT('"', auf.FieldID, '"', ':', auf.FieldValue)
                    ORDER BY auf.FieldID ASC SEPARATOR ','),
        '}') AS FieldList
FROM berytus_account_field AS auf
JOIN berytus_account_def_key_field_id AS akf
    ON akf.AccountVersion = auf.AccountVersion
    AND akf.FieldID = auf.FieldID
GROUP BY CONCAT(auf.AccountVersion, '-', auf.AccountID)
ORDER BY auf.AccountID ASC;
