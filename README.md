# TaskScheduler
A C++ implementation of a dependency-aware task scheduler that executes a graph of interdependent tasks

## Description
**TaskScheduler** is a class designed to manage and execute tasks linked by data dependencies. It processes a computation graph where:
- Nodes represent tasks (operations).
- Edges define dependencies between tasks (output of one task becomes input for another).
Optimizes execution by avoiding redundant calculations (e.g., reusing the discriminant in a quadratic equation solver).

## Example: Quadratic Equation Solver
Finds roots of a quadratic equation (assuming 2 real roots). Demonstrates efficient reuse of intermediate results (e.g., discriminant calculation):

```cpp
struct AddNumber {
  float add(float a) const {
    return a + number;
  }

  float number;
};

float a = 1;
float b = -2;
float c = 0;
AddNumber add{
  .number = 3
};

TaskScheduler scheduler;

auto id1 = scheduler.add([](float a, float c) {return -4 * a * c;}, a, c);

auto id2 = scheduler.add([](float b, float v) {return b * b + v;}, b, scheduler.getFutureResult<float>(id1));

auto id3 = scheduler.add([](float b, float d) {return -b + std::sqrt(d);}, b, scheduler.getFutureResult<float>(id2));

auto id4 = scheduler.add([](float b, float d) {return -b - std::sqrt(d);}, b, scheduler.getFutureResult<float>(id2));

auto id5 = scheduler.add([](float a, float v) {return v/(2*a);}, a, scheduler.getFutureResult<float>(id3));

auto id6 = scheduler.add([]{float a, float v} {return v/(2*a);}, a, scheduler.getFutureResult<float>(id4));

auto id7 = scheduler.add(&AddNumber::add, add, scheduler.getFutureResult<float>(id6));

scheduler.executeAll();

std::cout << "x1 = " << scheduler.getResult<float>(id5) << std::endl;
std::cout << "x2 = " << scheduler.getResult<float>(id6) << std::endl;
std::cout << "x3 = " << scheduler.getResult<float>(id7) << std::endl;
```

Key Concepts:
- getFutureResult<T>: Represents a future result of a task (type T).
- Lazy evaluation: Tasks compute only when needed (via getResult or executeAll).

## Public Interface

 - **add** - accepts a task to be executed. Returns a descriptor object for the added task.
 - **getFutureResult<T>** - returns a future result object that will contain the task's output of type T once computed.
 - **getResult<T>** - returns the computed result of type T for the specified task. Executes the task if not already calculated, without running unnecessary dependent tasks.
 - **executeAll** - executes all scheduled tasks in the correct dependency order.

## Operational Constraints:

 - Maximum of 2 arguments per task.
 - Tasks can be class method pointers. In such cases:
 - - The first argument must be the class instance
 - - The instance itself counts as one argument

## Implementation Notes

- DAG Constraints:
Cyclic dependencies are detected and rejected (tested via GoogleTest).
- Efficiency:
Each task executes once, even if referenced multiple times.
- Flexibility:
Tasks can modify external state (e.g., AddNumber example).
- Standard Library Restrictions
Uses only the following STL components: containers and smart pointers (other standard library components may be used sparingly)

## Testing

Complete test coverage including:
- Cyclic dependency detection
- All core functionality
Using Google Test framework.