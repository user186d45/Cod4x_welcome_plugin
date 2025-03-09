#include "../libs/pinc.h"
#include "version.h"

#include <string.h>
#include <sqlite3.h>
#include <pthread.h>

sqlite3 *db = NULL;
pthread_mutex_t mutex;

__cdecl int checkAndInsertPlayer(client_t* player);
__cdecl int deleteAllPlayers();


PCL int OnInit() {
	pthread_mutex_lock(&mutex);

	char* errMsg = NULL;

	int rc = sqlite3_open("WelcomeTmp.db", &db);
	if (rc != SQLITE_OK) {
		Plugin_PrintError("Welcome Plugin: Error occurred on opening database.\n");
		pthread_mutex_unlock(&mutex);
		return 1;

	}

	Plugin_Printf("Welocme Plugin: NOTICE: Database opened successfully.\n");

	const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS Players ("
								"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
								"PlayerName TEXT NOT NULL"
								");";

	rc = sqlite3_exec(db, sqlCreateTable, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        Plugin_Printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        pthread_mutex_unlock(&mutex);
        return 1;

    }

    Plugin_Printf("Welcome Plugin: NOTICE: Table created successfully\n");
    pthread_mutex_unlock(&mutex);

	return 0;

}

PCL void OnClientEnterWorld(client_t* client) {

	if(checkAndInsertPlayer(client) == 0) {
		Plugin_ChatPrintf(-1, "^4SERVER: %s^7, welcome to the ^2AMG ^7server!", Plugin_GetPlayerName(NUMFORCLIENT(client)));

	}

}

PCL void OnPlayerDC(client_t* client, const char* reason) {
	pthread_mutex_lock(&mutex);

	int rc = 0;
    const char *sqlDelete = "DELETE FROM Players WHERE PlayerName = ?;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sqlDelete, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Welcome Plugin: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    	pthread_mutex_unlock(&mutex);
        return;
    }

    sqlite3_bind_text(stmt, 1, Plugin_GetPlayerName(NUMFORCLIENT(client)), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
    	Plugin_PrintError("Execution failed: %s\n", sqlite3_errmsg(db));
    	sqlite3_finalize(stmt);
    	pthread_mutex_unlock(&mutex);
    	return;
    }

    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&mutex);

}

PCL void OnInfoRequest(pluginInfo_t *info){

    info->handlerVersion.major = PLUGIN_HANDLER_VERSION_MAJOR;
    info->handlerVersion.minor = PLUGIN_HANDLER_VERSION_MINOR;

    info->pluginVersion.major = Cod4x_Welcome_Plugin_VERSION_MAJOR;
    info->pluginVersion.minor = Cod4x_Welcome_Plugin_VERSION_MINOR;
    strncpy(info->fullName,"Cod4X Welcome Plugin",sizeof(info->fullName));
    strncpy(info->shortDescription,"Sends a hello message to the player.",sizeof(info->shortDescription));
    strncpy(info->longDescription,"This plugin is used to send a welcome message to the player entering the world. Coded my LM40 ( DevilHunter )",sizeof(info->longDescription));
}

PCL void OnTerminate() {
	deleteAllPlayers();
	sqlite3_close(db);

}

__cdecl int checkAndInsertPlayer(client_t* player) {
	pthread_mutex_lock(&mutex);

	int rc;
	const char* sqlCheck = "SELECT COUNT(*) FROM Players WHERE PlayerName = ?;";
    sqlite3_stmt* stmt;

    const char* playerName = Plugin_GetPlayerName(NUMFORCLIENT(player));

    rc = sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Welcome Plugin: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    	pthread_mutex_unlock(&mutex);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, playerName, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
    	Plugin_PrintError("Welcome Plugin: Failed to execute check statement: %s\n", sqlite3_errmsg(db));
    	sqlite3_finalize(stmt);
    	pthread_mutex_unlock(&mutex);
    	return -1;

    }

    int count = sqlite3_column_int(stmt, 0);

    Plugin_Printf("Welcome Plugin: NOTICE: Successfully selected playerName count.\n");

    if(count > 0) {
    	Plugin_Printf("Welcome Plugin: NOTICE: Player '%s^7' already exists.\n", Plugin_GetPlayerName(NUMFORCLIENT(player)));
    	sqlite3_finalize(stmt);
    	pthread_mutex_unlock(&mutex);
    	return 1;

    }

    sqlite3_reset(stmt);
    const char *sqlInsert = "INSERT INTO Players (PlayerName) VALUES (?);";

    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Welcome Plugin: Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
    	sqlite3_finalize(stmt);
    	pthread_mutex_unlock(&mutex);
    	return -1;

    }

    sqlite3_bind_text(stmt, 1, playerName, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
    	Plugin_PrintError("Welcome Plugin: Failed to insert player: %s\n", sqlite3_errmsg(db));
    	sqlite3_finalize(stmt);
    	pthread_mutex_unlock(&mutex);
    	return -1;

    }

    Plugin_Printf("Welcome Plugin: NOTICE: Successfully wrote player name into db.\n");

    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&mutex);

    return 0;
}


__cdecl int deleteAllPlayers() {
	pthread_mutex_lock(&mutex);

    char *errMsg = 0;
    int rc;

    const char *sqlDelete = "DELETE FROM Players;";

    rc = sqlite3_exec(db, sqlDelete, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
    	Plugin_PrintError("Failed to delete all players: %s\n", errMsg);
        sqlite3_free(errMsg);
        pthread_mutex_unlock(&mutex);
        return -1;

    }

    Plugin_Printf("All players deleted successfully.\n");
    pthread_mutex_unlock(&mutex);

    return 0;
}
