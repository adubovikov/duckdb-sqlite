#include "duckdb.hpp"

#include "sqlite3.h"
#include "sqlite_utils.hpp"
#include "sqlite_storage.hpp"
#include "storage/sqlite_catalog.hpp"
#include "storage/sqlite_transaction_manager.hpp"
#include "duckdb/parser/parsed_data/attach_info.hpp"
#include "duckdb/common/exception/binder_exception.hpp"
#include "duckdb/transaction/transaction_manager.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"

namespace duckdb {

static unique_ptr<Catalog> SQLiteAttach(optional_ptr<StorageExtensionInfo> storage_info, ClientContext &context,
                                        AttachedDatabase &db, const string &name, AttachInfo &info,
                                        AttachOptions &attach_options) {
	SQLiteOpenOptions options;
	options.access_mode = attach_options.access_mode;
	for (auto &entry : attach_options.options) {
		if (StringUtil::CIEquals(entry.first, "busy_timeout")) {
			options.busy_timeout = entry.second.GetValue<uint64_t>();
		} else if (StringUtil::CIEquals(entry.first, "journal_mode")) {
			options.journal_mode = SQLiteOpenOptions::ParseJournalMode(entry.second.ToString());
		} else if (StringUtil::CIEquals(entry.first, "synchronous")) {
			options.synchronous = SQLiteOpenOptions::ParseSynchronous(entry.second.ToString());
		} else if (StringUtil::CIEquals(entry.first, "wal_autocheckpoint")) {
			auto pages = entry.second.DefaultCastAs(LogicalType::BIGINT).GetValue<int64_t>();
			if (pages < 0) {
				throw BinderException("Invalid value \"%s\" for SQLite option wal_autocheckpoint: expected a "
				                      "number of pages >= 0",
				                      entry.second.ToString());
			}
			options.wal_autocheckpoint = pages;
		} else {
			throw NotImplementedException("Unsupported parameter for SQLite Attach: %s", entry.first);
		}
	}
	Value keep_open;
	if (context.TryGetCurrentSetting("sqlite_keep_database_open", keep_open) && !keep_open.IsNull()) {
		options.keep_database_open = BooleanValue::Get(keep_open);
	}
	return make_uniq<SQLiteCatalog>(db, info.path, std::move(options));
}

static unique_ptr<TransactionManager> SQLiteCreateTransactionManager(optional_ptr<StorageExtensionInfo> storage_info,
                                                                     AttachedDatabase &db, Catalog &catalog) {
	auto &sqlite_catalog = catalog.Cast<SQLiteCatalog>();
	return make_uniq<SQLiteTransactionManager>(db, sqlite_catalog);
}

SQLiteStorageExtension::SQLiteStorageExtension() {
	attach = SQLiteAttach;
	create_transaction_manager = SQLiteCreateTransactionManager;
}

} // namespace duckdb
