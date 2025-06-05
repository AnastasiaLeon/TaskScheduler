#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>

template<typename T> struct removeReference { using type = T; };
template<typename T> struct removeReference<T&> { using type = T; };
template<typename T> struct removeReference<T&&> { using type = T; };

template<typename T>
constexpr typename removeReference<T>::type&& myMove(T&& t) noexcept {
    return static_cast<typename removeReference<T>::type&&>(t);
}

template<typename T>
constexpr T&& myForward(typename removeReference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

class TaskScheduler {
public:
    class TaskSchedulerError {
    public:
        TaskSchedulerError(const char* msg) : message_(msg) {}
        const char* what() const noexcept { return message_; }
    private:
        const char* message_;
    };

    using TaskId = size_t;

    template<typename T>
    class FutureResult {
    public:
        T get() const {
            if (!scheduler_) {
                throw TaskSchedulerError("Invalid FutureResult: scheduler is null");
            }
            return scheduler_->getResult<T>(id_);
        }

        operator T() const { return get(); }

    private:
        friend class TaskScheduler;
        FutureResult(TaskId id, TaskScheduler* scheduler) noexcept
            : id_(id), scheduler_(scheduler) {}

        TaskId id_;
        TaskScheduler* scheduler_;
    };

    TaskScheduler() noexcept = default;
    ~TaskScheduler() noexcept = default;

    template<typename F, typename... Args>
    TaskId add(F&& func, Args&&... args) {
        static_assert(sizeof...(Args) <= 2, "Tasks can have max 2 arguments");

        using ResultType = typename std::invoke_result<F, Args...>::type;
        auto task = std::unique_ptr<TaskImpl<F, Args...>>(
            new TaskImpl<F, Args...>(myForward<F>(func), myForward<Args>(args)...));

        // First, check dependencies
        std::unordered_set<TaskId> new_deps;
        task->getDependencies(new_deps);

        // Check if all dependencies exist
        for (auto dep_id : new_deps) {
            if (tasks_.find(dep_id) == tasks_.end()) {
                throw TaskSchedulerError("Dependency task not found");
            }
        }

        // Check for cycles and self-dependencies
        if (new_deps.count(nextId_) || hasCycleWithNewTask(new_deps)) {
            throw TaskSchedulerError("Cycle detected in task dependencies");
        }

        TaskId id = nextId_++;
        try {
            tasks_[id] = myMove(task);
            dependency_graph_[id] = new_deps;
        } catch (...) {
            nextId_--;  // Roll back ID on error
            throw;
        }
        return id;
    }

    template<typename T>
    FutureResult<T> getFutureResult(TaskId id) noexcept {
        return FutureResult<T>(id, this);
    }

    template<typename T>
    T getResult(TaskId id) {
        auto it = tasks_.find(id);
        if (it == tasks_.end()) {
            throw TaskSchedulerError("Task not found");
        }

        auto* task = dynamic_cast<TaskBase<T>*>(it->second.get());
        if (!task) {
            throw TaskSchedulerError("Type mismatch in getResult");
        }

        if (!task->isExecuted()) {
            task->execute();
        }

        return task->getResult();
    }

    void executeAll() {
        std::queue<TaskId> queue;
        std::unordered_map<TaskId, size_t> in_degree;

        // Calculate in-degree for each task
        for (const auto& [id, _] : tasks_) {
            in_degree[id] = 0;
        }
        for (const auto& [id, deps] : dependency_graph_) {
            for (TaskId dep : deps) {
                in_degree[dep]++;
            }
        }

        // Add tasks with no dependencies to the queue
        for (const auto& [id, count] : in_degree) {
            if (count == 0) {
                queue.push(id);
            }
        }

        // Process tasks in topological order
        while (!queue.empty()) {
            TaskId id = queue.front();
            queue.pop();

            if (!tasks_[id]->isExecuted()) {
                tasks_[id]->execute();
            }

            // Update dependency counters and add new ready tasks
            for (TaskId dependent : reverse_dependency_graph_[id]) {
                if (--in_degree[dependent] == 0) {
                    queue.push(dependent);
                }
            }
        }

        // Check that all tasks were executed (no cycles missed)
        for (const auto& [id, task] : tasks_) {
            if (!task->isExecuted()) {
                throw TaskSchedulerError("Cycle detected during execution");
            }
        }
    }

private:
    struct ITask {
        virtual ~ITask() noexcept = default;
        virtual void execute() = 0;
        virtual bool isExecuted() const noexcept = 0;
        virtual void getDependencies(std::unordered_set<TaskId>& deps) const = 0;
    };

    template<typename T>
    struct TaskBase : ITask {
        virtual T getResult() const = 0;
    };

    template<typename F, typename... Args>
    class TaskImpl final : public TaskBase<typename std::invoke_result<F, Args...>::type> {
    public:
        using ResultType = typename std::invoke_result<F, Args...>::type;
        
        TaskImpl(F&& func, Args&&... args)
            : func_(myForward<F>(func)), args_(myForward<Args>(args)...) {}

        void execute() override {
            if (!executed_) {
                result_ = std::apply(func_, args_);
                executed_ = true;
            }
        }

        bool isExecuted() const noexcept override { return executed_; }

        ResultType getResult() const override {
            if (!executed_) {
                throw TaskSchedulerError("Task not executed");
            }
            return result_;
        }

        void getDependencies(std::unordered_set<TaskId>& deps) const override {
            getDependenciesFromTuple(deps, args_);
        }

    private:
        template<typename Tuple, size_t... Is>
        void getDependenciesFromTupleHelper(std::unordered_set<TaskId>& deps, const Tuple& tuple, std::index_sequence<Is...>) const {
            (..., processDependency(deps, std::get<Is>(tuple)));
        }

        void getDependenciesFromTuple(std::unordered_set<TaskId>& deps, const std::tuple<Args...>& args) const {
            getDependenciesFromTupleHelper(deps, args, std::index_sequence_for<Args...>{});
        }

        template<typename T>
        void processDependency(std::unordered_set<TaskId>& deps, const FutureResult<T>& fr) const {
            deps.insert(fr.id_);
        }

        template<typename U>
        void processDependency(std::unordered_set<TaskId>&, const U&) const {}

        F func_;
        std::tuple<Args...> args_;
        ResultType result_;
        bool executed_ = false;
    };

    bool hasCycleWithNewTask(const std::unordered_set<TaskId>& new_deps) const {
        std::unordered_set<TaskId> visited;
        std::unordered_set<TaskId> recursion_stack;

        // Check only new dependencies and their transitive dependencies
        for (TaskId dep_id : new_deps) {
            if (hasCycleDFS(dep_id, visited, recursion_stack)) {
                return true;
            }
        }
        return false;
    }

    bool hasCycleDFS(TaskId node, std::unordered_set<TaskId>& visited, std::unordered_set<TaskId>& recursion_stack) const {
        if (recursion_stack.count(node)) {
            return true;
        }
        if (visited.count(node)) {
            return false;
        }

        visited.insert(node);
        recursion_stack.insert(node);

        auto it = dependency_graph_.find(node);
        if (it != dependency_graph_.end()) {
            for (TaskId neighbor : it->second) {
                if (hasCycleDFS(neighbor, visited, recursion_stack)) {
                    return true;
                }
            }
        }

        recursion_stack.erase(node);
        return false;
    }

    std::unordered_map<TaskId, std::unique_ptr<ITask>> tasks_;
    std::unordered_map<TaskId, std::unordered_set<TaskId>> dependency_graph_;
    std::unordered_map<TaskId, std::unordered_set<TaskId>> reverse_dependency_graph_;
    TaskId nextId_ = 0;
};