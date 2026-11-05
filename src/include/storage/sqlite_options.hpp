//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/sqlite_options.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/enums/access_mode.hpp"

namespace duckdb {

struct SQLiteOpenOptions {
	// access mode
	AccessMode access_mode = AccessMode::READ_WRITE;
	// busy time-out in ms
	idx_t busy_timeout = 5000;
	// journal mode (PRAGMA journal_mode), validated against the SQLite keyword list
	string journal_mode;
	// synchronous setting (PRAGMA synchronous), validated against the SQLite keyword list
	string synchronous;
	// keep one handle open for the lifetime of an attached on-disk database (sqlite_keep_database_open)
	bool keep_database_open = false;
	// PRAGMA wal_autocheckpoint (pages) on every handle; -1 = leave SQLite's default (1000). 0 disables
	// automatic checkpoints, for callers that checkpoint the WAL themselves (sqlite3_wal_checkpoint_v2).
	int64_t wal_autocheckpoint = -1;

	//! Validate and normalise a journal_mode value; throws on anything SQLite would not accept
	static string ParseJournalMode(const string &value);
	//! Validate and normalise a synchronous value; throws on anything SQLite would not accept
	static string ParseSynchronous(const string &value);
};

} // namespace duckdb
