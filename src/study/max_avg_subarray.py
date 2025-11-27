# Maximum average subarray
def max_avg_subarray(data:list[float], window_size: int):
    if window_size > len(data):
        raise ValueError('window size greater than array size')
    if window_size == 0:
        raise ValueError('window size 0')
    if data is None or len(data) == 0:
        raise ValueError('array cannot be empty')
    
    max_sum = -float('inf')
    current_sum = 0
    left = 0

    for right in range(len(data)):
        current_sum += data[right]
                    
        if right - left + 1 > window_size:
            current_sum -= data[left]
            left += 1
        
        if right - left + 1 == window_size:
            if current_sum > max_sum:
                max_sum = current_sum

    return max_sum/window_size
