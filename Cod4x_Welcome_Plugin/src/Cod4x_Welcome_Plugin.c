#include "../libs/pinc.h"
#include "version.h"

#include <string.h>
#include <sqlite3.h>

sqlite3 *db = NULL;

int checkAndInsertPlayer(sqlite3 *db, const char *playerName) {
    int rc;

    const char *sqlCheck = "SELECT COUNT(*) FROM Players WHERE Name = ?;";
    sqlite3_stmt *stmtCheck;

    rc = sqlite3_prepare_v2(db, sqlCheck, -1, &stmtCheck, 0);
    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_text(stmtCheck, 1, playerName, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmtCheck);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmtCheck, 0);
        if (count == 0) {
            const char *sqlInsert = "INSERT INTO Players (Name, IsActive) VALUES (?, 1);";
            sqlite3_stmt *stmtInsert;

            rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmtInsert, 0);
            if (rc != SQLITE_OK) {
            	Plugin_PrintError("Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
                sqlite3_finalize(stmtCheck);
                return rc;

            }

            sqlite3_bind_text(stmtInsert, 1, playerName, -1, SQLITE_STATIC);

            rc = sqlite3_step(stmtInsert);
            if (rc != SQLITE_DONE) {
                Plugin_PrintError("Failed to insert player: %s\n", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmtInsert);
        } else {
            Plugin_Printf("Player '%s' already exists.\n", playerName);
            return 1;
        }
    } else {
    	Plugin_PrintError("Failed to execute check statement: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmtCheck);
    return 0;
}

int deleteAllPlayers(sqlite3 *db) {
    char *errMsg = 0;
    int rc;

    const char *sqlDelete = "DELETE FROM Players;";

    rc = sqlite3_exec(db, sqlDelete, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Failed to delete all players: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;

    } else {
        Plugin_Printf("All players deleted successfully.\n");

    }

    return 0;
}


PCL int OnInit() {
	int rc = sqlite3_open("WelcomeTmp.db", &db);
	char* errMsg = NULL;
	if (rc) {
		Plugin_PrintError("Welcome Plugin: Error occurred on opening database.");
		return 1;

	} else {
		Plugin_Printf("Welocme Plugin: Database opened successfully.");

	}

	const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS Players ("
								"Name TEXT PRIMARY KEY NOT NULL,"
								"IsActive INTEGER NOT NULL"
								");";

	rc = sqlite3_exec(db, sqlCreateTable, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        Plugin_Printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;

    } else {
        Plugin_Printf("Table created successfully\n");

    }
	return 0;

}

PCL void OnClientEnterWorld(client_t* client) {

	if(checkAndInsertPlayer(db, Plugin_GetPlayerName(NUMFORCLIENT(client))) == 0) {
		Plugin_ChatPrintf(-1, "SERVER: %s^7, welcome to the server!", Plugin_GetPlayerName(NUMFORCLIENT(client)));

	}

}

PCL void OnPlayerDC(client_t* client, const char* reason) {
	int rc = 0;
    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "DELETE FROM Players WHERE Name = ?;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, Plugin_GetPlayerName(NUMFORCLIENT(client)), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
    	Plugin_PrintError("Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

}

PCL void OnInfoRequest(pluginInfo_t *info){

    info->handlerVersion.major = 4;
    info->handlerVersion.minor = 0;

    strncpy(info->fullName,"Cod4X Welcome Plugin",sizeof(info->fullName));
    strncpy(info->shortDescription,"Sends a hello message to the player.",sizeof(info->shortDescription));
    strncpy(info->longDescription,"This plugin is used to send a welcome message to the player entering the world. Coded my LM40 ( DevilHunter )",sizeof(info->longDescription));
}

PCL void OnTerminate() {
	deleteAllPlayers(db);
	sqlite3_close(db);

}
