/**
*
* Copyright (c) 2015 Hal Martin
* This is free software, licensed under the GNU General Public License v2.
* See /LICENSE for more information.
*
*/

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <libconfig.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  int fd;
  // libconfig variables
  config_t cfg, *cf;
  const char *sqlite_file = NULL;

  // sqlite variables
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;
  int sqlite_db_conn = 0;
  int rc = 0;
  char *sql = NULL;

  // initialize libconfig
  cf = &cfg;
  config_init(cf);
 
  if (!config_read_file(cf,"/etc/config/load2sqlite")) {
    fprintf(stderr, "%s:%d - %s\n",
      config_error_file(cf),
      config_error_line(cf),
      config_error_text(cf));
    config_destroy(cf);
    return(EXIT_FAILURE);
  }

  config_lookup_string(cf,"sqlitedb", &sqlite_file);

  // verify that the file exists
  fd = open(sqlite_file, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    // failed to create the file
    // file didn't exist
    if (errno == EEXIST) {
      // file already exists!
      // open the SQLite file defined in the configuration
      rc = sqlite3_open(sqlite_file, &db);
      if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
      } else {
        fprintf(stderr, "SQLite database opened\n");
      }
      // verify that the file has the required table
      sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='readings'";
      rc = sqlite3_prepare_v2(db,sql,-1,&res,0);
      int step = sqlite3_step(res);
      if (step == SQLITE_ROW) {
        if (sqlite3_column_int(res,0) == 1) {
          // we are good
          fprintf(stderr,"Found readings table\n");
          sqlite_db_conn = 1;
        } else {
          fprintf(stderr,"Database not initalized\n");
        }
      }
      sqlite3_finalize(res);
    }
  } else {
    fprintf(stderr, "Database file %s does not exist\n", sqlite_file);
    // initialize the SQLite database in the file
    // should not fail because we just created the file
    rc = sqlite3_open(sqlite_file, &db);
    // create the table
    sql = "CREATE TABLE readings(timestamp DATETIME, load_1m REAL, load_5m REAL, load_15m REAL)";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "Could not initialize database file %s\n", sqlite_file);
    } else {
      fprintf(stderr, "Initialized database with readings table\n");
      sqlite_db_conn = 1;
    }
  }
  

  if (sqlite_db_conn == 0) {
    fprintf(stderr,"Database connection failed, not saving data to SQLite!\n");
  } else {

    // we have to read load from proc because uclibc doesn't include
    // the getloadavg() function
    double load[3] = {0.0, 0.0, 0.0};
    FILE *LOADAVG;
    int status = -1;
    if ((LOADAVG = fopen("/proc/loadavg","r"))) {
      fscanf(LOADAVG, "%lf %lf %lf", &load[0], &load[1], &load[2]);
      status = 0;
      fclose(LOADAVG);
    }

    // we read the load average successfully from /proc/loadavg
    if (status == 0) {
      // sqlite store
      sql = "INSERT INTO readings(timestamp,load_1m,load_5m,load_15m) VALUES(DateTime('now'),@load1m,@load5m,@load15m)";
      rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
      if (rc == SQLITE_OK) {
        int idx = sqlite3_bind_parameter_index(res,"@load1m");
        sqlite3_bind_double(res,idx,load[0]);
        idx = sqlite3_bind_parameter_index(res,"@load5m");
        sqlite3_bind_double(res,idx,load[1]);
        idx = sqlite3_bind_parameter_index(res,"@load15m");
        sqlite3_bind_double(res,idx,load[2]);
        sqlite3_step(res);
        sqlite3_finalize(res);
      } else {
        fprintf(stderr, "Could not prepare statement: %s\n", sqlite3_errmsg(db));
      }
    }
  }

  return 0;

}

