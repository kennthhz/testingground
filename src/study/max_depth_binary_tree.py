class TreeNode:
    def __init__(self, val):
        self.val = val
        self.left = None
        self.right = None

# return max depth of a binary tree
def max_depth(node : TreeNode) -> int:
    if node == None:
        return 0

    r_max = max_depth(node.right)
    l_max = max_depth(node.left)

    return r_max + 1 if r_max >= l_max else l_max + 1

# given a binary tree and target sum, return true if path exists where sum = target
def path_sum(node : TreeNode, target : int) -> bool:
    if node == None:
        return False
    cur_sum = 0
    stack = [(node, node.val)]

    while stack:
        node, cur_sum = stack.pop()
        if node.left is None and node.right is None and cur_sum == target:
            return True
        
        if node.left:
            stack.append((node.left, cur_sum + node.left.val))
        if node.right:
            stack.append((node.right, cur_sum + node.right.val))

    return False

def min_depth_tree(node: TreeNode) -> int:
    if node.right == None and node.left == None:
        return 1

    l = float('inf')
    r = float('inf')

    if node.right:
        r = min_depth_tree(node.right)
    if node.left:
        l = min_depth_tree(node.left)

    return 1 + min(r, l)


