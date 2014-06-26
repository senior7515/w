#pragma once
#ifndef PERSISTENCE_POSTGRESQL_HPP_INCLUDED
#define PERSISTENCE_POSTGRESQL_HPP_INCLUDED

#include <persistence/connection.hpp>
#include <persistence/adapter.hpp>
#include <persistence/sql_type_mapper.hpp>

#include <wayward/support/error.hpp>

namespace persistence {
  struct PostgreSQLError : wayward::Error {
    PostgreSQLError(const std::string& message) : wayward::Error{message} {}
  };

  struct PostgreSQLAdapter : IAdapter {
    std::unique_ptr<IConnection> connect(std::string connection_string) const final;
  };

  struct PostgreSQLTypeMapper : ISQLTypeMapper {
    wayward::CloningPtr<ast::SingleValue> literal_for_value(AnyConstRef value) final;
  };

  using wayward::ILogger;

  struct PostgreSQLConnection : IConnection {
    virtual ~PostgreSQLConnection();

    // Info
    std::string database() const final;
    std::string user() const final;
    std::string host() const final;
    std::shared_ptr<ILogger> logger() const final;
    void set_logger(std::shared_ptr<ILogger> l) final;

    // Querying
    std::string sanitize(std::string sql_fragment) final;
    std::string to_sql(const ast::IQuery& query) final;
    std::string to_sql(const ast::IQuery& query, const relational_algebra::IResolveSymbolicRelation&) final;
    std::unique_ptr<IResultSet> execute(const ast::IQuery& query) final;
    std::unique_ptr<IResultSet> execute(std::string sql) final;
    std::unique_ptr<IResultSet> execute(const ast::IQuery& query, const relational_algebra::IResolveSymbolicRelation&) final;
    wayward::CloningPtr<ast::SingleValue> literal_for_value(AnyConstRef data) final;

    static std::unique_ptr<PostgreSQLConnection>
    connect(std::string connection_string, std::string* out_error = nullptr);
  private:
    PostgreSQLConnection();
    struct Private;
    std::unique_ptr<Private> priv;
  };
}

#endif // PERSISTENCE_POSTGRESQL_HPP_INCLUDED
