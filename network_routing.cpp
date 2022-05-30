#include <iostream>
#include <vector>
#include <sstream>
#include <map>

#define MAX_N 100
#define SPACE ' '
#define TOPOLOGY_COMMAND "topology"
#define TOPOLOGY_DELIMITER '-'
#define SHOW_COMMAND "show"
#define LSRP_COMMAND "lsrp"
#define DVRP_COMMAND "dvrp"
#define MODIFY_COMMAND "modify"
#define REMOVE_COMMAND "remove"

const int INF = 1e9;

using namespace std;

int topologies_count = 0;
map<pair<int, int>, int> edges;
int topologies[MAX_N][MAX_N] = {-1};
bool topologies_set = false;


void initialize_topologies(const int &default_value);

void validate_topologies();

vector<string> parse_string(const string &str, char delimiter);

void set_topology(const vector<string> &topologies_str);

void modify_topology(const vector<string> &topologies_str);

void remove_topology(const vector<string> &topologies_str);

string get_topology_str();

void lsrp(const vector<string> &lsrp_src_node);

void dvrp(const vector<string> &dvrp_src_node);

int min_distance(const int distances[], const bool test[]);

void print_parents(const int parent[], int j, stringstream &ss);

void dijkstra(int src_node);

void bellman_ford(int src_node);

void get_command();


int main() {
  initialize_topologies(-1);
  get_command();
}


void validate_topologies() {
  if (!topologies_set) {
    cerr << "Topologies not set" << endl;
  }
}

vector<string> parse_string(const string &str, char delimiter) {
  vector<string> result;
  stringstream ss(str);
  string token;
  while (getline(ss, token, delimiter)) {
    result.push_back(token);
  }
  return result;
}


void set_topology(const vector<string> &topologies_str) {
  auto n = topologies_str.size();
  for (int i = 0; i < n; i++) {
    auto topology_str = parse_string(topologies_str[i], TOPOLOGY_DELIMITER);
    auto source = stoi(topology_str[0]);
    auto destination = stoi(topology_str[1]);
    auto cost = stoi(topology_str[2]);
    if (source == destination) {
      throw invalid_argument("source and destination cannot be the same");
    }
    if (topologies[source][destination] != -1 || topologies[destination][source] != -1) {
      throw invalid_argument("topology already set");
    }
    topologies[source][destination] = topologies[destination][source] = cost;
    topologies[source][source] = topologies[destination][destination] = 0;
    edges.insert(make_pair(make_pair(source, destination), cost));
    edges.insert(make_pair(make_pair(destination, source), cost));
    topologies_count = max(topologies_count, max(source, destination));
  }
  topologies_set = true;
}


void get_command() {
  string command;
  while (getline(cin, command)) {
    if (command == "exit")
      break;
    auto parsed_command = parse_string(command, SPACE);
    if (parsed_command[0] == TOPOLOGY_COMMAND) {
      parsed_command.erase(parsed_command.begin());
      set_topology(parsed_command);
    } else if (parsed_command[0] == SHOW_COMMAND) {
      cout << get_topology_str();
    } else if (parsed_command[0] == LSRP_COMMAND) {
      parsed_command.erase(parsed_command.begin());
      lsrp(parsed_command);
    } else if (parsed_command[0] == DVRP_COMMAND) {
      parsed_command.erase(parsed_command.begin());
      dvrp(parsed_command);
    } else if (parsed_command[0] == MODIFY_COMMAND) {
      parsed_command.erase(parsed_command.begin());
      modify_topology(parsed_command);
    } else if (parsed_command[0] == REMOVE_COMMAND) {
      parsed_command.erase(parsed_command.begin());
      remove_topology(parsed_command);
    } else {
      cout << "invalid command" << endl;
    }
  }
}

string get_topology_str() {
  validate_topologies();
  stringstream ss;
  ss << "u|v | ";
  for (int i = 0; i < topologies_count; i++) {
    ss << i + 1 << " ";
  }
  ss << endl;
  for (int i = 0; i < 3 * topologies_count + 1; i++) {
    ss << "-";
  }
  ss << endl;
  for (int i = 0; i < topologies_count; i++) {
    ss << i + 1 << " | ";
    for (int j = 0; j < topologies_count; j++) {
      ss << topologies[i + 1][j + 1] << " ";
    }
    ss << endl;
  }
  return ss.str();
}

void initialize_topologies(const int &default_value) {
  for (auto &topology: topologies) {
    for (int &j: topology) {
      j = default_value;
    }
  }

}

void lsrp(const vector<string> &lsrp_src_node) {
  validate_topologies();
  int src_node = -1;
  if (!lsrp_src_node.empty()) {
    src_node = stoi(lsrp_src_node[0]);
  }
  if (src_node != -1) {
    dijkstra(src_node);
    return;
  }
  for (int i = 1; i < topologies_count + 1; ++i) {
    cout << "\t************LSRP from node " << i << "************" << endl;
    dijkstra(i);
  }

}

int min_distance(const int distances[], const bool test[]) {
  int minimum = INF;
  int min_index;
  for (int i = 1; i < topologies_count + 1; ++i) {
    if (!test[i] && distances[i] <= minimum) {
      minimum = distances[i];
      min_index = i;
    }
  }
  return min_index;
}

void dijkstra(int src_node) {
  validate_topologies();
  if (src_node < 1 || src_node > topologies_count) {
    throw invalid_argument("invalid source node");
  }
  int distances[topologies_count + 1];
  bool test[topologies_count + 1];
  int parent[topologies_count + 1];
  for (int i = 1; i < topologies_count + 1; i++) {
    distances[i] = INF;
    parent[i] = -1;
    test[i] = false;
  }
  string delim;
  distances[src_node] = 0;
  int iterate = 1;
  for (int i = 1; i < topologies_count + 1; i++) {
    if (i == src_node)
      continue;
    int m = min_distance(distances, test);
    test[m] = true;
    for (int j = 1; j < topologies_count + 1; ++j) {
      if (!test[j] && topologies[m][j] && topologies[m][j] != -1 && distances[m] != INF &&
          distances[m] + topologies[m][j] < distances[j]) {
        distances[j] = distances[m] + topologies[m][j];
        parent[j] = m;
      }
    }
    stringstream ss;
    ss << delim;
    ss << "\t\tIter" << iterate << ": " << endl;
    ss << "Dest\t\t|";
    for (int j = 1; j < topologies_count + 1; j++) {
      ss << j << "| ";
    }
    ss << endl;
    ss << "Cost\t\t|";
    for (int j = 1; j < topologies_count + 1; j++) {
      if (distances[j] == INF) {
        ss << "-1| ";
      } else {
        ss << distances[j] << "| ";
      }
    }
    ss << endl;
    cout << ss.str();
    delim = "----------------------------------------\n";
    iterate++;
  }
  stringstream ss;
  ss << delim << "Path: [s] -> [d]\tMin-Cost\tShortest Path\n";
  ss << "     -----------\t----------\t----------------\n";
  for (int i = 1; i < topologies_count + 1; i++) {
    if (i != src_node) {
      ss << "      [" << src_node << "] -> [" << i << "]\t    " << distances[i] << "   \t";
      print_parents(parent, i, ss);
      ss << i << endl;
    }
  }
  cout << ss.str();
}

void print_parents(const int *parent, int j, stringstream &ss) {
  if (parent[j] != -1) {
    print_parents(parent, parent[j], ss);
    ss << parent[j] << " -> ";
  }
}

void bellman_ford(int src_node) {
  validate_topologies();
  if (src_node < 1 || src_node > topologies_count) {
    throw invalid_argument("invalid source node");
  }
  int distances[topologies_count + 1];
  int parent[topologies_count + 1];
  for (int i = 0; i < topologies_count + 1; i++) {
    distances[i] = INF;
    parent[i] = -1;
  }
  distances[src_node] = 0;
  for (int i = 0; i < topologies_count - 1; i++) {
    for (const auto &item: edges) {
      int src = item.first.first;
      int dest = item.first.second;
      int cost = item.second;
      if (distances[src] != INF && distances[src] + cost < distances[dest]) {
        distances[dest] = distances[src] + cost;
        parent[dest] = src;
      }
    }
  }

  stringstream ss;
  ss << "Dest\tNext Hop\tDist\tShortest Path\n";
  ss << "---------------------------------------\n";
  for (int i = 1; i < topologies_count + 1; i++) {
    auto next_hop = (parent[i] == -1) ? src_node : parent[i];
    ss << i << "\t" << next_hop << "\t\t" << distances[i] << "\t" << "[";
    print_parents(parent, i, ss);
    ss << i << "]\n";
  }
  cout << ss.str();
}

void dvrp(const vector<string> &dvrp_src_node) {
  validate_topologies();
  int src_node = -1;
  if (!dvrp_src_node.empty()) {
    src_node = stoi(dvrp_src_node[0]);
  }
  if (src_node != -1) {
    bellman_ford(src_node);
    return;
  }
  for (int i = 1; i < topologies_count + 1; ++i) {
    cout << "\t************DVRP from node " << i << "************" << endl;
    bellman_ford(i);
  }
}

void modify_topology(const vector<string> &topologies_str) {
  validate_topologies();
  if (topologies_str.empty() || topologies_str.size() > 1) {
    cout << "Usage: modify topology <src-dest-cost>" << endl;
    return;
  }
  auto topology_str = parse_string(topologies_str[0], TOPOLOGY_DELIMITER);
  auto source = stoi(topology_str[0]);
  auto dest = stoi(topology_str[1]);
  auto cost = stoi(topology_str[2]);
  topologies_count = max(topologies_count, max(source, dest));
  if (topologies[source][dest] == -1) {
    topologies[source][dest] = topologies[dest][source] = cost;
    topologies[source][source] = topologies[dest][dest] = 0;
    edges.insert(make_pair(make_pair(source, dest), cost));
    edges.insert(make_pair(make_pair(dest, source), cost));
  } else {
    topologies[source][dest] = topologies[dest][source] = cost;
    edges[make_pair(source, dest)] = cost;
    edges[make_pair(dest, source)] = cost;
  }
}

void remove_topology(const vector<string> &topologies_str) {
  validate_topologies();
  if (topologies_str.empty() || topologies_str.size() > 1) {
    cout << "Usage: remove topology <src-dest>" << endl;
    return;
  }
  auto topology_str = parse_string(topologies_str[0], TOPOLOGY_DELIMITER);
  auto source = stoi(topology_str[0]);
  auto dest = stoi(topology_str[1]);
  if (topologies[source][dest] == -1) {
    cout << "No such topology exists" << endl;
    return;
  }
  topologies[source][dest] = topologies[dest][source] = -1;
  edges.erase(make_pair(source, dest));
  edges.erase(make_pair(dest, source));
}
