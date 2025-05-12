#ifndef EXECUTION_PLANNER_H
#define EXECUTION_PLANNER_H

#include "dag.h"
#include "graph_analysis.h"
#include <vector>
#include <memory>
#include <queue>
#include <functional>

class ExecutionPlanner {
public:
    struct ResourceConstraints {
        int max_parallel_tasks;
        int max_memory_mb;
        int max_cpu_cores;
        double max_execution_time;
    };

    struct TaskNode {
        int vertex_id;
        std::vector<int> dependencies;
        double estimated_cost;
        int required_memory;
        int required_cores;
        std::vector<int> alternative_paths;
    };

    struct ExecutionPlan {
        std::vector<std::vector<TaskNode>> execution_stages;
        double total_estimated_time;
        double peak_memory_usage;
        int max_parallel_tasks;
        std::vector<std::string> optimization_notes;
    };

    ExecutionPlanner(const DAG& dag, const ResourceConstraints& constraints);
    ~ExecutionPlanner() = default;

    ExecutionPlan generatePlan();
    ExecutionPlan generateOptimizedPlan();
    
    void updateResourceConstraints(const ResourceConstraints& new_constraints);
    bool validateResourceRequirements(const ExecutionPlan& plan) const;
    
    ExecutionPlan optimizeForTime();
    ExecutionPlan optimizeForMemory();
    ExecutionPlan optimizeForParallelism();
    
    bool validateDependencies(const ExecutionPlan& plan) const;
    std::vector<TaskNode> getReadyTasks() const;
    
    struct PlanAnalysis {
        double efficiency_score;
        std::vector<std::string> bottlenecks;
        std::vector<std::string> improvement_suggestions;
        std::map<std::string, double> resource_utilization;
    };
    
    PlanAnalysis analyzePlan(const ExecutionPlan& plan) const;

private:
    const DAG& dag_;
    ResourceConstraints constraints_;
    std::unique_ptr<GraphAnalysis> graph_analysis_;
    std::vector<TaskNode> task_nodes_;
    
    void initializeTaskNodes();
    std::vector<std::vector<int>> createExecutionLevels() const;
    double estimateTaskCost(const TaskNode& task) const;
    bool canScheduleTogether(const TaskNode& task1, const TaskNode& task2) const;
    void optimizeTaskOrder(std::vector<TaskNode>& tasks);
    std::vector<TaskNode> findCriticalPath() const;
    void balanceResources(ExecutionPlan& plan);
};

#endif // EXECUTION_PLANNER_H 