#include "../include/performance_analyzer.h"
#include <algorithm>
#include <numeric>
#include <thread>
#include <chrono>

PerformanceAnalyzer::PerformanceAnalyzer(const DAG& dag, const MonitoringConfig& config)
    : dag_(dag), config_(config), is_monitoring_(false) {
}

void PerformanceAnalyzer::startMonitoring() {
    if (!is_monitoring_) {
        is_monitoring_ = true;
        start_time_ = std::chrono::steady_clock::now();
        metrics_history_.clear();
        event_timestamps_.clear();
        vertex_metrics_.clear();
    }
}

void PerformanceAnalyzer::stopMonitoring() {
    if (is_monitoring_) {
        is_monitoring_ = false;
        markEventTimestamp("monitoring_end");
    }
}

void PerformanceAnalyzer::pauseMonitoring() {
    if (is_monitoring_) {
        markEventTimestamp("pause");
        is_monitoring_ = false;
    }
}

void PerformanceAnalyzer::resumeMonitoring() {
    if (!is_monitoring_) {
        markEventTimestamp("resume");
        is_monitoring_ = true;
    }
}

void PerformanceAnalyzer::recordMetric(const std::string& metric_name, double value) {
    if (!is_monitoring_) return;
    
    updateMetrics();
    if (!metrics_history_.empty()) {
        metrics_history_.back().custom_metrics[metric_name] = value;
    }
}

void PerformanceAnalyzer::recordVertexExecution(int vertex_id, const PerformanceMetrics& metrics) {
    if (!is_monitoring_) return;
    
    vertex_metrics_[vertex_id].push_back(metrics);
    updateMetrics();
}

void PerformanceAnalyzer::markEventTimestamp(const std::string& event_name) {
    event_timestamps_[event_name] = std::chrono::steady_clock::now();
}

PerformanceAnalyzer::PerformanceReport PerformanceAnalyzer::generateReport() const {
    PerformanceReport report;
    
    report.time_series_data = metrics_history_;
    
    for (const auto& metrics : metrics_history_) {
        for (const auto& [metric_name, value] : metrics.custom_metrics) {
            report.aggregated_metrics[metric_name] += value;
        }
        report.aggregated_metrics["total_cpu_usage"] += metrics.cpu_usage;
        report.aggregated_metrics["total_memory_usage"] += metrics.memory_usage;
        report.aggregated_metrics["max_active_threads"] = 
            std::max(report.aggregated_metrics["max_active_threads"], 
                    static_cast<double>(metrics.active_threads));
    }
    
    size_t n = metrics_history_.size();
    if (n > 0) {
        for (auto& [metric_name, value] : report.aggregated_metrics) {
            if (metric_name != "max_active_threads") {
                value /= n;
            }
        }
    }
    
    report.bottlenecks = analyzeBottlenecks();
    
    auto advice = generateOptimizationAdvice();
    for (const auto& suggestion : advice) {
        report.optimization_suggestions.push_back(suggestion.description);
    }
    
    for (const auto& [vertex_id, metrics] : vertex_metrics_) {
        std::vector<double> execution_times;
        for (const auto& m : metrics) {
            execution_times.push_back(m.execution_time);
        }
        report.vertex_metrics[vertex_id] = execution_times;
    }
    
    return report;
}

std::vector<std::string> PerformanceAnalyzer::analyzeBottlenecks() const {
    std::vector<std::string> bottlenecks;
    
    double avg_cpu_usage = 0;
    for (const auto& metrics : metrics_history_) {
        avg_cpu_usage += metrics.cpu_usage;
    }
    avg_cpu_usage /= metrics_history_.size();
    
    if (avg_cpu_usage > 90) {
        bottlenecks.push_back("High CPU utilization (>90%)");
    }
    
    double peak_memory = 0;
    for (const auto& metrics : metrics_history_) {
        peak_memory = std::max(peak_memory, metrics.memory_usage);
    }
    
    if (peak_memory > 90) {
        bottlenecks.push_back("High memory utilization (>90%)");
    }
    
    int max_threads = 0;
    for (const auto& metrics : metrics_history_) {
        max_threads = std::max(max_threads, metrics.active_threads);
    }
    
    if (max_threads > std::thread::hardware_concurrency()) {
        bottlenecks.push_back("Thread oversubscription detected");
    }
    
    for (const auto& [vertex_id, metrics] : vertex_metrics_) {
        double avg_time = 0;
        for (const auto& m : metrics) {
            avg_time += m.execution_time;
        }
        avg_time /= metrics.size();
        
        if (avg_time > 1.0) {  
            bottlenecks.push_back("Vertex " + std::to_string(vertex_id) + 
                                " has high execution time");
        }
    }
    
    return bottlenecks;
}

std::map<int, double> PerformanceAnalyzer::analyzeVertexPerformance() const {
    std::map<int, double> performance_scores;
    
    for (const auto& [vertex_id, metrics] : vertex_metrics_) {
        double avg_execution_time = 0;
        double avg_cpu_usage = 0;
        double avg_memory_usage = 0;
        
        for (const auto& m : metrics) {
            avg_execution_time += m.execution_time;
            avg_cpu_usage += m.cpu_usage;
            avg_memory_usage += m.memory_usage;
        }
        
        size_t n = metrics.size();
        if (n > 0) {
            avg_execution_time /= n;
            avg_cpu_usage /= n;
            avg_memory_usage /= n;
            
            performance_scores[vertex_id] = 
                avg_execution_time * 0.5 +
                avg_cpu_usage * 0.3 +
                avg_memory_usage * 0.2;
        }
    }
    
    return performance_scores;
}

std::vector<PerformanceAnalyzer::OptimizationAdvice> 
PerformanceAnalyzer::generateOptimizationAdvice() const {
    std::vector<OptimizationAdvice> advice;
    
    auto patterns = identifyPerformancePatterns();
    for (const auto& pattern : patterns) {
        OptimizationAdvice suggestion;
        suggestion.category = "Performance Pattern";
        suggestion.description = pattern;
        suggestion.potential_improvement = 0.2;  // 假设20%的改进潜力
        suggestion.action_items = {"Review algorithm efficiency",
                                 "Consider caching results",
                                 "Optimize data structures"};
        advice.push_back(suggestion);
    }
    

    auto resource_util = analyzeResourceUtilization();
    
    if (!resource_util.cpu_usage_timeline.empty()) {
        double avg_cpu = std::accumulate(resource_util.cpu_usage_timeline.begin(),
                                       resource_util.cpu_usage_timeline.end(),
                                       0.0) / resource_util.cpu_usage_timeline.size();
        if (avg_cpu > 80) {
            OptimizationAdvice cpu_advice;
            cpu_advice.category = "CPU Usage";
            cpu_advice.description = "High CPU utilization detected";
            cpu_advice.potential_improvement = 0.3;
            cpu_advice.action_items = {"Optimize compute-intensive operations",
                                     "Consider parallel processing",
                                     "Review algorithm complexity"};
            advice.push_back(cpu_advice);
        }
    }

    if (!resource_util.memory_usage_timeline.empty()) {
        double peak_memory = *std::max_element(resource_util.memory_usage_timeline.begin(),
                                             resource_util.memory_usage_timeline.end());
        if (peak_memory > 90) {
            OptimizationAdvice memory_advice;
            memory_advice.category = "Memory Usage";
            memory_advice.description = "High memory usage detected";
            memory_advice.potential_improvement = 0.25;
            memory_advice.action_items = {"Implement memory pooling",
                                        "Review data structure sizes",
                                        "Consider memory-efficient algorithms"};
            advice.push_back(memory_advice);
        }
    }
    
    auto concurrent_execs = findConcurrentExecutions();
    if (!concurrent_execs.empty()) {
        OptimizationAdvice concurrency_advice;
        concurrency_advice.category = "Concurrency";
        concurrency_advice.description = "Potential for improved parallelization";
        concurrency_advice.potential_improvement = 0.15;
        concurrency_advice.action_items = {"Review thread synchronization",
                                         "Optimize task scheduling",
                                         "Consider load balancing"};
        advice.push_back(concurrency_advice);
    }
    
    return advice;
}

PerformanceAnalyzer::ResourceUtilization 
PerformanceAnalyzer::analyzeResourceUtilization() const {
    ResourceUtilization utilization;
    
    for (const auto& metrics : metrics_history_) {
        utilization.cpu_usage_timeline.push_back(metrics.cpu_usage);
        utilization.memory_usage_timeline.push_back(metrics.memory_usage);
        utilization.thread_count_timeline.push_back(metrics.active_threads);
        
        for (const auto& [metric_name, value] : metrics.custom_metrics) {
            utilization.custom_metric_timelines[metric_name].push_back(value);
        }
    }
    
    return utilization;
}

void PerformanceAnalyzer::updateMetrics() {
    if (!is_monitoring_) return;
    
    PerformanceMetrics current_metrics;
    auto current_time = std::chrono::steady_clock::now();
    
    // 计算执行时间
    current_metrics.execution_time = 
        std::chrono::duration<double>(current_time - start_time_).count();
    
    current_metrics.cpu_usage = 0.0;  
    current_metrics.memory_usage = 0.0;  
    
    current_metrics.active_threads = 
        static_cast<int>(std::thread::hardware_concurrency()); 
    
    metrics_history_.push_back(current_metrics);
    
    cleanupStaleData();
}

double PerformanceAnalyzer::calculateAggregateMetric(const std::string& metric_name) const {
    std::vector<double> values;
    for (const auto& metrics : metrics_history_) {
        auto it = metrics.custom_metrics.find(metric_name);
        if (it != metrics.custom_metrics.end()) {
            values.push_back(it->second);
        }
    }
    
    if (values.empty()) return 0.0;
    
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

std::vector<std::string> PerformanceAnalyzer::identifyPerformancePatterns() const {
    std::vector<std::string> patterns;
    
    if (!metrics_history_.empty()) {
        std::vector<double> execution_times;
        for (const auto& metrics : metrics_history_) {
            execution_times.push_back(metrics.execution_time);
        }
        
        bool increasing = true;
        for (size_t i = 1; i < execution_times.size(); ++i) {
            if (execution_times[i] <= execution_times[i-1]) {
                increasing = false;
                break;
            }
        }
        
        if (increasing) {
            patterns.push_back("Execution time shows increasing trend");
        }
        
        std::vector<double> differences;
        for (size_t i = 1; i < execution_times.size(); ++i) {
            differences.push_back(execution_times[i] - execution_times[i-1]);
        }
        
        bool periodic = true;
        if (!differences.empty()) {
            double avg_diff = differences[0];
            for (size_t i = 1; i < differences.size(); ++i) {
                if (std::abs(differences[i] - avg_diff) > avg_diff * 0.1) {
                    periodic = false;
                    break;
                }
            }
            
            if (periodic) {
                patterns.push_back("Periodic execution pattern detected");
            }
        }
    }
    
    bool resource_spikes = false;
    for (const auto& metrics : metrics_history_) {
        if (metrics.cpu_usage > 90 || metrics.memory_usage > 90) {
            resource_spikes = true;
            break;
        }
    }
    
    if (resource_spikes) {
        patterns.push_back("Resource usage spikes detected");
    }
    
    return patterns;
}

void PerformanceAnalyzer::cleanupStaleData() {
    auto current_time = std::chrono::steady_clock::now();
    auto cutoff_time = current_time - std::chrono::hours(24); 
    
    for (auto it = event_timestamps_.begin(); it != event_timestamps_.end();) {
        if (it->second < cutoff_time) {
            it = event_timestamps_.erase(it);
        } else {
            ++it;
        }
    }
    
    while (!metrics_history_.empty() && 
           start_time_ + std::chrono::seconds(
               static_cast<int>(metrics_history_[0].execution_time)) < cutoff_time) {
        metrics_history_.erase(metrics_history_.begin());
    }
}

bool PerformanceAnalyzer::isResourceOverutilized(const std::string& resource_type) const {
    if (resource_type == "CPU") {
        for (const auto& metrics : metrics_history_) {
            if (metrics.cpu_usage > 90) return true;
        }
    } else if (resource_type == "Memory") {
        for (const auto& metrics : metrics_history_) {
            if (metrics.memory_usage > 90) return true;
        }
    } else if (resource_type == "Threads") {
        for (const auto& metrics : metrics_history_) {
            if (metrics.active_threads > static_cast<int>(std::thread::hardware_concurrency())) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::pair<int, int>> PerformanceAnalyzer::findConcurrentExecutions() const {
    std::vector<std::pair<int, int>> concurrent_pairs;
    
    for (const auto& [vertex1, metrics1] : vertex_metrics_) {
        for (const auto& [vertex2, metrics2] : vertex_metrics_) {
            if (vertex1 >= vertex2) continue;
            
            for (const auto& m1 : metrics1) {
                for (const auto& m2 : metrics2) {
                    if (std::abs(m1.execution_time - m2.execution_time) < 0.1) {
                        concurrent_pairs.push_back({vertex1, vertex2});
                        goto next_pair;
                    }
                }
            }
            next_pair:;
        }
    }
    
    return concurrent_pairs;
} 