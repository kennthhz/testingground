from typing import Optional, List
class TreeNode:
    def __init__(self, val):
        self.val = val
        self.left = None
        self.right = None

# Definition for a binary tree node.
# class TreeNode:
#     def __init__(self, val=0, left=None, right=None):
#         self.val = val
#         self.left = left
#         self.right = right
class Solution:

    def build(self, vals : List[int]) -> TreeNode:
        root : TreeNode
        root = TreeNode(vals.pop(0))
        cur_queue = []
        cur_queue.append(root)
        while(len(cur_queue) > 0):
            cur_node = cur_queue.pop(0)
            left = vals.pop(0) if len(vals) > 0 else None
            if left is not None:
                cur_node.left = TreeNode(left)
                cur_queue.append(cur_node.left)
            else:
                cur_node.left = None
            
            right = vals.pop(0) if len(vals) > 0 else None
            if right is not None:
                cur_node.right = TreeNode(right)
                cur_queue.append(cur_node.right)
            else:
                cur_node.right = None

        return root

    def zigzagLevelOrder(self, root: Optional[TreeNode]) -> List[List[int]]:
        if not root:
            return []
        result = []
        node_list = []
        
        node_list.append((root, 1))
        while(len(node_list) > 0):
            cur_level = node_list[0][1]
            if cur_level % 2 == 0:
                node = node_list.pop(0)
                if node[1] > len(result):
                    result.append([])
                result[node[1] - 1].append(node[0].val)

                if node[0].left:
                    node_list.append((node[0].left, node[1] + 1))
                if node[0].right:
                    node_list.append((node[0].right, node[1] + 1))
            else:
                node = node_list.pop()
                if node[1] > len(result):
                    result.append([])
                result[node[1] - 1].append(node[0].val)

                if node[0].right:
                    node_list.insert(0, (node[0].right, node[1] + 1))
                if node[0].left:
                    node_list.insert(0, (node[0].left, node[1] + 1))
    