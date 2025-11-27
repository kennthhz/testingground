

def min_size_subarray(nums : list[int], target : int):
    left = 0
    right = 0
    current_sum = nums[left]
    min_length = len(nums) + 1

    while True:
        while True:
            if current_sum < target and right < len(nums) - 1:
                right = right + 1
                current_sum += nums[right]
            else:
                break

        if current_sum >= target:
            length = right - left + 1
            if length < min_length:
                min_length = length

        if (right == len(nums) -1) and current_sum < target:
            break

        if left <= right:
            current_sum = current_sum - nums[left]
            left = left + 1

        if (right == len(nums) - 1) and current_sum < target:
            break

    if (min_length != len(nums) + 1):
        return min_length
    else:
        return -1 








