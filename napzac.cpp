#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>

using namespace std;
using namespace std::chrono;

// タスクの構造体
struct Task {
    int time;
    int value;
};

// 力任せ法
int bruteForceKnapsack(vector<Task>& tasks, int totalTime, vector<int>& selectedTasks) {
    int n = tasks.size();
    int maxValue = 0;
    int bestMask = 0;

    for (int mask = 0; mask < (1 << n); ++mask) {
        int currentTime = 0;
        int currentValue = 0;
        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) {
                currentTime += tasks[i].time;
                currentValue += tasks[i].value;
            }
        }
        if (currentTime <= totalTime && currentValue > maxValue) {
            maxValue = currentValue;
            bestMask = mask;
        }
    }

    for (int i = 0; i < n; ++i) {
        if (bestMask & (1 << i)) {
            selectedTasks.push_back(i);
        }
    }

    return maxValue;
}

// 分枝限定法
struct Node {
    int level;
    int value;
    int weight;
    float bound;
    vector<int> items;
};

bool compare(Task a, Task b) {
    return (float)a.value / a.time > (float)b.value / a.time;
}

float bound(Node u, int n, int totalTime, vector<Task>& tasks) {
    if (u.weight >= totalTime) return 0;

    float profit_bound = u.value;
    int j = u.level + 1;
    int totalWeight = u.weight;

    while ((j < n) && (totalWeight + tasks[j].time <= totalTime)) {
        totalWeight += tasks[j].time;
        profit_bound += tasks[j].value;
        j++;
    }

    if (j < n) {
        profit_bound += (totalTime - totalWeight) * tasks[j].value / (float)tasks[j].time;
    }

    return profit_bound;
}

int branchAndBoundKnapsack(vector<Task>& tasks, int totalTime, vector<int>& selectedTasks) {
    sort(tasks.begin(), tasks.end(), compare);
    vector<Node> Q;

    Node u, v;
    u.level = -1;
    u.value = 0;
    u.weight = 0;
    u.bound = bound(u, tasks.size(), totalTime, tasks);
    Q.push_back(u);

    int maxProfit = 0;
    vector<int> bestItems;

    while (!Q.empty()) {
        u = Q.back();
        Q.pop_back();

        if (u.level == -1) v.level = 0;
        else v.level = u.level + 1;

        if (v.level < tasks.size()) {
            v.weight = u.weight + tasks[v.level].time;
            v.value = u.value + tasks[v.level].value;
            v.items = u.items;
            v.items.push_back(v.level);

            if (v.weight <= totalTime && v.value > maxProfit) {
                maxProfit = v.value;
                bestItems = v.items;
            }

            v.bound = bound(v, tasks.size(), totalTime, tasks);
            if (v.bound > maxProfit) Q.push_back(v);

            v.weight = u.weight;
            v.value = u.value;
            v.items = u.items;
            v.bound = bound(v, tasks.size(), totalTime, tasks);
            if (v.bound > maxProfit) Q.push_back(v);
        }
    }

    selectedTasks = bestItems;
    return maxProfit;
}


// 動的計画法
int dynamicProgrammingKnapsack(vector<Task>& tasks, int totalTime, vector<int>& selectedTasks) {
    int n = tasks.size();
    vector<vector<int>> dp(n + 1, vector<int>(totalTime + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int t = 1; t <= totalTime; ++t) {
            if (tasks[i - 1].time <= t) {
                dp[i][t] = max(dp[i - 1][t], dp[i - 1][t - tasks[i - 1].time] + tasks[i - 1].value);
            } else {
                dp[i][t] = dp[i - 1][t];
            }
        }
    }

    int t = totalTime;
    for (int i = n; i > 0 && t > 0; --i) {
        if (dp[i][t] != dp[i - 1][t]) {
            selectedTasks.push_back(i - 1);
            t -= tasks[i - 1].time;
        }
    }

    return dp[n][totalTime];
}

// 線形計画法
int linearProgrammingKnapsack(vector<Task>& tasks, int totalTime, vector<int>& selectedTasks) {
    sort(tasks.begin(), tasks.end(), compare);
    int currentTime = 0;
    int currentValue = 0;

    for (int i = 0; i < tasks.size(); ++i) {
        if (currentTime + tasks[i].time <= totalTime) {
            currentTime += tasks[i].time;
            currentValue += tasks[i].value;
            selectedTasks.push_back(i);
        } else {
            break;
        }
    }

    return currentValue;
}

bool checkFeasibility(vector<Task>& tasks, int totalTime, vector<int>& selectedTasks) {
    int totalSelectedTime = 0;
    for (int i : selectedTasks) {
        totalSelectedTime += tasks[i].time;
    }
    return totalSelectedTime <= totalTime;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile) {
        cerr << "Error opening file: " << argv[1] << endl;
        return 1;
    }

    int n, totalTime;
    inputFile >> n >> totalTime;

    vector<Task> tasks(n);
    for (int i = 0; i < n; ++i) {
        inputFile >> tasks[i].time >> tasks[i].value;
    }
    /*
    cout << "Number of tasks: " << n << endl;
    cout << "Total time: " << totalTime << endl;
    for(int i=0; i<n; i++){
        cout << "Task " << i+1 << ": " << tasks[i].time << " " << tasks[i].value << endl;
    }
    */
    inputFile.close();

    cout << "Select the algorithm to solve the knapsack problem:\n";
    cout << "1. Brute Force\n";
    cout << "2. Branch and Bound\n";
    cout << "3. Dynamic Programming\n";
    cout << "4. Linear Programming (Greedy Approximation)\n";
    cout << "Enter your choice: ";
    int choice;
    cin >> choice;

    vector<int> selectedTasks;
    int maxValue = 0;
    auto start = high_resolution_clock::now();

    switch (choice) {
        case 1:
            maxValue = bruteForceKnapsack(tasks, totalTime, selectedTasks);
            break;
        case 2:
            maxValue = branchAndBoundKnapsack(tasks, totalTime, selectedTasks);
            break;
        case 3:
            maxValue = dynamicProgrammingKnapsack(tasks, totalTime, selectedTasks);
            break;
        case 4:
            maxValue = linearProgrammingKnapsack(tasks, totalTime, selectedTasks);
            break;
        default:
            cerr << "Invalid choice." << endl;
            return 1;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    if (checkFeasibility(tasks, totalTime, selectedTasks)) {
        cout << "Maximum value: " << maxValue << endl;
        cout << "Selected tasks: ";
        for (int i : selectedTasks) {
            cout << i << " ";
        }
        cout << endl;
    } else {
        cout << "The selected tasks do not satisfy the time constraint." << endl;
    }

    cout << "Execution time: " << duration.count() << " microseconds" << endl;

    return 0;
}
