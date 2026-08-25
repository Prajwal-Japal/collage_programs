#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <climits>
#include <sstream>

using namespace std;

/*
 * ==========================================================================
 * ADAPTIVE COORDINATION STRATEGY FOR MULTIPLE WAREHOUSE ROBOTS  (C++ version)
 * --------------------------------------------------------------------------
 * A beginner-friendly DSA simulation project (single file, standard C++ only)
 *
 * DSA CONCEPTS USED:
 *   1. GRAPH            -> WarehouseGraph (locations = vertices, corridors = edges)
 *   2. PRIORITY QUEUE    -> pendingTasks (urgent tasks handled first)
 *   3. MAP / HASH MAP    -> robots (fast lookup by Robot ID), graph adjacency list
 *   4. QUEUE             -> eventQueue (low battery / blocked path / failure events)
 *   5. PATHFINDING       -> Dijkstra's Algorithm (simple shortest-path search)
 * ==========================================================================
 */

// --------------------------------------------------------------------------
// ENUMS - simple fixed sets of values used across the project
// --------------------------------------------------------------------------

enum class Priority { LOW = 1, MEDIUM = 2, HIGH = 3, RUSH = 4 };

enum class TaskStatus { PENDING, ASSIGNED, IN_PROGRESS, COMPLETED, UNASSIGNED };

enum class RobotStatus { AVAILABLE, BUSY, CHARGING, FAILED };

// C++ enums don't automatically print their name like Java enums do,
// so we write small helper functions to convert them to readable text.
string priorityToString(Priority p) {
    switch (p) {
        case Priority::LOW: return "LOW";
        case Priority::MEDIUM: return "MEDIUM";
        case Priority::HIGH: return "HIGH";
        case Priority::RUSH: return "RUSH";
    }
    return "UNKNOWN";
}

string taskStatusToString(TaskStatus s) {
    switch (s) {
        case TaskStatus::PENDING: return "PENDING";
        case TaskStatus::ASSIGNED: return "ASSIGNED";
        case TaskStatus::IN_PROGRESS: return "IN_PROGRESS";
        case TaskStatus::COMPLETED: return "COMPLETED";
        case TaskStatus::UNASSIGNED: return "UNASSIGNED";
    }
    return "UNKNOWN";
}

string robotStatusToString(RobotStatus s) {
    switch (s) {
        case RobotStatus::AVAILABLE: return "AVAILABLE";
        case RobotStatus::BUSY: return "BUSY";
        case RobotStatus::CHARGING: return "CHARGING";
        case RobotStatus::FAILED: return "FAILED";
    }
    return "UNKNOWN";
}

// Small global constants (equivalent to the Java "static final" fields)
const string CHARGING_STATION = "A"; // robots recharge here
const int SAFETY_MARGIN = 5;         // extra battery kept as buffer

// Prints a vector<string> the same way Java prints an ArrayList: [A, B, C]
string pathToString(const vector<string>& path) {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < path.size(); i++) {
        oss << path[i];
        if (i + 1 < path.size()) oss << ", ";
    }
    oss << "]";
    return oss.str();
}

// --------------------------------------------------------------------------
// EDGE - represents one corridor (connection) between two warehouse locations
// --------------------------------------------------------------------------
struct Edge {
    string to;
    int weight;      // distance / cost of travelling this corridor
    bool blocked;     // true if corridor is temporarily unusable

    Edge(string to, int weight) : to(to), weight(weight), blocked(false) {}
};

// --------------------------------------------------------------------------
// NodeDistance - small helper used inside Dijkstra's algorithm.
// It is placed inside a priority_queue so that the location with the
// SMALLEST known distance is always processed first.
// --------------------------------------------------------------------------
struct NodeDistance {
    string location;
    int distance;
};

// std::priority_queue is a MAX-heap by default, so to get a MIN-heap
// (smallest distance first) we give it a comparator that reverses the order.
struct CompareNodeDistance {
    bool operator()(const NodeDistance& a, const NodeDistance& b) const {
        return a.distance > b.distance; // smaller distance = higher priority
    }
};

// --------------------------------------------------------------------------
// PathResult - simple container returned by the pathfinding method
// --------------------------------------------------------------------------
struct PathResult {
    bool found = false;
    vector<string> path;
    int distance = 0;
};

// --------------------------------------------------------------------------
// WarehouseGraph - THE GRAPH DATA STRUCTURE
// Locations (A, B, C, D, E, F) are vertices.
// Corridors between them are weighted, undirected edges.
// A hash map (unordered_map) is used so we can instantly find "what is
// connected to X" instead of scanning a whole list every time.
// --------------------------------------------------------------------------
class WarehouseGraph {
private:
    unordered_map<string, vector<Edge>> adjacencyList;

public:
    void addLocation(const string& location) {
        if (adjacencyList.find(location) == adjacencyList.end()) {
            adjacencyList[location] = vector<Edge>();
        }
    }

    // Corridors are two-way, so we add the edge in both directions
    void addCorridor(const string& loc1, const string& loc2, int weight) {
        addLocation(loc1);
        addLocation(loc2);
        adjacencyList[loc1].push_back(Edge(loc2, weight));
        adjacencyList[loc2].push_back(Edge(loc1, weight));
    }

    // A corridor is identified the same way no matter which direction
    // it is travelled in, e.g. "C-D" and "D-C" both become "C-D"
    string corridorKey(const string& a, const string& b) const {
        return (a < b) ? (a + "-" + b) : (b + "-" + a);
    }

    void blockCorridor(const string& a, const string& b) {
        for (Edge& e : adjacencyList[a]) if (e.to == b) e.blocked = true;
        for (Edge& e : adjacencyList[b]) if (e.to == a) e.blocked = true;
        cout << "[GRAPH] Corridor " << a << "-" << b << " is now BLOCKED." << endl;
    }

    void unblockCorridor(const string& a, const string& b) {
        for (Edge& e : adjacencyList[a]) if (e.to == b) e.blocked = false;
        for (Edge& e : adjacencyList[b]) if (e.to == a) e.blocked = false;
        cout << "[GRAPH] Corridor " << a << "-" << b << " is now CLEAR." << endl;
    }

    // Direct lookup of the weight of one specific corridor between two
    // ADJACENT locations (used after a path has already been calculated,
    // so we don't need to run Dijkstra again just to read a weight).
    int getDirectWeight(const string& from, const string& to) const {
        auto it = adjacencyList.find(from);
        if (it != adjacencyList.end()) {
            for (const Edge& e : it->second) {
                if (e.to == to) return e.weight;
            }
        }
        return 0;
    }

    void display() const {
        cout << "\n----- WAREHOUSE MAP (GRAPH) -----" << endl;
        for (const auto& entry : adjacencyList) {
            cout << "Location " << entry.first << "  -->  ";
            for (const Edge& e : entry.second) {
                cout << e.to << "(dist=" << e.weight << (e.blocked ? ", BLOCKED" : "") << ")  ";
            }
            cout << endl;
        }
    }

    /*
     * PATHFINDING - Dijkstra's Algorithm (simple shortest-path search)
     * -------------------------------------------------------------
     * WHY Dijkstra: our corridors have different distances (weights),
     * so a plain BFS is not enough - we need the path with the smallest
     * TOTAL distance, not just the fewest number of stops.
     *
     * HOW IT WORKS (explained simply):
     *  1. We keep a "distance so far" for every location, starting at
     *     infinity, except the start location which is 0.
     *  2. We always expand the closest not-yet-visited location first
     *     (this is exactly what a priority_queue gives us for free).
     *  3. For every neighbour of that location, if going through the
     *     current location gives a SHORTER distance than what we knew
     *     before, we update it ("relaxation").
     *  4. We skip corridors that are blocked, and (optionally) corridors
     *     that we are told to avoid, e.g. because another robot is
     *     currently using them.
     *  5. Once we reach the destination we can stop early.
     *  6. Finally we rebuild the path by walking backwards through the
     *     "previous location" map.
     *
     * avoidCorridors is a pointer so it can be nullptr (meaning "nothing to avoid"),
     * the same way the Java version passed a nullable HashSet.
     */
    PathResult findShortestPath(const string& start, const string& end,
                                 const set<string>* avoidCorridors) const {
        unordered_map<string, int> distance;
        unordered_map<string, string> previous;
        unordered_set<string> visited;
        priority_queue<NodeDistance, vector<NodeDistance>, CompareNodeDistance> pq;

        for (const auto& entry : adjacencyList) {
            distance[entry.first] = INT_MAX;
        }
        distance[start] = 0;
        pq.push(NodeDistance{start, 0});

        while (!pq.empty()) {
            NodeDistance current = pq.top();
            pq.pop();

            if (visited.count(current.location)) continue;
            visited.insert(current.location);

            if (current.location == end) break;

            auto it = adjacencyList.find(current.location);
            if (it == adjacencyList.end()) continue;

            for (const Edge& edge : it->second) {
                if (edge.blocked) continue;

                string key = corridorKey(current.location, edge.to);
                if (avoidCorridors != nullptr && avoidCorridors->count(key)) continue;

                if (visited.count(edge.to)) continue;

                int newDistance = distance[current.location] + edge.weight;
                if (newDistance < distance[edge.to]) {
                    distance[edge.to] = newDistance;
                    previous[edge.to] = current.location;
                    pq.push(NodeDistance{edge.to, newDistance});
                }
            }
        }

        PathResult result;
        auto distIt = distance.find(end);
        if (distIt == distance.end() || distIt->second == INT_MAX) {
            result.found = false;
            return result;
        }

        vector<string> path;
        string step = end;
        while (true) {
            path.insert(path.begin(), step);
            auto prevIt = previous.find(step);
            if (prevIt == previous.end()) break; // reached the start
            step = prevIt->second;
        }

        result.found = true;
        result.path = path;
        result.distance = distIt->second;
        return result;
    }
};

// --------------------------------------------------------------------------
// Task - represents one delivery/order job in the warehouse
// --------------------------------------------------------------------------
class Task {
public:
    string taskId;
    string start;
    string destination;
    Priority priority;
    TaskStatus status;
    string assignedRobotId; // empty string means "not assigned"
    int insertionOrder;     // used to keep FIFO order among equal priorities

    Task(string taskId, string start, string destination, Priority priority, int insertionOrder)
        : taskId(taskId), start(start), destination(destination), priority(priority),
          status(TaskStatus::PENDING), assignedRobotId(""), insertionOrder(insertionOrder) {}

    string toString() const {
        ostringstream oss;
        oss << taskId << " [" << start << " -> " << destination << "]  priority=" << priorityToString(priority)
            << "  status=" << taskStatusToString(status)
            << "  robot=" << (assignedRobotId.empty() ? "-" : assignedRobotId);
        return oss.str();
    }
};

// Comparator for the task priority queue.
// Higher Priority value comes first; if equal, the earlier-inserted task
// comes first (FIFO among tasks of the same priority).
struct TaskComparator {
    bool operator()(const Task* t1, const Task* t2) const {
        int v1 = static_cast<int>(t1->priority);
        int v2 = static_cast<int>(t2->priority);
        if (v1 != v2) {
            return v1 < v2; // smaller priority value => goes to the back
        }
        return t1->insertionOrder > t2->insertionOrder; // earlier task => goes to the front
    }
};

// --------------------------------------------------------------------------
// Robot - represents one warehouse robot
// --------------------------------------------------------------------------
class Robot {
public:
    string robotId;
    string currentLocation;
    int battery; // 0 - 100
    string currentTaskId; // empty string means "no active task"
    RobotStatus status;

    Robot() : robotId(""), currentLocation(""), battery(0), currentTaskId(""), status(RobotStatus::AVAILABLE) {}

    Robot(string robotId, string currentLocation, int battery)
        : robotId(robotId), currentLocation(currentLocation), battery(battery),
          currentTaskId(""), status(RobotStatus::AVAILABLE) {}

    string toString() const {
        ostringstream oss;
        oss << robotId << "  loc=" << currentLocation << "  battery=" << battery << "%"
            << "  status=" << robotStatusToString(status)
            << "  task=" << (currentTaskId.empty() ? "-" : currentTaskId);
        return oss.str();
    }
};

// --------------------------------------------------------------------------
// Event - used for the event queue (battery, blocked path, robot failure...)
// --------------------------------------------------------------------------
struct Event {
    string type;
    string description;
};

// --------------------------------------------------------------------------
// WarehouseManagementSystem - THE BRAIN OF THE PROJECT
// This class ties the Graph, Map, Priority Queue and Queue together.
//
// NOTE ON MEMORY: Task objects are created with "new" and referenced by
// raw pointer (Task*) from several places at once (the priority queue,
// the history list, and each robot's "current task"), exactly like Java
// object references. Since this is a short-lived console simulation,
// we do not bother deleting them - the operating system reclaims all
// memory automatically when the program exits.
// --------------------------------------------------------------------------
class WarehouseManagementSystem {
public:
    WarehouseGraph graph;

    // MAP #2: robots stored by ID for fast lookup
    unordered_map<string, Robot> robots;

    // PRIORITY QUEUE: pending tasks, most urgent first, FIFO among ties
    priority_queue<Task*, vector<Task*>, TaskComparator> pendingTasks;

    vector<Task*> unassignedTasks; // tasks that could not be assigned yet
    vector<Task*> allTasks;        // history of every task created

    // QUEUE: used for event handling (battery, blocked path, failure...)
    queue<Event> eventQueue;

    // Tracks which robot is currently occupying which corridor
    // (used to detect two robots wanting the same corridor)
    unordered_map<string, string> corridorOwner;

    int taskCounter = 0;

    // ---------------------------------------------------------------
    // SETUP
    // ---------------------------------------------------------------
    void setupWarehouse() {
        vector<string> locations = {"A", "B", "C", "D", "E", "F"};
        for (const string& loc : locations) graph.addLocation(loc);

        graph.addCorridor("A", "B", 4);
        graph.addCorridor("A", "C", 2);
        graph.addCorridor("B", "C", 3);
        graph.addCorridor("B", "D", 5);
        graph.addCorridor("C", "D", 3);
        graph.addCorridor("C", "E", 6);
        graph.addCorridor("D", "E", 1);
        graph.addCorridor("D", "F", 4);
        graph.addCorridor("E", "F", 2);

        cout << "Warehouse graph created. (A = charging station)" << endl;
    }

    void setupRobots() {
        addRobot("R1", "A", 80);
        addRobot("R2", "C", 10); // deliberately low battery for demo
        addRobot("R3", "D", 60);
    }

    void addRobot(const string& id, const string& location, int battery) {
        robots[id] = Robot(id, location, battery);
    }

    // ---------------------------------------------------------------
    // DISPLAY HELPERS
    // ---------------------------------------------------------------
    void displayWarehouse() {
        graph.display();
    }

    void displayRobots() {
        cout << "\n----- ROBOTS (Map: robotId -> Robot) -----" << endl;
        for (const auto& entry : robots) {
            cout << entry.second.toString() << endl;
        }
    }

    void displayTasks() {
        cout << "\n----- ALL TASKS -----" << endl;
        if (allTasks.empty()) {
            cout << "No tasks created yet." << endl;
            return;
        }
        for (Task* t : allTasks) cout << t->toString() << endl;
    }

    void displaySystemStatus() {
        cout << "\n===== SYSTEM STATUS =====" << endl;
        int available = 0, busy = 0, charging = 0, failed = 0;
        for (const auto& entry : robots) {
            switch (entry.second.status) {
                case RobotStatus::AVAILABLE: available++; break;
                case RobotStatus::BUSY: busy++; break;
                case RobotStatus::CHARGING: charging++; break;
                case RobotStatus::FAILED: failed++; break;
            }
        }
        cout << "Robots -> available:" << available << " busy:" << busy
             << " charging:" << charging << " failed:" << failed << endl;
        cout << "Pending tasks in priority queue: " << pendingTasks.size() << endl;
        cout << "Unassigned tasks waiting for retry: " << unassignedTasks.size() << endl;
        cout << "Total tasks created so far: " << allTasks.size() << endl;
        processEvents();
    }

    // ---------------------------------------------------------------
    // TASK CREATION
    // ---------------------------------------------------------------
    Task* createTask(const string& start, const string& destination, Priority priority) {
        taskCounter++;
        string id = "T" + to_string(taskCounter);
        Task* task = new Task(id, start, destination, priority, taskCounter);
        allTasks.push_back(task);
        pendingTasks.push(task); // goes straight into the priority queue
        cout << "[TASK CREATED] " << task->toString() << endl;
        return task;
    }

    // ---------------------------------------------------------------
    // EVENT QUEUE HANDLING
    // ---------------------------------------------------------------
    void logEvent(const string& type, const string& description) {
        eventQueue.push(Event{type, description});
    }

    // Processes (prints and removes) every event currently waiting in the queue.
    void processEvents() {
        if (eventQueue.empty()) return;
        cout << "\n----- PROCESSING EVENT QUEUE -----" << endl;
        while (!eventQueue.empty()) {
            Event e = eventQueue.front();
            eventQueue.pop(); // FIFO: oldest event handled first
            cout << "[EVENT: " << e.type << "] " << e.description << endl;
        }
    }

    // ---------------------------------------------------------------
    // TASK ASSIGNMENT
    // This is where the Graph, Map and Priority Queue all work together.
    // ---------------------------------------------------------------
    void assignTasks() {
        cout << "\n===== ASSIGNING TASKS =====" << endl;

        // Retry any tasks that failed to get a robot last time
        if (!unassignedTasks.empty()) {
            cout << "Retrying " << unassignedTasks.size() << " previously unassigned task(s)..." << endl;
            for (Task* t : unassignedTasks) pendingTasks.push(t);
            unassignedTasks.clear();
        }

        if (pendingTasks.empty()) {
            cout << "No pending tasks to assign." << endl;
            return;
        }

        // Process every task currently in the priority queue exactly once
        int roundsToProcess = (int) pendingTasks.size();
        for (int i = 0; i < roundsToProcess; i++) {
            Task* task = pendingTasks.top();
            pendingTasks.pop(); // most urgent task comes out first
            cout << "\nProcessing " << task->taskId << " (priority=" << priorityToString(task->priority) << ")" << endl;
            assignSingleTask(task);
        }
    }

private:
    void assignSingleTask(Task* task) {
        Robot* bestRobot = nullptr;
        PathResult bestPickupPath;
        int bestDistance = INT_MAX;

        Robot* lowBatteryCandidate = nullptr;
        PathResult lowBatteryPickupPath;

        // SEARCH: scan every robot (map values) to find the nearest
        // AVAILABLE robot that also has enough battery for the whole trip.
        for (auto& entry : robots) {
            Robot& r = entry.second;
            if (r.status != RobotStatus::AVAILABLE) continue;

            PathResult pickupPath = graph.findShortestPath(r.currentLocation, task->start, nullptr);
            if (!pickupPath.found) continue;

            PathResult deliveryPath = graph.findShortestPath(task->start, task->destination, nullptr);
            if (!deliveryPath.found) continue;

            int totalNeeded = pickupPath.distance + deliveryPath.distance + SAFETY_MARGIN;

            if (r.battery >= totalNeeded) {
                if (pickupPath.distance < bestDistance) {
                    bestDistance = pickupPath.distance;
                    bestRobot = &r;
                    bestPickupPath = pickupPath;
                }
            } else {
                // Remember this robot in case NO robot has enough battery
                if (lowBatteryCandidate == nullptr || pickupPath.distance < lowBatteryPickupPath.distance) {
                    lowBatteryCandidate = &r;
                    lowBatteryPickupPath = pickupPath;
                }
            }
        }

        if (bestRobot != nullptr) {
            executeTask(*bestRobot, task, bestPickupPath);
            return;
        }

        // No robot currently has enough battery - handle the low battery event
        if (lowBatteryCandidate != nullptr) {
            logEvent("LOW_BATTERY", lowBatteryCandidate->robotId +
                    " does not have enough battery (" + to_string(lowBatteryCandidate->battery) +
                    "%) for task " + task->taskId + ". Sending it to charge.");
            cout << "[LOW BATTERY] " << lowBatteryCandidate->robotId
                 << " battery too low for this task. Redirecting to charging station "
                 << CHARGING_STATION << "..." << endl;

            chargeRobot(*lowBatteryCandidate);

            // Try again now that the robot is fully charged
            PathResult pickupPath = graph.findShortestPath(lowBatteryCandidate->currentLocation, task->start, nullptr);
            PathResult deliveryPath = graph.findShortestPath(task->start, task->destination, nullptr);
            if (pickupPath.found && deliveryPath.found &&
                lowBatteryCandidate->battery >= pickupPath.distance + deliveryPath.distance + SAFETY_MARGIN) {
                executeTask(*lowBatteryCandidate, task, pickupPath);
                return;
            }
        }

        // Still nothing could be done -> keep task for a later retry
        task->status = TaskStatus::UNASSIGNED;
        unassignedTasks.push_back(task);
        cout << "[UNASSIGNED] No suitable robot found right now for " << task->taskId
             << ". It will be retried on the next 'Assign Tasks' run." << endl;
    }

    // Moves the chosen robot to the pickup point, then to the destination,
    // and marks the task completed.
    void executeTask(Robot& robot, Task* task, PathResult& pickupPath) {
        robot.status = RobotStatus::BUSY;
        robot.currentTaskId = task->taskId;
        task->status = TaskStatus::ASSIGNED;
        task->assignedRobotId = robot.robotId;

        cout << "[ASSIGNED] " << robot.robotId << " -> " << task->taskId << endl;

        // Move to pickup location (if not already there)
        if (robot.currentLocation != task->start) {
            cout << robot.robotId << " travelling to pickup point " << task->start << "..." << endl;
            moveRobotAlongPath(robot, pickupPath.path);
        }

        // Move from pickup point to destination
        PathResult deliveryPath = graph.findShortestPath(robot.currentLocation, task->destination, nullptr);
        cout << robot.robotId << " delivering " << task->taskId << " to " << task->destination << "..." << endl;
        moveRobotAlongPath(robot, deliveryPath.path);

        task->status = TaskStatus::COMPLETED;
        robot.status = RobotStatus::AVAILABLE;
        robot.currentTaskId = "";
        cout << "[COMPLETED] " << task->taskId << " delivered by " << robot.robotId
             << ". Remaining battery: " << robot.battery << "%" << endl;
    }

public:
    /*
     * Moves a robot step by step along a path.
     * Also demonstrates CORRIDOR CONFLICT detection: before crossing a
     * corridor we check whether another robot currently owns it. If so,
     * we reroute around that corridor instead of colliding.
     */
    void moveRobotAlongPath(Robot& robot, vector<string>& path) {
        for (size_t i = 0; i + 1 < path.size(); i++) {
            string from = path[i];
            string to = path[i + 1];
            string key = graph.corridorKey(from, to);

            auto ownerIt = corridorOwner.find(key);
            if (ownerIt != corridorOwner.end() && ownerIt->second != robot.robotId) {
                // CONFLICT: another robot is using this corridor right now
                cout << "[CORRIDOR CONFLICT] " << robot.robotId << " wants corridor " << key
                     << " but it is currently used by " << ownerIt->second << ". Rerouting..." << endl;
                logEvent("CORRIDOR_CONFLICT", robot.robotId + " rerouted away from busy corridor " + key);

                set<string> avoid;
                avoid.insert(key);
                string finalDestination = path.back();
                PathResult alternate = graph.findShortestPath(from, finalDestination, &avoid);

                if (alternate.found) {
                    cout << robot.robotId << " found alternate route: " << pathToString(alternate.path) << endl;
                    moveRobotAlongPath(robot, alternate.path); // continue on the new route
                    return;
                } else {
                    cout << "[BLOCKED] No alternate route available for " << robot.robotId
                         << ". Waiting for corridor to free up." << endl;
                    logEvent("NO_ALTERNATE_ROUTE", robot.robotId + " could not find another path around " + key);
                    return;
                }
            }

            // Occupy the corridor while crossing it
            corridorOwner[key] = robot.robotId;

            int edgeWeight = getEdgeWeight(from, to);
            robot.battery -= edgeWeight;
            robot.currentLocation = to;
            cout << "   " << robot.robotId << " moved " << from << " -> " << to
                 << "  (battery now " << robot.battery << "%)" << endl;

            if (robot.battery <= 15) {
                logEvent("LOW_BATTERY_WARNING", robot.robotId + " battery is low (" + to_string(robot.battery) + "%).");
            }

            // Release the corridor once the robot has fully crossed it
            corridorOwner.erase(key);
        }
    }

private:
    int getEdgeWeight(const string& from, const string& to) {
        return graph.getDirectWeight(from, to);
    }

public:
    // Sends a robot to the charging station and recharges it to 100%
    void chargeRobot(Robot& robot) {
        RobotStatus previousStatus = robot.status;
        robot.status = RobotStatus::CHARGING;

        if (robot.currentLocation != CHARGING_STATION) {
            PathResult toCharger = graph.findShortestPath(robot.currentLocation, CHARGING_STATION, nullptr);
            if (toCharger.found) {
                moveRobotAlongPath(robot, toCharger.path);
            }
        }

        cout << robot.robotId << " is charging at " << CHARGING_STATION << "..." << endl;
        robot.battery = 100;
        cout << robot.robotId << " fully charged (100%)." << endl;
        robot.status = (previousStatus == RobotStatus::BUSY) ? RobotStatus::AVAILABLE : previousStatus;
        if (robot.status == RobotStatus::CHARGING) robot.status = RobotStatus::AVAILABLE;
    }

    // ---------------------------------------------------------------
    // ROBOT FAILURE + TASK REASSIGNMENT
    // ---------------------------------------------------------------
    void simulateRobotFailure(const string& robotId) {
        auto it = robots.find(robotId);
        if (it == robots.end()) {
            cout << "No such robot: " << robotId << endl;
            return;
        }
        Robot& robot = it->second;

        robot.status = RobotStatus::FAILED;
        logEvent("ROBOT_FAILURE", robotId + " has failed and is now offline.");
        cout << "[FAILURE] " << robotId << " has broken down!" << endl;

        if (!robot.currentTaskId.empty()) {
            Task* failedTask = findTaskById(robot.currentTaskId);
            if (failedTask != nullptr && failedTask->status != TaskStatus::COMPLETED) {
                failedTask->status = TaskStatus::PENDING;
                failedTask->assignedRobotId = "";
                pendingTasks.push(failedTask); // put back into the priority queue
                cout << "[REASSIGN] " << failedTask->taskId
                     << " returned to the pending queue for another robot." << endl;
            }
            robot.currentTaskId = "";
        }
    }

    Task* findTaskById(const string& taskId) {
        for (Task* t : allTasks) if (t->taskId == taskId) return t;
        return nullptr;
    }

    // ---------------------------------------------------------------
    // MANUAL PATHFINDING / BATTERY CHECK (used by menu options)
    // ---------------------------------------------------------------
    void findRobotPath(const string& robotId, const string& destination) {
        auto it = robots.find(robotId);
        if (it == robots.end()) {
            cout << "No such robot: " << robotId << endl;
            return;
        }
        Robot& robot = it->second;
        PathResult result = graph.findShortestPath(robot.currentLocation, destination, nullptr);
        if (!result.found) {
            cout << "No path found from " << robot.currentLocation << " to " << destination << "." << endl;
            return;
        }
        cout << "Path: " << pathToString(result.path) << "   Total distance: " << result.distance << endl;
        if (robot.battery < result.distance + SAFETY_MARGIN) {
            cout << "WARNING: " << robotId << " battery (" << robot.battery
                 << "%) may not be enough for this trip (needs about "
                 << (result.distance + SAFETY_MARGIN) << "%)." << endl;
        }
    }

    void checkBattery(const string& robotId) {
        auto it = robots.find(robotId);
        if (it == robots.end()) {
            cout << "No such robot: " << robotId << endl;
            return;
        }
        Robot& robot = it->second;
        cout << robotId << " battery: " << robot.battery << "%" << endl;
        if (robot.battery <= 20) {
            cout << "WARNING: Battery is low. Recommend charging soon." << endl;
        }
    }

    // ---------------------------------------------------------------
    // DEMONSTRATION: two robots wanting the SAME corridor at once
    // ---------------------------------------------------------------
    void demonstrateCorridorConflict() {
        cout << "\n===== DEMO: TWO ROBOTS, SAME CORRIDOR =====" << endl;
        auto it = robots.find("R1");
        if (it == robots.end() || it->second.status == RobotStatus::FAILED) {
            cout << "R1 is not available for this demo." << endl;
            return;
        }
        Robot& r1 = it->second;

        // Pretend a "ghost" robot is already using corridor B-D
        string key = graph.corridorKey("B", "D");
        corridorOwner[key] = "R_GHOST";
        cout << "(Simulating that another robot is currently occupying corridor B-D)" << endl;

        r1.currentLocation = "B"; // place R1 right next to the busy corridor for the demo
        PathResult toF = graph.findShortestPath("B", "F", nullptr);
        cout << r1.robotId << " wants to travel B -> F using route: " << pathToString(toF.path) << endl;
        moveRobotAlongPath(r1, toF.path);

        // Clean up the pretend occupation
        corridorOwner.erase(key);
    }

    // ---------------------------------------------------------------
    // FULL AUTOMATIC DEMONSTRATION (menu option 11)
    // ---------------------------------------------------------------
    void runCompleteDemo() {
        cout << "\n##########################################################" << endl;
        cout << "#           RUNNING COMPLETE SYSTEM DEMONSTRATION       #" << endl;
        cout << "##########################################################" << endl;

        cout << "\n--- STEP 1: Initial warehouse and robot state ---" << endl;
        displayWarehouse();
        displayRobots();

        cout << "\n--- STEP 2: Normal task assignment ---" << endl;
        createTask("A", "F", Priority::MEDIUM);
        assignTasks();

        cout << "\n--- STEP 3: Rush / high priority task jumps the queue ---" << endl;
        createTask("A", "E", Priority::LOW);
        createTask("A", "B", Priority::RUSH);
        cout << "(Even though the LOW task was added first, RUSH will be processed first)" << endl;
        assignTasks();

        cout << "\n--- STEP 4: Low battery robot (R2, 10%) gets a task ---" << endl;
        createTask("C", "F", Priority::HIGH); // R2 starts at C, this should trigger low battery handling
        assignTasks();

        cout << "\n--- STEP 5: Blocked corridor forces a reroute ---" << endl;
        graph.blockCorridor("C", "D");
        findRobotPath("R1", "F");
        graph.unblockCorridor("C", "D");

        cout << "\n--- STEP 6: Two robots wanting the same corridor ---" << endl;
        demonstrateCorridorConflict();

        cout << "\n--- STEP 7: Robot failure and automatic task reassignment ---" << endl;
        createTask("D", "A", Priority::HIGH);
        assignTasks(); // this assigns the new task to some robot
        simulateRobotFailure("R3");
        assignTasks(); // this should pick up any task R3 was doing

        cout << "\n--- STEP 8: Final system status ---" << endl;
        displayRobots();
        displayTasks();
        displaySystemStatus();

        cout << "\n##########################################################" << endl;
        cout << "#                 DEMONSTRATION COMPLETE                #" << endl;
        cout << "##########################################################" << endl;
    }
};

// --------------------------------------------------------------------------
// SMALL STRING HELPERS (equivalent to Java's .trim() and .toUpperCase())
// --------------------------------------------------------------------------
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string toUpperStr(const string& s) {
    string result = s;
    for (char& c : result) c = toupper((unsigned char) c);
    return result;
}

// --------------------------------------------------------------------------
// MENU HANDLER FUNCTIONS - console driven, same layout as the Java version
// --------------------------------------------------------------------------
void printMenu() {
    cout << "\n================ WAREHOUSE ROBOT SIMULATION ================" << endl;
    cout << "1.  Display warehouse" << endl;
    cout << "2.  Display robots" << endl;
    cout << "3.  Add task" << endl;
    cout << "4.  Assign tasks" << endl;
    cout << "5.  Find robot path" << endl;
    cout << "6.  Simulate movement" << endl;
    cout << "7.  Block a corridor" << endl;
    cout << "8.  Simulate robot failure" << endl;
    cout << "9.  Check battery" << endl;
    cout << "10. Display system status" << endl;
    cout << "11. Run complete demonstration" << endl;
    cout << "12. Exit" << endl;
}

void handleAddTask(WarehouseManagementSystem& system) {
    string line;
    cout << "Start location (A-F): ";
    getline(cin, line);
    string start = toUpperStr(trim(line));

    cout << "Destination location (A-F): ";
    getline(cin, line);
    string dest = toUpperStr(trim(line));

    cout << "Priority: 1=LOW 2=MEDIUM 3=HIGH 4=RUSH" << endl;
    cout << "Choose priority: ";
    getline(cin, line);
    string p = trim(line);

    Priority priority;
    if (p == "1") priority = Priority::LOW;
    else if (p == "2") priority = Priority::MEDIUM;
    else if (p == "3") priority = Priority::HIGH;
    else if (p == "4") priority = Priority::RUSH;
    else {
        cout << "Invalid priority, defaulting to MEDIUM." << endl;
        priority = Priority::MEDIUM;
    }
    system.createTask(start, dest, priority);
}

void handleFindPath(WarehouseManagementSystem& system) {
    string line;
    cout << "Robot ID (e.g. R1): ";
    getline(cin, line);
    string robotId = toUpperStr(trim(line));

    cout << "Destination location: ";
    getline(cin, line);
    string dest = toUpperStr(trim(line));

    system.findRobotPath(robotId, dest);
}

void handleSimulateMovement(WarehouseManagementSystem& system) {
    string line;
    cout << "Robot ID to move: ";
    getline(cin, line);
    string robotId = toUpperStr(trim(line));

    auto it = system.robots.find(robotId);
    if (it == system.robots.end()) {
        cout << "No such robot." << endl;
        return;
    }
    Robot& robot = it->second;
    if (robot.currentTaskId.empty()) {
        cout << robotId << " has no active task to move towards." << endl;
        return;
    }
    Task* task = system.findTaskById(robot.currentTaskId);
    PathResult path = system.graph.findShortestPath(robot.currentLocation, task->destination, nullptr);
    if (path.found) {
        system.moveRobotAlongPath(robot, path.path);
    } else {
        cout << "No path available right now." << endl;
    }
}

void handleBlockCorridor(WarehouseManagementSystem& system) {
    string line;
    cout << "First location: ";
    getline(cin, line);
    string a = toUpperStr(trim(line));

    cout << "Second location: ";
    getline(cin, line);
    string b = toUpperStr(trim(line));

    system.graph.blockCorridor(a, b);
    system.logEvent("BLOCKED_PATH", "Corridor " + a + "-" + b + " was manually blocked.");
}

void handleRobotFailure(WarehouseManagementSystem& system) {
    string line;
    cout << "Robot ID to fail: ";
    getline(cin, line);
    string robotId = toUpperStr(trim(line));
    system.simulateRobotFailure(robotId);
}

void handleCheckBattery(WarehouseManagementSystem& system) {
    string line;
    cout << "Robot ID: ";
    getline(cin, line);
    string robotId = toUpperStr(trim(line));
    system.checkBattery(robotId);
}

// --------------------------------------------------------------------------
// MAIN - menu driven console program
// --------------------------------------------------------------------------
int main() {
    WarehouseManagementSystem system;

    system.setupWarehouse();
    system.setupRobots();

    bool running = true;
    while (running) {
        printMenu();
        cout << "Enter your choice: ";

        string choiceStr;
        if (!getline(cin, choiceStr)) break; // input stream ended (e.g. piped input)
        choiceStr = trim(choiceStr);

        int choice;
        try {
            choice = stoi(choiceStr);
        } catch (...) {
            cout << "Please enter a valid number." << endl;
            continue;
        }

        switch (choice) {
            case 1:
                system.displayWarehouse();
                break;
            case 2:
                system.displayRobots();
                break;
            case 3:
                handleAddTask(system);
                break;
            case 4:
                system.assignTasks();
                break;
            case 5:
                handleFindPath(system);
                break;
            case 6:
                handleSimulateMovement(system);
                break;
            case 7:
                handleBlockCorridor(system);
                break;
            case 8:
                handleRobotFailure(system);
                break;
            case 9:
                handleCheckBattery(system);
                break;
            case 10:
                system.displaySystemStatus();
                break;
            case 11:
                system.runCompleteDemo();
                break;
            case 12:
                running = false;
                cout << "Exiting simulation. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice, please try again." << endl;
        }
    }

    return 0;
}