"""
Maximum Difference Between Node and Ancestor
Given the root of a binary tree, find the maximum value v for which there exist different nodes a and b where v = |a.val - b.val| and a is an ancestor of b
"""
class TreeNode:
    def __init__(self, val):
        self.val = val
        self.left = None
        self.right = None

max_val = 0
def dep_search(node : TreeNode, val_range : tuple[int, int]):
    global max
    low, high = val_range
    if node.val < low:
        diff = abs(high - node.val)
        max = diff if diff > max else max
        low = node.val
    elif node.val > high:
        diff = abs(node.val - low)
        max = diff if diff > max else max
        high = node.val
    
    new_tuple = [low, high]
    if node.left:
        dep_search(node.left, new_tuple)
    if node.right:
        dep_search(node.right, new_tuple)

def maxAncestorDiff(root : TreeNode):
    if not root:
        return 0
    dep_search(root, [root.val, root.val])
    return max

def main():
    root = TreeNode(8)
    root.left = TreeNode(3)
    root.left.left = TreeNode(1)
    root.left.right = TreeNode(6)
    root.left.right.left = TreeNode(4)
    root.left.right.right = TreeNode(7)
    root.right = TreeNode(10)
    root.right.right = TreeNode(14)
    root.right.right.left = TreeNode(13)

    print(maxAncestorDiff(root))

if __name__ == "__main__":
    main()

    

