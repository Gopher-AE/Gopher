#include "../include/execution_planner.h"
#include <algorithm>
#include <numeric>
#include <cmath>

ExecutionPlanner::ExecutionPlanner(const DAG& dag, const ResourceConstraints& constraints)
    : dag_(dag), constraints_(constraints), graph_analysis_(std::make_unique<GraphAnalysis>(dag)) {
    initializeTaskNodes();
}

void ExecutionPlanner::initializeTaskNodes() {
    int size = dag_.get_size();
    task_nodes_.resize(size);
    
    auto dependency_map = graph_analysis_->getDependencyMap();
    auto performance_metrics = graph_analysis_->estimatePerformance();
    
    for (int i = 0; i < size; ++i) {
        task_nodes_[i].vertex_id = i;
        task_nodes_[i].dependencies.assign(dependency_map[i].begin(), dependency_map[i].end());
        task_nodes_[i].estimated_cost = performance_metrics.estimated_execution_time / size;
        task_nodes_[i].required_memory = 1024;  
        task_nodes_[i].required_cores = 1;      
    }
}

ExecutionPlanner::ExecutionPlan ExecutionPlanner::generatePlan() {
    ExecutionPlan plan;
    auto levels = graph_analysis_->getExecutionLevels();
    
    for (const auto& level : levels) {
        std::vector<TaskNode> stage;
        for (int vertex : level) {
            stage.push_back(task_nodes_[vertex]);
        }
        plan.execution_stages.push_back(stage);
    }
    
    plan.total_estimated_time = 0;
    plan.peak_memory_usage = 0;
    plan.max_parallel_tasks = 0;
    
    for (const auto& stage : plan.execution_stages) {
        double stage_time = 0;
        double stage_memory = 0;
        for (const auto& task : stage) {
            stage_time = std::max(stage_time, task.estimated_cost);
            stage_memory += task.required_memory;
        }
        plan.total_estimated_time += stage_time;
        plan.peak_memory_usage = std::max(plan.peak_memory_usage, stage_memory);
        plan.max_parallel_tasks = std::max(plan.max_parallel_tasks, 
                                         static_cast<int>(stage.size()));
    }
    
    return plan;
}

ExecutionPlanner::ExecutionPlan ExecutionPlanner::generateOptimizedPlan() {
    auto plan = generatePlan();
    
    optimizeTaskOrder(plan.execution_stages[0]); 
    balanceResources(plan);                      
    
    // 添加优化说明
    if (plan.max_parallel_tasks > constraints_.max_parallel_tasks) {
        plan.optimization_notes.push_back(
            "Warning: Plan exceeds maximum parallel task limit");
    }
    if (plan.peak_memory_usage > constraints_.max_memory_mb) {
        plan.optimization_notes.push_back(
            "Warning: Plan exceeds maximum memory limit");
    }
    
    return plan;
}

void ExecutionPlanner::updateResourceConstraints(const ResourceConstraints& new_constraints) {
    constraints_ = new_constraints;
}

bool ExecutionPlanner::validateResourceRequirements(const ExecutionPlan& plan) const {
    if (plan.max_parallel_tasks > constraints_.max_parallel_tasks) {
        return false;
    }
    if (plan.peak_memory_usage > constraints_.max_memory_mb) {
        return false;
    }
    if (plan.total_estimated_time > constraints_.max_execution_time) {
        return false;
    }
    return true;
}

ExecutionPlanner::ExecutionPlan ExecutionPlanner::optimizeForTime() {
    auto plan = generatePlan();
    
    for (auto& stage : plan.execution_stages) {
        std::sort(stage.begin(), stage.end(),
                 [](const TaskNode& a, const TaskNode& b) {
                     return a.estimated_cost > b.estimated_cost;
                 });
        
        for (auto& task : stage) {
            if (task.estimated_cost > plan.total_estimated_time * 0.2) {
                for (int alt : task.alternative_paths) {
                    if (task.estimated_cost * 0.6 > task_nodes_[alt].estimated_cost) {
                        task = task_nodes_[alt];
                        break;
                    }
                }
            }
        }
    }
    
    return plan;
}

ExecutionPlanner::ExecutionPlan ExecutionPlanner::optimizeForMemory() {
    auto plan = generatePlan();
    
    for (auto& stage : plan.execution_stages) {

        std::sort(stage.begin(), stage.end(),
                 [](const TaskNode& a, const TaskNode& b) {
                     return a.required_memory < b.required_memory;
                 });
        
        double stage_memory = std::accumulate(stage.begin(), stage.end(), 0.0,
            [](double sum, const TaskNode& task) {
                return sum + task.required_memory;
            });
        
        if (stage_memory > constraints_.max_memory_mb) {
            std::vector<TaskNode> new_stage;
            double current_memory = 0;
            
            for (const auto& task : stage) {
                if (current_memory + task.required_memory > constraints_.max_memory_mb) {
                    plan.execution_stages.push_back(new_stage);
                    new_stage.clear();
                    current_memory = 0;
                }
                new_stage.push_back(task);
                current_memory += task.required_memory;
            }
            
            if (!new_stage.empty()) {
                plan.execution_stages.push_back(new_stage);
            }
        }
    }
    
    return plan;
}

ExecutionPlanner::ExecutionPlan ExecutionPlanner::optimizeForParallelism() {
    auto plan = generatePlan();
    
    std::vector<std::vector<TaskNode>> new_stages;
    std::set<int> completed_vertices;
    std::vector<TaskNode> ready_tasks;
    
    while (completed_vertices.size() < task_nodes_.size()) {
        for (const auto& task : task_nodes_) {
            if (completed_vertices.find(task.vertex_id) == completed_vertices.end()) {
                bool dependencies_met = true;
                for (int dep : task.dependencies) {
                    if (completed_vertices.find(dep) == completed_vertices.end()) {
                        dependencies_met = false;
                        break;
                    }
                }
                if (dependencies_met) {
                    ready_tasks.push_back(task);
                }
            }
        }
        
        if (!ready_tasks.empty()) {
            std::vector<TaskNode> current_stage;
            int current_cores = 0;
            
            for (const auto& task : ready_tasks) {
                if (current_cores + task.required_cores <= constraints_.max_cpu_cores) {
                    current_stage.push_back(task);
                    current_cores += task.required_cores;
                    completed_vertices.insert(task.vertex_id);
                }
            }
            
            if (!current_stage.empty()) {
                new_stages.push_back(current_stage);
            }
            
            ready_tasks.erase(
                std::remove_if(ready_tasks.begin(), ready_tasks.end(),
                    [&completed_vertices](const TaskNode& task) {
                        return completed_vertices.find(task.vertex_id) != 
                               completed_vertices.end();
                    }),
                ready_tasks.end());
        }
    }
    
    plan.execution_stages = new_stages;
    return plan;
}

bool ExecutionPlanner::validateDependencies(const ExecutionPlan& plan) const {
    std::set<int> completed_vertices;
    
    for (const auto& stage : plan.execution_stages) {
        for (const auto& task : stage) {
            for (int dep : task.dependencies) {
                if (completed_vertices.find(dep) == completed_vertices.end()) {
                    return false;
                }
            }
            completed_vertices.insert(task.vertex_id);
        }
    }
    
    return true;
}

std::vector<ExecutionPlanner::TaskNode> ExecutionPlanner::getReadyTasks() const {
    std::vector<TaskNode> ready_tasks;
    std::set<int> completed_vertices;
    
    for (const auto& task : task_nodes_) {
        if (task.dependencies.empty()) {
            ready_tasks.push_back(task);
            completed_vertices.insert(task.vertex_id);
        }
    }
    
    return ready_tasks;
}

ExecutionPlanner::PlanAnalysis ExecutionPlanner::analyzePlan(const ExecutionPlan& plan) const {
    PlanAnalysis analysis;
    
    double theoretical_min_time = 0;
    double actual_time = plan.total_estimated_time;
    for (const auto& task : task_nodes_) {
        theoretical_min_time += task.estimated_cost;
    }
    theoretical_min_time /= constraints_.max_parallel_tasks;
    
    analysis.efficiency_score = theoretical_min_time / actual_time;
    
    for (size_t i = 0; i < plan.execution_stages.size(); ++i) {
        const auto& stage = plan.execution_stages[i];
        double stage_time = 0;
        for (const auto& task : stage) {
            stage_time = std::max(stage_time, task.estimated_cost);
        }
        
        if (stage_time > actual_time * 0.2) {
            analysis.bottlenecks.push_back(
                "Stage " + std::to_string(i) + " is a bottleneck");
        }
    }
    
    if (plan.max_parallel_tasks < constraints_.max_parallel_tasks) {
        analysis.improvement_suggestions.push_back(
            "Consider increasing parallelism");
    }
    if (plan.peak_memory_usage > constraints_.max_memory_mb * 0.9) {
        analysis.improvement_suggestions.push_back(
            "Memory usage is close to limit");
    }
    
    analysis.resource_utilization["CPU"] = 
        static_cast<double>(plan.max_parallel_tasks) / constraints_.max_parallel_tasks;
    analysis.resource_utilization["Memory"] = 
        plan.peak_memory_usage / constraints_.max_memory_mb;
    
    return analysis;
}

void ExecutionPlanner::optimizeTaskOrder(std::vector<TaskNode>& tasks) {
    std::sort(tasks.begin(), tasks.end(),
             [](const TaskNode& a, const TaskNode& b) {
                 return a.estimated_cost > b.estimated_cost;
             });
}

std::vector<ExecutionPlanner::TaskNode> ExecutionPlanner::findCriticalPath() const {
    std::vector<TaskNode> critical_path;
    auto path = graph_analysis_->findCriticalPath();
    
    for (int vertex : path) {
        critical_path.push_back(task_nodes_[vertex]);
    }
    
    return critical_path;
}

void ExecutionPlanner::balanceResources(ExecutionPlan& plan) {
    for (auto& stage : plan.execution_stages) {
        double total_memory = 0;
        int total_cores = 0;
        
        for (const auto& task : stage) {
            total_memory += task.required_memory;
            total_cores += task.required_cores;
        }
        
        if (total_memory > constraints_.max_memory_mb || 
            total_cores > constraints_.max_cpu_cores) {
            std::vector<TaskNode> new_stage;
            double current_memory = 0;
            int current_cores = 0;
            
            for (const auto& task : stage) {
                if (current_memory + task.required_memory <= constraints_.max_memory_mb &&
                    current_cores + task.required_cores <= constraints_.max_cpu_cores) {
                    new_stage.push_back(task);
                    current_memory += task.required_memory;
                    current_cores += task.required_cores;
                } else {
                    if (!new_stage.empty()) {
                        plan.execution_stages.push_back(new_stage);
                        new_stage.clear();
                        current_memory = task.required_memory;
                        current_cores = task.required_cores;
                        new_stage.push_back(task);
                    }
                }
            }
            
            if (!new_stage.empty()) {
                plan.execution_stages.push_back(new_stage);
            }
        }
    }
} 