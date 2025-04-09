#ifndef MONGODB_UTILS_H
#define MONGODB_UTILS_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <string>
#include <vector>
#include <memory>

class MongoDBUtils {
public:

    MongoDBUtils(const std::string& uri = "mongodb://localhost:27017", 
                const std::string& db_name = "graph_db",
                const std::string& collection_name = "graphs");
    ~MongoDBUtils();
    bool storeGraph(const std::string& graph_name, 
                   const std::vector<std::pair<int, int>>& edges,
                   const std::vector<int>& vertices);

    bool storePattern(const std::string& pattern_name,
                     const std::vector<std::pair<int, int>>& edges,
                     const std::vector<int>& vertices,
                     const std::string& pattern_type);

    std::vector<std::pair<int, int>> getGraphEdges(const std::string& graph_name);
    std::vector<int> getGraphVertices(const std::string& graph_name);
    std::vector<std::pair<int, int>> getPatternEdges(const std::string& pattern_name);
    std::vector<int> getPatternVertices(const std::string& pattern_name);
    std::string getPatternType(const std::string& pattern_name);
    bool deleteGraph(const std::string& graph_name);
    bool deletePattern(const std::string& pattern_name);

private:
    std::unique_ptr<mongocxx::instance> instance;
    std::unique_ptr<mongocxx::client> client;
    mongocxx::database db;
    mongocxx::collection graph_collection;
    mongocxx::collection pattern_collection;
};

#endif // MONGODB_UTILS_H 