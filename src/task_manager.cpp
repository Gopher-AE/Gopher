#include "../include/task_manager.h"
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>


void Customer::start() {
    if (!is_running) {
        is_running = true;
        worker_thread = std::thread(&Customer::run, this);
    }
}

void Customer::stop() {
    if (is_running) {
        is_running = false;
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}

void Customer::run() {
    while (is_running) {
        auto task = getTask();
        
        if (!task) {
            task = TaskManager::getInstance().tryStealTask(*this);
        }
        
        if (task) {
            processTask(*task);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}


void Producer::submitTask(std::shared_ptr<Task> task) {
    TaskManager::getInstance().submitTask(task);
}

TaskManager::~TaskManager() {
    stop();
}

void TaskManager::start() {
    if (!is_running) {
        is_running = true;
        total_tasks = 0;
        dispatcher_thread = std::thread(&TaskManager::dispatchTasks, this);
    }
}

void TaskManager::stop() {
    if (is_running) {
        is_running = false;
        if (dispatcher_thread.joinable()) {
            dispatcher_thread.join();
        }
    }
}

void TaskManager::addCustomer(std::shared_ptr<Customer> customer) {
    std::lock_guard<std::mutex> lock(customer_mutex);
    customers.push_back(customer);
    customer->start();
}

void TaskManager::addProducer(std::shared_ptr<Producer> producer) {
    std::lock_guard<std::mutex> lock(customer_mutex);
    producers.push_back(producer);
}

void TaskManager::submitTask(std::shared_ptr<Task> task) {
    scheduleTask(task);
    total_tasks++;

    if (total_tasks > load_balance_threshold) {
        balanceLoad();
    }
}

void TaskManager::scheduleTask(std::shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(customer_mutex);
    
    int min_load = std::numeric_limits<int>::max();
    int target_idx = 0;
    
    #pragma omp parallel for
    for (size_t i = 0; i < customers.size(); ++i) {
        int load = static_cast<int>(customers[i]->work_queue->getSize());
        #pragma omp critical
        {
            if (load < min_load) {
                min_load = load;
                target_idx = static_cast<int>(i);
            }
        }
    }
    
    if (target_idx < customers.size()) {
        customers[target_idx]->addTask(task);
    }
}

std::shared_ptr<Task> TaskManager::tryStealTask(const Customer& thief) {
    std::lock_guard<std::mutex> lock(customer_mutex);
    
    std::vector<size_t> indices(customers.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);
    

    for (size_t i = 0; i < customers.size(); ++i) {
        if (customers[indices[i]].get() != &thief && customers[indices[i]]->hasWork()) {
            return customers[indices[i]]->stealTask();
        }
    }
    
    return nullptr;
}

void TaskManager::balanceLoad() {
    std::lock_guard<std::mutex> lock(customer_mutex);
    
    size_t total_load = 0;
    for (const auto& customer : customers) {
        total_load += customer->work_queue->getSize();
    }
    size_t avg_load = total_load / customers.size();
    

    #pragma omp parallel for
    for (size_t i = 0; i < customers.size(); ++i) {
        auto& customer = customers[i];
        size_t current_load = customer->work_queue->getSize();
        
        if (current_load > avg_load) {

            size_t tasks_to_transfer = current_load - avg_load;
            for (size_t j = 0; j < tasks_to_transfer && customer->hasWork(); ++j) {
                auto task = customer->stealTask();
                if (task) {
                    size_t target_idx = i;
                    size_t min_load = current_load;
                    
                    for (size_t k = 0; k < customers.size(); ++k) {
                        if (k != i) {
                            size_t load = customers[k]->work_queue->getSize();
                            if (load < min_load) {
                                min_load = load;
                                target_idx = k;
                            }
                        }
                    }
                    
                    if (target_idx != i) {
                        customers[target_idx]->addTask(task);
                    } else {

                        customer->addTask(task);
                    }
                }
            }
        }
    }
    
    total_tasks = 0;
}

void TaskManager::dispatchTasks() {
    while (is_running) {
        if (total_tasks > load_balance_threshold) {
            balanceLoad();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::shared_ptr<Customer> TaskManager::getAvailableCustomer() {
    std::lock_guard<std::mutex> lock(customer_mutex);
    for (auto& customer : customers) {
        if (customer->isRunning()) {
            return customer;
        }
    }
    return nullptr;
} 