from collections import defaultdict
from collections import deque

def build_adjacency_list(edges: list, directed:bool = False) -> defaultdict:
    graph = defaultdict(list)
    for src, target in edges:
        graph[src].append(target)
        if not directed:
            graph[target].append(src)
    return graph

def dfs(graph: defaultdict)-> int:
    stack = deque()
    visited = set()
    num_island = 0

    for src in graph.keys():
        if src not in visited:
            num_island = num_island + 1
            stack.append(src)
            while (stack):
                node = stack.pop()
                if node not in visited:
                    visited.add(node)
                    for target in graph[node]:
                        if target not in visited:
                            stack.append(target)
    return num_island
            
def bfs(graph: defaultdict)-> int:
    queue = deque()
    visited = set()
    num_island = 0

    for node in graph.keys():
        if node not in visited:
            num_island = num_island + 1
            queue.append(node)
            while (queue):
                current_node = queue.popleft()
                if current_node not in visited:
                    visited.add(current_node)
                    for neighbor in graph[current_node]:
                        if neighbor not in visited:
                            queue.append(neighbor)
    return num_island
