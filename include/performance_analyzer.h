#ifndef PERFORMANCE_ANALYZER_H
#define PERFORMANCE_ANALYZER_H

#include "dag.h"
#include "execution_planner.h"
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <memory>

class PerformanceAnalyzer {
public:
    struct PerformanceMetrics {
        double execution_time;
        double cpu_usage;
        double memory_usage;
        int active_threads;
        std::map<std::string, double> custom_metrics;
    };

    struct PerformanceReport {
        std::vector<PerformanceMetrics> time_series_data;
        std::map<std::string, double> aggregated_metrics;
        std::vector<std::string> bottlenecks;
        std::vector<std::string> optimization_suggestions;
        std::map<int, std::vector<double>> vertex_metrics;
    };

    struct MonitoringConfig {
        bool enable_cpu_profiling;
        bool enable_memory_profiling;
        bool enable_thread_profiling;
        int sampling_interval_ms;
        std::vector<std::string> custom_metrics;
    };

    PerformanceAnalyzer(const DAG& dag, const MonitoringConfig& config);
    ~PerformanceAnalyzer() = default;
    
    void startMonitoring();
    void stopMonitoring();
    void pauseMonitoring();
    void resumeMonitoring();

    void recordMetric(const std::string& metric_name, double value);
    void recordVertexExecution(int vertex_id, const PerformanceMetrics& metrics);
    void markEventTimestamp(const std::string& event_name);

    PerformanceReport generateReport() const;
    std::vector<std::string> analyzeBottlenecks() const;
    std::map<int, double> analyzeVertexPerformance() const;

    struct OptimizationAdvice {
        std::string category;
        std::string description;
        double potential_improvement;
        std::vector<std::string> action_items;
    };
    
    std::vector<OptimizationAdvice> generateOptimizationAdvice() const;
    
    struct ResourceUtilization {
        std::vector<double> cpu_usage_timeline;
        std::vector<double> memory_usage_timeline;
        std::vector<int> thread_count_timeline;
        std::map<std::string, std::vector<double>> custom_metric_timelines;
    };
    
    ResourceUtilization analyzeResourceUtilization() const;

private:
    const DAG& dag_;
    MonitoringConfig config_;
    bool is_monitoring_;
    std::chrono::steady_clock::time_point start_time_;
    std::vector<PerformanceMetrics> metrics_history_;
    std::map<std::string, std::chrono::steady_clock::time_point> event_timestamps_;
    std::map<int, std::vector<PerformanceMetrics>> vertex_metrics_;

    void updateMetrics();
    double calculateAggregateMetric(const std::string& metric_name) const;
    std::vector<std::string> identifyPerformancePatterns() const;
    void cleanupStaleData();
    bool isResourceOverutilized(const std::string& resource_type) const;
    std::vector<std::pair<int, int>> findConcurrentExecutions() const;
};

#endif // PERFORMANCE_ANALYZER_H 