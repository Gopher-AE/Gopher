#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <string>
#include <deque>
#include <omp.h>

class Task {
public:
    Task(const std::string& id, const std::string& type, const std::string& data, int priority = 0)
        : task_id(id), task_type(type), task_data(data), task_priority(priority) {}
    
    std::string getTaskId() const { return task_id; }
    std::string getTaskType() const { return task_type; }
    std::string getTaskData() const { return task_data; }
    int getPriority() const { return task_priority; }

private:
    std::string task_id;
    std::string task_type;
    std::string task_data;
    int task_priority;
};

class WorkQueue {
public:
    WorkQueue() : size(0) {}
    
    bool push(std::shared_ptr<Task> task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (size >= max_size) return false;
        task_queue.push_back(task);
        size++;
        return true;
    }
    
    std::shared_ptr<Task> pop() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (task_queue.empty()) return nullptr;
        auto task = task_queue.front();
        task_queue.pop_front();
        size--;
        return task;
    }
    
    std::shared_ptr<Task> steal() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (task_queue.empty()) return nullptr;
        auto task = task_queue.back();
        task_queue.pop_back();
        size--;
        return task;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return task_queue.empty();
    }
    
    size_t getSize() const {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return size;
    }

private:
    std::deque<std::shared_ptr<Task>> task_queue;
    mutable std::mutex queue_mutex;
    size_t size;
    static const size_t max_size = 1000;
};

class Customer {
public:
    Customer(const std::string& id) : customer_id(id), is_running(false), work_queue(new WorkQueue()) {}
    virtual ~Customer() { stop(); }

    void start();
    void stop();
    bool isRunning() const { return is_running; }
    std::string getId() const { return customer_id; }
    
    bool addTask(std::shared_ptr<Task> task) {
        return work_queue->push(task);
    }
    
    std::shared_ptr<Task> getTask() {
        return work_queue->pop();
    }
    
    std::shared_ptr<Task> stealTask() {
        return work_queue->steal();
    }
    
    bool hasWork() const {
        return !work_queue->empty();
    }

protected:
    virtual void processTask(const Task& task) = 0;

private:
    void run();

    std::string customer_id;
    std::atomic<bool> is_running;
    std::thread worker_thread;
    std::shared_ptr<WorkQueue> work_queue;
};


class Producer {
public:
    Producer(const std::string& id) : producer_id(id) {}
    virtual ~Producer() = default;

    void submitTask(std::shared_ptr<Task> task);
    std::string getId() const { return producer_id; }

protected:
    virtual std::shared_ptr<Task> generateTask() = 0;

private:
    std::string producer_id;
};

class TaskManager {
public:
    static TaskManager& getInstance() {
        static TaskManager instance;
        return instance;
    }

    void start();
    void stop();
    void addCustomer(std::shared_ptr<Customer> customer);
    void addProducer(std::shared_ptr<Producer> producer);
    void submitTask(std::shared_ptr<Task> task);

    std::shared_ptr<Task> tryStealTask(const Customer& thief);
    void balanceLoad();

private:
    TaskManager() = default;
    ~TaskManager();
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    void dispatchTasks();
    std::shared_ptr<Customer> getAvailableCustomer();
    void scheduleTask(std::shared_ptr<Task> task);

    std::vector<std::shared_ptr<Customer>> customers;
    std::vector<std::shared_ptr<Producer>> producers;
    
    std::mutex customer_mutex;
    std::atomic<bool> is_running;
    std::thread dispatcher_thread;

    std::atomic<size_t> total_tasks;
    static const size_t load_balance_threshold = 100;
    static const size_t max_steal_attempts = 3;
};

#endif // TASK_MANAGER_H 