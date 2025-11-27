"""
the longest path between 2 nodes of a tree is the longest(left) + longest(right)
the longest path of a tree or root are max of all such trees.
"""
class TreeNode:
    def __init__(self, val):
        self.val = val
        self.left = None
        self.right = None


def get_longest_child(root : TreeNode, cur_longest : list):
    
    longest_left = get_longest_child(root.left, cur_longest) + 1 if root.left else 0
    longest_right = get_longest_child(root.right, cur_longest) + 1 if root.right else 0

    diameter = longest_left + longest_right
    cur_longest[0] = diameter if diameter > cur_longest[0] else cur_longest[0]
    return longest_left if longest_left > longest_right else longest_right


