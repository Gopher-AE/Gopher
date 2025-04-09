#include "../include/mongodb_utils.h"
#include <iostream>

MongoDBUtils::MongoDBUtils(const std::string& uri, 
                          const std::string& db_name,
                          const std::string& collection_name) {
    try {

        instance = std::make_unique<mongocxx::instance>();

        client = std::make_unique<mongocxx::client>(mongocxx::uri{uri});
        
        db = (*client)[db_name];
        graph_collection = db[collection_name];
        pattern_collection = db["patterns"];
    } catch (const std::exception& e) {
        std::cerr << "MongoDB connection error: " << e.what() << std::endl;
        throw;
    }
}

MongoDBUtils::~MongoDBUtils() {

}

bool MongoDBUtils::storeGraph(const std::string& graph_name,
                            const std::vector<std::pair<int, int>>& edges,
                            const std::vector<int>& vertices) {
    try {
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::array;
        using bsoncxx::builder::stream::finalize;

        auto builder = document{};
        builder << "name" << graph_name
                << "vertices" << [&vertices](array context) {
                    for (const auto& v : vertices) {
                        context << v;
                    }
                }
                << "edges" << [&edges](array context) {
                    for (const auto& e : edges) {
                        context << [&e](document sub_context) {
                            sub_context << "from" << e.first
                                      << "to" << e.second;
                        };
                    }
                };

        auto result = graph_collection.insert_one(builder.view());
        return result->result().inserted_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "Error storing graph: " << e.what() << std::endl;
        return false;
    }
}

bool MongoDBUtils::storePattern(const std::string& pattern_name,
                              const std::vector<std::pair<int, int>>& edges,
                              const std::vector<int>& vertices,
                              const std::string& pattern_type) {
    try {
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::array;
        using bsoncxx::builder::stream::finalize;

        auto builder = document{};
        builder << "name" << pattern_name
                << "type" << pattern_type
                << "vertices" << [&vertices](array context) {
                    for (const auto& v : vertices) {
                        context << v;
                    }
                }
                << "edges" << [&edges](array context) {
                    for (const auto& e : edges) {
                        context << [&e](document sub_context) {
                            sub_context << "from" << e.first
                                      << "to" << e.second;
                        };
                    }
                };

        auto result = pattern_collection.insert_one(builder.view());
        return result->result().inserted_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "Error storing pattern: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::pair<int, int>> MongoDBUtils::getGraphEdges(const std::string& graph_name) {
    std::vector<std::pair<int, int>> edges;
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << graph_name
            << bsoncxx::builder::stream::finalize;

        auto result = graph_collection.find_one(filter.view());
        if (result) {
            auto doc = result->view();
            auto edges_array = doc["edges"].get_array().value;
            for (const auto& edge : edges_array) {
                auto edge_doc = edge.get_document().value;
                edges.emplace_back(
                    edge_doc["from"].get_int32().value,
                    edge_doc["to"].get_int32().value
                );
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting graph edges: " << e.what() << std::endl;
    }
    return edges;
}

std::vector<int> MongoDBUtils::getGraphVertices(const std::string& graph_name) {
    std::vector<int> vertices;
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << graph_name
            << bsoncxx::builder::stream::finalize;

        auto result = graph_collection.find_one(filter.view());
        if (result) {
            auto doc = result->view();
            auto vertices_array = doc["vertices"].get_array().value;
            for (const auto& vertex : vertices_array) {
                vertices.push_back(vertex.get_int32().value);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting graph vertices: " << e.what() << std::endl;
    }
    return vertices;
}

std::vector<std::pair<int, int>> MongoDBUtils::getPatternEdges(const std::string& pattern_name) {
    std::vector<std::pair<int, int>> edges;
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << pattern_name
            << bsoncxx::builder::stream::finalize;

        auto result = pattern_collection.find_one(filter.view());
        if (result) {
            auto doc = result->view();
            auto edges_array = doc["edges"].get_array().value;
            for (const auto& edge : edges_array) {
                auto edge_doc = edge.get_document().value;
                edges.emplace_back(
                    edge_doc["from"].get_int32().value,
                    edge_doc["to"].get_int32().value
                );
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting pattern edges: " << e.what() << std::endl;
    }
    return edges;
}

std::vector<int> MongoDBUtils::getPatternVertices(const std::string& pattern_name) {
    std::vector<int> vertices;
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << pattern_name
            << bsoncxx::builder::stream::finalize;

        auto result = pattern_collection.find_one(filter.view());
        if (result) {
            auto doc = result->view();
            auto vertices_array = doc["vertices"].get_array().value;
            for (const auto& vertex : vertices_array) {
                vertices.push_back(vertex.get_int32().value);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting pattern vertices: " << e.what() << std::endl;
    }
    return vertices;
}

std::string MongoDBUtils::getPatternType(const std::string& pattern_name) {
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << pattern_name
            << bsoncxx::builder::stream::finalize;

        auto result = pattern_collection.find_one(filter.view());
        if (result) {
            auto doc = result->view();
            return doc["type"].get_utf8().value.to_string();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting pattern type: " << e.what() << std::endl;
    }
    return "";
}

bool MongoDBUtils::deleteGraph(const std::string& graph_name) {
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << graph_name
            << bsoncxx::builder::stream::finalize;

        auto result = graph_collection.delete_one(filter.view());
        return result->result().deleted_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting graph: " << e.what() << std::endl;
        return false;
    }
}

bool MongoDBUtils::deletePattern(const std::string& pattern_name) {
    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << pattern_name
            << bsoncxx::builder::stream::finalize;

        auto result = pattern_collection.delete_one(filter.view());
        return result->result().deleted_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting pattern: " << e.what() << std::endl;
        return false;
    }
} 