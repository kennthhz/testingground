
# two pointer technique
def sum_sorted(nums : list[int], target: int) -> tuple[int, int]:
    """
    Given a sorted array, return indices of the two numbers that add up to a target.
    return (-1, -1) if no combination is possible
    """
    if len(nums) < 2:
        return (-1, -1)
    
    left = 0
    right = len(nums) - 1
    
    # smallest possible
    if nums[0] + nums[1] > target:
        return (-1,-1)
    
    # larget possible
    if nums[-1] + nums[-2] < target:
        return (-1,-1)

    while left < right:
        cur_sum = nums[left] + nums[right]
        if cur_sum == target:
            return (left, right)
        elif cur_sum > target:
            right = right - 1
        else:
            left = left + 1
    
    return (-1, -1)

    





